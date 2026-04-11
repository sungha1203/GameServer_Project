#include "pch.h"
#include "SessionManager.h"
#include "Session.h"
#include <mutex>

SessionManager::SessionManager(std::unique_ptr<SessionFactory> factory)
	: sessionFactory(std::move(factory))
{
}

SessionManager::~SessionManager()
{
}

SessionPtr SessionManager::AcquireSession()
{
	if (!sessionFactory)
		return nullptr;

	return sessionFactory->Acquire();
}

void SessionManager::ActivateSession(const SessionPtr& session)
{
	if (session == nullptr) return;

	int sessionId = sessionIdCnt.fetch_add(1);
	session->SetSessionId(sessionId);

	{
		std::lock_guard<std::mutex> lock(sessionLock);
		activeSessions[sessionId] = session;
	}

	//PLOGI << "현재 접속자 수 : " << GetActiveSessionCnt();
}

void SessionManager::ReleaseSession(const SessionPtr& session)
{
	if (session == nullptr) return;

	{
		std::lock_guard<std::mutex> lock(sessionLock);
		activeSessions.erase(session->GetSessionId());
	}
}

int SessionManager::GetActiveSessionCnt()
{
	std::lock_guard<std::mutex> lock(sessionLock);
	return static_cast<int>(activeSessions.size());
}

std::vector<SessionPtr> SessionManager::GetActiveSessionsCopy()
{
	std::lock_guard<std::mutex> lock(sessionLock);

	std::vector<SessionPtr> sessions;
	sessions.reserve(activeSessions.size());

	for (auto it = activeSessions.begin(); it != activeSessions.end(); ++it)
	{
		sessions.push_back(it->second);
	}

	return sessions;
}

//void SessionManager::AddSession(std::shared_ptr<Session> session)
//{
//	std::lock_guard<std::mutex> lock(sessionLock);
//	sessions.push_back(session);
//	++sessionCnt;
//	if(sessionCnt % 10 == 0)
//	{
//		PLOGI << "현재 접속 : " << sessionCnt.load();
//	}
//}

//void SessionManager::RemoveSession(std::shared_ptr<Session> session)
//{
//	std::lock_guard<std::mutex> lock(sessionLock);
//	
//	auto it = std::find(sessions.begin(), sessions.end(), session);
//	if(it != sessions.end())
//	{
//		sessions.erase(it);
//		--sessionCnt;
//	}
//	if(sessionCnt % 10 == 1)
//	{
//		PLOGI << "현재 접속 : " << sessionCnt.load() - 1;
//	}
//}