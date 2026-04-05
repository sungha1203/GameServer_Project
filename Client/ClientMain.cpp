#include "pch.h"
#include "Client.h"

int main()
{
	Client client;
	client.Init();
	client.Connect();
	client.Start();
	client.End();

	WSACleanup();
}