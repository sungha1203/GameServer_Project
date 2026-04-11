#pragma once
#include "pch.h"
#include "GameSession.h"
#include "SessionFactory.h"
#include "ObjectPool.h"

class GameSessionFactory : public SessionFactory
{
public:
	GameSessionFactory(int initCnt = 1000);

public:
	virtual SessionPtr	Acquire() override;

private:
	ObjectPool<GameSession>		sessionPool;
};