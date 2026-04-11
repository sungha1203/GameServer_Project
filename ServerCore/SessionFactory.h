#pragma once
#include "pch.h"
#include "Session.h"

class SessionFactory
{
public:
	using SessionPtr = std::shared_ptr<Session>;

public:
	virtual ~SessionFactory() = default;

public:
	virtual SessionPtr Acquire() = 0;
};