#include "pch.h"
#include "ChatClient.h"

#include <iostream>
#include <sstream>

ChatClient::ChatClient()
{
}

ChatClient::~ChatClient()
{
}

bool ChatClient::Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	if (ConfigLoader<ConfigServer>::Load("ChatClientConfig.ini", config) == false)
	{
		PLOGE << "Failed to load ChatClientConfig.ini";
		return false;
	}

	iocpCore = std::make_unique<IocpCore>();
	connector = std::make_unique<ChatConnector>(iocpCore.get());
	packetProcessor = std::make_unique<PacketProcessor>();
	packetProcessor->Start(1);
	sessionManager = std::make_unique<SessionManager>(std::make_unique<ChatClientSessionFactory>(1));

	return true;
}

bool ChatClient::Connect()
{
	if (!connector || !sessionManager)
		return false;

	std::shared_ptr<Session> baseSession = sessionManager->AcquireSession();
	baseSession->SetSessionManager(sessionManager.get());

	if (!connector->Connect(baseSession, config.ip, config.port))
		return false;

	sessionManager->ActivateSession(baseSession);
	baseSession->OnConnected();
	session = std::static_pointer_cast<ChatClientSession>(baseSession);
	return true;
}

void ChatClient::Start()
{
	running = true;

	const unsigned int hardwareCount = std::thread::hardware_concurrency();
	const int workerCount = static_cast<int>(hardwareCount == 0 ? 1 : hardwareCount);
	for (int i = 0; i < static_cast<int>(workerCount); ++i)
	{
		workers.emplace_back([this]()
			{
				while (running)
					iocpCore->Dispatch();
			});
	}
}

void ChatClient::RunConsole()
{
	PrintCommands();

	std::string line;
	while (running && std::getline(std::cin, line))
	{
		if (line.empty())
			continue;

		HandleCommand(line);
	}
}

void ChatClient::End()
{
	running = false;

	if (session)
		session->Disconnect();

	for (auto& worker : workers)
	{
		if (worker.joinable())
			worker.join();
	}

	if (packetProcessor)
	{
		packetProcessor->Stop();
		packetProcessor.reset();
	}

	workers.clear();
	session.reset();
	sessionManager.reset();
	connector.reset();
	iocpCore.reset();

	WSACleanup();
}

void ChatClient::PrintCommands()
{
	std::cout << "Commands: login [nickname], rooms, enter <roomId>, say <message>, leave, quit" << std::endl;
}

void ChatClient::HandleCommand(const std::string& line)
{
	std::istringstream iss(line);
	std::string cmd;
	iss >> cmd;

	std::shared_ptr<ChatClientSession> chatSession = GetSession();
	if (!chatSession || !chatSession->IsConnected())
	{
		std::cout << "Not connected." << std::endl;
		return;
	}

	if (cmd == "login")
	{
		std::string nickname;
		std::getline(iss, nickname);
		if (!nickname.empty() && nickname.front() == ' ')
			nickname.erase(0, 1);

		chatSession->SendLogin(nickname);
	}
	else if (cmd == "rooms")
	{
		chatSession->SendRoomListReq();
	}
	else if (cmd == "enter")
	{
		int roomId = 0;
		iss >> roomId;
		chatSession->SendRoomEnter(roomId);
	}
	else if (cmd == "leave")
	{
		chatSession->SendRoomLeave();
	}
	else if (cmd == "say")
	{
		std::string message;
		std::getline(iss, message);
		if (!message.empty() && message.front() == ' ')
			message.erase(0, 1);

		chatSession->SendChat(message);
	}
	else if (cmd == "quit")
	{
		running = false;
	}
	else
	{
		std::cout << "Unknown command: " << cmd << std::endl;
		PrintCommands();
	}
}

std::shared_ptr<ChatClientSession> ChatClient::GetSession()
{
	return session;
}
