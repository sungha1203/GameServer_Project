#include "pch.h"
#include "IocpCore.h"


IocpCore::IocpCore()
{
	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

IocpCore::~IocpCore()
{
	CloseHandle(h_iocp);
}

bool IocpCore::RegisterHandle(IocpObject* iocpObject)
{
	return CreateIoCompletionPort(iocpObject->GetHandle(), h_iocp, reinterpret_cast<ULONG_PTR>(iocpObject), 0);
}

bool IocpCore::Dispatch()
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	OVERLAPPED* overlapped = nullptr;

	bool ret = GetQueuedCompletionStatus(h_iocp, &numOfBytes, &key, &overlapped, 100);//INFINITE);

	if (FALSE == ret)
	{
		int err = ::WSAGetLastError();
		if (err == ERROR_NETNAME_DELETED || err == ERROR_CONNECTION_ABORTED || err == ERROR_BROKEN_PIPE) {
			PLOGE << "클라이언트 비정상 종료 감지.";
		}
	}

	IocpObject* iocpObject = reinterpret_cast<IocpObject*>(key);
	IocpEvent* iocpEvent = reinterpret_cast<IocpEvent*>(overlapped);

	if (overlapped == nullptr || iocpObject == nullptr)
		return false;

	iocpObject->Dispatch(iocpEvent, numOfBytes);
	return true;
}
