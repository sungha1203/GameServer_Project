#include "ClientSessionFactory.h"

ClientSessionFactory::ClientSessionFactory(int initCnt)
	: sessionPool(initCnt)
{
}

SessionFactory::SessionPtr ClientSessionFactory::Acquire()
{
	std::shared_ptr<ClientSession> session = sessionPool.Acquire();
	return std::static_pointer_cast<Session>(session);
}