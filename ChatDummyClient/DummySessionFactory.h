#pragma once

#include "DummySession.h"
#include "ObjectPool.h"
#include "SessionFactory.h"

class DummySessionFactory : public SessionFactory
{
public:
	DummySessionFactory(int initCnt);

	virtual SessionPtr Acquire() override;

private:
	ObjectPool<DummySession> sessionPool;
};
