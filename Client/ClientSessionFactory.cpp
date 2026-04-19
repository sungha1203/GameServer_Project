#include "ClientSessionFactory.h"

ClientSessionFactory::ClientSessionFactory(int initCnt)
	: sessionPool(initCnt)
{
}

SessionFactory::SessionPtr ClientSessionFactory::Acquire()
{
	std::shared_ptr<ClientSession> session = sessionPool.Acquire();
	return std::static_pointer_cast<Session>(session);  //업캐스팅 후 반환, 세션 매니저에서 SessionPtr로 관리하기 위해
}