#pragma once
#include "pch.h"

class Session;

enum class EventType
{
	Accept,
	Recv,
	Send
};

class IocpEvent
{
public:
	IocpEvent(EventType type) : type(type) {}

public:
	OVERLAPPED overlapped{};
	EventType type;
};

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) 
	{
		ZeroMemory(&overlapped, sizeof(overlapped));
	}

public:
	std::shared_ptr<Session> session = nullptr;
	char buffer[sizeof(SOCKADDR_IN) * 2 + 32]{};
};

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}

public:
	char buffer[1024]{};
};

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}

public:
	std::vector<char> sendbuffer;
};