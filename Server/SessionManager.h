#pragma once
#include "pch.h"
#include "Session.h"
#include <mutex>

class SessionManager
{
public:
	SessionManager();
	~SessionManager();

	void AddSession(std::shared_ptr<Session> session);
	void RemoveSession(std::shared_ptr<Session> session);

	const std::vector<std::shared_ptr<Session>>& GetSessions() const { return sessions; }

private:
	std::mutex sessionLock;
	std::vector<std::shared_ptr<Session>> sessions;
	std::atomic<int> sessionCnt{};
};

