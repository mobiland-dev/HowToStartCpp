#include "pch.h"

#include <DataFS\Access\DataFS Access.h>
using namespace DataFSAccess;

#include "_data\AccessDefinition.h"

// {56F8EB44-B7E3-4564-B9A6-22E5E1B9110C}
const GUID guidRootName =
{ 0x56f8eb44, 0xb7e3, 0x4564, { 0xb9, 0xa6, 0x22, 0xe5, 0xe1, 0xb9, 0x11, 0xc } };

int _tmain(int argc, wchar_t* argv[])
{
	const wchar_t* strServerAddress = argv[1];

	UINT16 usServerPort = (UINT16)_wtoi(argv[2]);

	GUID guidDomainId;
	::CLSIDFromString(argv[3], &guidDomainId);

	UINT32 ulStorageId = 0;

	// connect

	InitializeThread();

	Connection* pConnection = Connection_Create();

	if(FAILED(pConnection->Initialize(&guidDomainId)))
	{
		Connection_Destroy(pConnection);
		UninitializeThread();
		return -1;
	}

	if(FAILED(pConnection->Connect(strServerAddress, usServerPort, NULL)))
	{
		pConnection->Uninitialize();
		Connection_Destroy(pConnection);
		UninitializeThread();
		return -1;
	}

	if(FAILED(pConnection->QueryStorage(ulStorageId, false)))
	{
		pConnection->DisconnectAll();
		pConnection->Uninitialize();
		Connection_Destroy(pConnection);
		UninitializeThread();
		return -1;
	}

	// build domain

	WDomain* pWDomain = Domain_Create();
	if(FAILED(pWDomain->Initialize(pConnection, 0)))
	{
		Domain_Destroy(pWDomain);
		pConnection->ReleaseAllStorages();
		pConnection->DisconnectAll();
		pConnection->Uninitialize();
		Connection_Destroy(pConnection);
		UninitializeThread();
		return -1;
	}

	// bind types

	AccessDefinition::Bind(pWDomain);

	// open named object

	DataFS::ObjectId oiRootObject;

	pWDomain->QueryNamedLinkId(&guidRootName, 1, &oiRootObject, NULL);

	TestRoot* pRootObject;
	TestRoot::Open(&pRootObject, pWDomain, &oiRootObject);

	pRootObject->Load();
	pWDomain->Execute(Transaction::Load, NULL);

	// open the list for writing

	TestObjectList* pList;
	pRootObject->SetAllObjects(&pList);

	// create an add a new object

	TestObject* pTestObject;
	TestObject::Create(&pTestObject, pRootObject->GetObject());

	TestObjectListItem itm;
	itm.anObject = pTestObject->BuildLink(true);
	itm.theType = 12;

	pList->Insert(NULL, &itm);

	pTestObject->SetText(L"something");
	pTestObject->SetNumber(343);

	pTestObject->Store();
	pRootObject->Store();

	pWDomain->Execute(Transaction::Store, NULL);

	pTestObject->Release();
	pRootObject->Release();

	// unbind types

	AccessDefinition::Unbind();

	// destroy domain

	pWDomain->Uninitialize();
	Domain_Destroy(pWDomain);

	// disconnect

	pConnection->ReleaseAllStorages();
	pConnection->DisconnectAll();
	pConnection->Uninitialize();
	Connection_Destroy(pConnection);
	UninitializeThread();

	return 0;
}
