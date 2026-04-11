#pragma once
#include "pch.h"
#include "Session.h"
#include "ObjectPool.h"
#include <mutex>
#include <functional>

using SessionPtr = std::shared_ptr<Session>;
using SessionFactory = std::function<SessionPtr()>;

class SessionManager
{
public:
	SessionManager(SessionFactory factory);
	~SessionManager();
	
	SessionPtr					AcquireSession();
	void						ActivateSession(const SessionPtr& session);		// 클라 실제 접속 완료 후 활성화
	void						ReleaseSession(const SessionPtr& session);

	int							GetActiveSessionCnt();
	std::vector<SessionPtr>		GetActiveSessionsCopy();

private:
	std::mutex							sessionLock;
	SessionFactory						sessionFactory;
	//ObjectPool<Session>				sessionPool;
	unordered_map<int, SessionPtr>		activeSessions;
	atomic<int>							sessionIdCnt = 1;
};

