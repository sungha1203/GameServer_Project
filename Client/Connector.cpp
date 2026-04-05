#include "Connector.h"

Connector::Connector(IocpCore* iocpCore)
	:iocpCore(iocpCore)
{

}

bool Connector::Connect()
{
	return true;
}