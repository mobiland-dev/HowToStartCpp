#include "pch.h"

#include "resource.h"

#include <DataFS\Client\DataFS Client.h>
//using namespace DataFoundation;

int _tmain(int argc, wchar_t* argv[])
{
	const wchar_t* strServerAddress = argv[1];

	UINT16 usServerPort = (UINT16)_wtoi(argv[2]);

	GUID guidDomainId;
	::CLSIDFromString(argv[3], &guidDomainId);

	// connect

	DataFS::InitializeThread();

	DataFS::Connection* pConnection;
	DataFS::Connection_Create(&pConnection);

	if(FAILED(pConnection->ConnectW(strServerAddress, usServerPort, NULL)))
	{
		DataFS::Connection_Destroy(pConnection);
		DataFS::UninitializeThread();
		return -1;
	}

	// extend schema

	HRSRC hBdtd = ::FindResource(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_RT_BDTD1), L"RT_BDTD");
	void* pBdtd = ::LockResource(::LoadResource(::GetModuleHandle(NULL), hBdtd));
	int iBdtdSize = ::SizeofResource(::GetModuleHandle(NULL), hBdtd);

	DataFS::USchemaEdit* pSchema;

	if(SUCCEEDED(pConnection->QuerySchemaEdit(&pSchema, &guidDomainId)))
	{
		pSchema->CreateFromBinary(pBdtd, iBdtdSize);
		pSchema->Commit();
		pSchema->Release();
	}

	pConnection->Disconnect();
	DataFS::Connection_Destroy(pConnection);
	DataFS::UninitializeThread();

	return 0;
}
