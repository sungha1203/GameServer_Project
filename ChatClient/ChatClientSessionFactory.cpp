#include "pch.h"
#include "ChatClientSessionFactory.h"

ChatClientSessionFactory::ChatClientSessionFactory(int initCnt)
	: sessionPool(initCnt)
{
}

SessionFactory::SessionPtr ChatClientSessionFactory::Acquire()
{
	std::shared_ptr<ChatClientSession> session = sessionPool.Acquire();
	session->Reset();
	return session;
}
