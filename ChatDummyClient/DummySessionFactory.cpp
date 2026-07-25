#include "pch.h"
#include "DummySessionFactory.h"

DummySessionFactory::DummySessionFactory(int initCnt)
	: sessionPool(initCnt)
{
}

SessionFactory::SessionPtr DummySessionFactory::Acquire()
{
	std::shared_ptr<DummySession> session = sessionPool.Acquire();
	session->Reset();
	return session;
}
