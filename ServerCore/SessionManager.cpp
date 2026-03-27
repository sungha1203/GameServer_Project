#include "pch.h"
#include "SessionManager.h"
#include "Session.h"
#include <mutex>

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
}

void SessionManager::AddSession(std::shared_ptr<Session> session)
{
	std::lock_guard<std::mutex> lock(sessionLock);
	sessions.push_back(session);
	++sessionCnt;
	if(sessionCnt % 10 == 0)
	{
		cout << "泅犁 立加 : " << sessionCnt.load() << endl;
	}
}

void SessionManager::RemoveSession(std::shared_ptr<Session> session)
{
	std::lock_guard<std::mutex> lock(sessionLock);
	
	auto it = std::find(sessions.begin(), sessions.end(), session);
	if(it != sessions.end())
	{
		sessions.erase(it);
		--sessionCnt;
	}
	if(sessionCnt % 10 == 1)
	{
		cout << "泅犁 立加 : " << sessionCnt.load() - 1 << endl;
	}
}