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
	// 직접 세션을 만들지 않음. 팩토리에 위임.

	SessionPtr session = sessionFactory->Acquire();
	if(session)
		session->SetSessionManager(this);

	return session;
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
	//PLOGD << "ActivateSession id = " << session->GetSessionId() << ", active =  " << activeSessions.size();
	//PLOGI << "현재 접속자 수 : " << GetActiveSessionCnt();
}

void SessionManager::ReleaseSession(const SessionPtr& session)
{
	if (session == nullptr) return;

	{
		std::lock_guard<std::mutex> lock(sessionLock);
		activeSessions.erase(session->GetSessionId());
	}
	//PLOGE << "ReleaseSession id = " << session->GetSessionId() << ", active =  " << activeSessions.size();
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