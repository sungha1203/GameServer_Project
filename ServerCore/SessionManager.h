#pragma once
#include "pch.h"
#include "Session.h"
#include "ObjectPool.h"
#include "SessionFactory.h"
#include <mutex>
#include <functional>

using SessionPtr = std::shared_ptr<Session>;

class SessionManager
{
public:
	SessionManager(std::unique_ptr<SessionFactory> factory);
	~SessionManager();
	
	SessionPtr					AcquireSession();
	void						ActivateSession(const SessionPtr& session);		// 클라 실제 접속 완료 후 활성화
	void						ReleaseSession(const SessionPtr& session);

	int							GetActiveSessionCnt();
	std::vector<SessionPtr>		GetActiveSessionsCopy();

private:
	std::mutex								sessionLock;
	std::unique_ptr<SessionFactory>			sessionFactory;
	std::unordered_map<int, SessionPtr>		activeSessions;
	std::atomic<int>						sessionIdCnt = 1;
};

