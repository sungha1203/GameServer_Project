#include "GameSessionFactory.h"

GameSessionFactory::GameSessionFactory(int initCnt)
	: sessionPool(initCnt)
{
}

SessionFactory::SessionPtr GameSessionFactory::Acquire()
{
	std::shared_ptr<GameSession> session = sessionPool.Acquire();
	return std::static_pointer_cast<Session>(session);
}