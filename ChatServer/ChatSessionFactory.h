#pragma once

#include "ObjectPool.h"
#include "SessionFactory.h"
#include "ChatSession.h"

class ChatSessionFactory : public SessionFactory
{
public:
	ChatSessionFactory(int initCnt = 1000);

public:
	virtual SessionPtr Acquire() override;

private:
	ObjectPool<ChatSession> sessionPool;
};
