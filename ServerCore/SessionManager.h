#pragma once
#include "pch.h"
#include "Session.h"
#include "ObjectPool.h"
#include <mutex>

using SessionPtr = std::shared_ptr<Session>;

class SessionManager
{
public:
	SessionManager(int initCnt);
	~SessionManager();
	
	SessionPtr					AcquireSession();
	void						ActivateSession(const SessionPtr& session);		// 클라 실제 접속 완료 후 활성화
	void						ReleaseSession(const SessionPtr& session);

	int							GetActiveSessionCnt();
	std::vector<SessionPtr>		GetActiveSessionsCopy();

	//void			AddSession(std::shared_ptr<Session> session);
	//void			RemoveSession(std::shared_ptr<Session> session);
	//const std::vector<std::shared_ptr<Session>>& GetSessions() const { return sessions; }

private:
	std::mutex							sessionLock;
	ObjectPool<Session>					sessionPool;
	unordered_map<int, SessionPtr>		activeSessions;
	atomic<int>							sessionIdCnt = 1;

	//std::vector<std::shared_ptr<Session>> sessions;
	//std::atomic<int> sessionCnt{};
};

