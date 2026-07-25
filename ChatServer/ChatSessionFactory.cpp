#include "ChatSessionFactory.h"

ChatSessionFactory::ChatSessionFactory(int initCnt)
	: sessionPool(initCnt)
{
}

SessionFactory::SessionPtr ChatSessionFactory::Acquire()
{
	std::shared_ptr<ChatSession> session = sessionPool.Acquire();
	return std::static_pointer_cast<Session>(session);
}
