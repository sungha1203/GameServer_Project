#pragma once

#include "ChatClientSession.h"
#include "ObjectPool.h"
#include "SessionFactory.h"

class ChatClientSessionFactory : public SessionFactory
{
public:
	ChatClientSessionFactory(int initCnt = 1);

	virtual SessionPtr Acquire() override;

private:
	ObjectPool<ChatClientSession> sessionPool;
};
