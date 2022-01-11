#include "stdafx.h"

#include <DataFS\Access\DataFS Access.h>
using namespace DataFoundationAccess;

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

	WDomain* pWDomain = Domain_Create();
	if (FAILED(pWDomain->Initialize(&guidDomainId)))
	{
		Domain_Destroy(pWDomain);
		UninitializeThread();
		return -1;
	}

	if (FAILED(pWDomain->Connect(strServerAddress, usServerPort, NULL)))
	{
		pWDomain->Uninitialize();
		Domain_Destroy(pWDomain);
		UninitializeThread();
		return -1;
	}

	if (FAILED(pWDomain->QueryStorage(ulStorageId, false)))
	{
		pWDomain->DisconnectAll();
		pWDomain->Uninitialize();
		Domain_Destroy(pWDomain);
		UninitializeThread();
		return -1;
	}

	// bind types
	AccessDefinition::Bind(pWDomain);

	// open named object

	DataFoundation::ObjectId oiRootObject;

	pWDomain->QueryNamedObjectId(&guidRootName, 1, &oiRootObject);

	ITestRoot* pRootObject;
	ITestRoot::Open(&pRootObject, &oiRootObject, pWDomain);

	pRootObject->Load();
	pWDomain->Execute(Transaction::Load);

	// open the list for writing

	TestObjectList* pList;
	pRootObject->SetAllObjects(&pList);

	// create an add a new object

	ITestObject* pTestObject;
	ITestObject::Create(&pTestObject, pRootObject);

	TestObjectListItem itm;
	itm.anObject = pTestObject->BuildLink(true);
	itm.theType = 12;

	pList->Insert(NULL, &itm);

	pTestObject->SetText(L"something");
	pTestObject->SetNumber(343);

	pTestObject->StoreData();
	pRootObject->StoreData();

	pWDomain->Execute(Transaction::Store);

	pTestObject->Release();
	pRootObject->Release();

	// unbind types
	AccessDefinition::Unbind();

	// disconnect

	pWDomain->ReleaseStorage(ulStorageId);
	pWDomain->DisconnectAll();
	pWDomain->Uninitialize();
	Domain_Destroy(pWDomain);
	UninitializeThread();

	return 0;
}
