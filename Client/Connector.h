#pragma once

class IocpCore;

class Connector
{
public:
	Connector(IocpCore* iocpCore);

	bool Connect();

private:
	IocpCore* iocpCore = nullptr;
};

