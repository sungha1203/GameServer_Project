#pragma once

#include "ChatProtocol.h"
#include "PacketSession.h"

#include <mutex>
#include <unordered_map>

class DummyMetrics;

enum class DummySessionState
{
	Disconnected,
	Connected,
	LoggedIn,
	InRoom,
};

class DummySession : public PacketSession
{
public:
	DummySession();
	virtual ~DummySession();

	void InitDummy(int dummyId, int roomId, DummyMetrics* metrics);

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void Reset() override;

	bool IsReadyToChat() const { return state.load() == DummySessionState::InRoom; }
	bool SendChatTick();

protected:
	virtual void OnRecvPacket(PacketHeader header, const char* buffer, int len) override;

private:
	bool SendPacket(uint16 packetId, const std::string& payload);
	void HandleLoginRes(const std::string& payload);
	void HandleRoomEnterRes(const std::string& payload);
	void HandleChat(const std::string& payload);
	void TrackLatency(int seq, long long sendTickMs);
	std::string MakePayload(const char* buffer, int len);

private:
	int dummyId = 0;
	int roomId = 1;
	int nextSeq = 1;
	std::string nickname;
	DummyMetrics* metrics = nullptr;
	std::atomic<DummySessionState> state = DummySessionState::Disconnected;

	std::mutex pendingLock;
	std::unordered_map<int, long long> pendingSendTicks;
};
