#include "pch.h"
#include "DummySession.h"

#include "DummyMetrics.h"

#include <chrono>

namespace
{
	long long NowMs()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	bool TryReadField(const std::string& message, const std::string& key, int& value)
	{
		const std::string prefix = key + "=";
		const size_t begin = message.find(prefix);
		if (begin == std::string::npos)
			return false;

		const size_t valueBegin = begin + prefix.size();
		const size_t valueEnd = message.find(';', valueBegin);
		const std::string token = message.substr(valueBegin, valueEnd == std::string::npos ? std::string::npos : valueEnd - valueBegin);

		try
		{
			value = std::stoi(token);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
}

DummySession::DummySession()
{
}

DummySession::~DummySession()
{
}

void DummySession::InitDummy(int id, int targetRoomId, DummyMetrics* targetMetrics)
{
	dummyId = id;
	roomId = targetRoomId;
	metrics = targetMetrics;
	nickname = "dummy" + std::to_string(dummyId);
}

void DummySession::OnConnected()
{
	state.store(DummySessionState::Connected);
	if (metrics)
		metrics->OnConnected();

	SendPacket(C_LOGIN_REQ, nickname);
}

void DummySession::OnDisconnected()
{
	const DummySessionState oldState = state.exchange(DummySessionState::Disconnected);
	if (metrics && oldState != DummySessionState::Disconnected)
		metrics->OnDisconnected();

	std::lock_guard<std::mutex> lock(pendingLock);
	pendingSendTicks.clear();
}

void DummySession::Reset()
{
	PacketSession::Reset();
	nextSeq = 1;
	nickname.clear();
	metrics = nullptr;
	state.store(DummySessionState::Disconnected);

	std::lock_guard<std::mutex> lock(pendingLock);
	pendingSendTicks.clear();
}

bool DummySession::SendChatTick()
{
	if (!IsReadyToChat())
		return false;

	const int seq = nextSeq++;
	const long long now = NowMs();
	const std::string message = "seq=" + std::to_string(seq) + ";time=" + std::to_string(now) + ";msg=load-test";

	{
		std::lock_guard<std::mutex> lock(pendingLock);
		pendingSendTicks[seq] = now;
	}

	if (!SendPacket(C_CHAT_REQ, message))
	{
		std::lock_guard<std::mutex> lock(pendingLock);
		pendingSendTicks.erase(seq);
		return false;
	}

	if (metrics)
		metrics->OnSent();

	return true;
}

void DummySession::OnRecvPacket(PacketHeader header, const char* buffer, int len)
{
	const std::string payload = MakePayload(buffer, len);

	switch (header.id)
	{
	case S_LOGIN_RES:
		HandleLoginRes(payload);
		break;
	case S_ROOM_ENTER_RES:
		HandleRoomEnterRes(payload);
		break;
	case S_CHAT:
		HandleChat(payload);
		break;
	case S_ERROR:
		if (metrics)
			metrics->OnError();
		break;
	default:
		break;
	}
}

bool DummySession::SendPacket(uint16 packetId, const std::string& payload)
{
	PacketHeader header;
	header.size = static_cast<uint16>(sizeof(PacketHeader) + payload.size());
	header.id = packetId;

	std::vector<char> sendBuffer(header.size);
	memcpy(sendBuffer.data(), &header, sizeof(PacketHeader));
	memcpy(sendBuffer.data() + sizeof(PacketHeader), payload.data(), payload.size());

	return Send(sendBuffer.data(), static_cast<int>(sendBuffer.size()));
}

void DummySession::HandleLoginRes(const std::string& payload)
{
	if (payload.rfind("OK|", 0) != 0)
	{
		if (metrics)
			metrics->OnError();
		return;
	}

	state.store(DummySessionState::LoggedIn);
	if (metrics)
		metrics->OnLoggedIn();

	SendPacket(C_ROOM_ENTER_REQ, std::to_string(roomId));
}

void DummySession::HandleRoomEnterRes(const std::string& payload)
{
	if (payload.rfind("OK|", 0) != 0)
	{
		if (metrics)
			metrics->OnError();
		return;
	}

	state.store(DummySessionState::InRoom);
	if (metrics)
		metrics->OnEnterRoom();
}

void DummySession::HandleChat(const std::string& payload)
{
	if (metrics)
		metrics->OnReceived();

	const size_t sep = payload.find('|');
	if (sep == std::string::npos)
		return;

	const std::string sender = payload.substr(0, sep);
	if (sender != nickname)
		return;

	const std::string message = payload.substr(sep + 1);
	int seq = 0;
	if (!TryReadField(message, "seq", seq))
		return;

	long long sendTickMs = 0;
	{
		std::lock_guard<std::mutex> lock(pendingLock);
		auto it = pendingSendTicks.find(seq);
		if (it == pendingSendTicks.end())
			return;

		sendTickMs = it->second;
		pendingSendTicks.erase(it);
	}

	TrackLatency(seq, sendTickMs);
}

void DummySession::TrackLatency(int seq, long long sendTickMs)
{
	UNREFERENCED_PARAMETER(seq);

	if (metrics)
		metrics->OnLatency(NowMs() - sendTickMs);
}

std::string DummySession::MakePayload(const char* buffer, int len)
{
	if (buffer == nullptr || len <= 0)
		return "";

	return std::string(buffer, buffer + len);
}
