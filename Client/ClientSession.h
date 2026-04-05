#pragma once
#include "Session.h"

class ClientSession : public Session
{
public:
	void OnConnected();
	void OnDisconnected();

protected:
	virtual void ProcessPacket() override;
};

