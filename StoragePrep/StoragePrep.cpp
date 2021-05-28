#include "stdafx.h"

#include <DataFS\Client\DataFS Client.h>
//using namespace DataFoundation;

#include <DataFS\Access\DataFS Access.h>
using namespace DataFoundationAccess;

#include "_data\PrepareDefinition.h"

// {56F8EB44-B7E3-4564-B9A6-22E5E1B9110C}
const GUID guidRootName =
{ 0x56f8eb44, 0xb7e3, 0x4564, { 0xb9, 0xa6, 0x22, 0xe5, 0xe1, 0xb9, 0x11, 0xc } };

int _tmain(int argc, wchar_t* argv[])
{
	const wchar_t* strServerAddress = argv[1];

	UINT16 usServerPort = (UINT16)_wtoi(argv[2]);

	GUID guidDomainId;
	::CLSIDFromString(argv[3], &guidDomainId);

	UINT32 ulStorageId = 0; // currently always 0

	// connect
	DataFoundation::InitializeThread();

	WDomain* pWDomain = NewWDomain(0);
	if (FAILED(pWDomain->Initialize()))
	{
		DeleteWDomain(pWDomain);
		DataFoundation::UninitializeThread();
		return -1;
	}

	if (FAILED(pWDomain->Connect(strServerAddress, usServerPort, &guidDomainId, NULL)))
	{
		pWDomain->Uninitialize();
		DeleteWDomain(pWDomain);
		DataFoundation::UninitializeThread();
		return -1;
	}

	if (FAILED(pWDomain->QueryStorage(ulStorageId, false)))
	{
		pWDomain->DisconnectAll();
		pWDomain->Uninitialize();
		DeleteWDomain(pWDomain);
		DataFoundation::UninitializeThread();
		return -1;
	}

	// bind types
	PrepareDefinition::Bind(pWDomain);

	// create named object

	ITestRoot* pRootObject;

	PrepareDefinition::Create(pWDomain, &pRootObject);

	pRootObject->SetRootName(L"first test root");

	pRootObject->StoreData();

	pWDomain->InsertNamedObject(&pRootObject->BuildLink(true), &guidRootName, L"first entry point");

	pWDomain->Execute(TRANSACTION_STORE);

	pRootObject->Release();

	// unbind types
	PrepareDefinition::Unbind();

	// disconnect

	pWDomain->ReleaseStorage(ulStorageId);
	pWDomain->DisconnectAll();
	pWDomain->Uninitialize();
	DeleteWDomain(pWDomain);
	DataFoundation::UninitializeThread();

	return 0;
}
