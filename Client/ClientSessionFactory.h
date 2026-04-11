#pragma once
#include "pch.h"
#include "ClientSession.h"
#include "ObjectPool.h"
#include "SessionFactory.h"

class ClientSessionFactory : public SessionFactory
{
public:
	ClientSessionFactory(int initCnt = 1000);

public:
	virtual SessionPtr	Acquire() override;

private:
	ObjectPool<ClientSession>		sessionPool;
};