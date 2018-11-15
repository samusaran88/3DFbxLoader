#include "stdafx.h"
#include "FbxLoadTest.h"


FbxLoadTest::FbxLoadTest()
{
	testObj = new FbxLoader;
	//testObj->LoadFbx("Object/WOW/bear2.fbx");
	testObj->LoadFbx("Object/WOW/Deathwing.fbx");

	//FbxDx11->LoadFBX("Object/WOW/bear2.fbx", DEVICE);
	//FbxDx11->LoadFBX("Object/Cloud/Cloud.fbx", DEVICE);
	//FbxDx11->LoadFBX("Object/Splicer Woman/Splicer Woman.fbx", DEVICE);
	//FbxDx11->LoadFBX("Object/Neptune/Saturn/Saturn.fbx", DEVICE);
	//FbxDx11->LoadFBX("Object/Protector Bouncer Elite/ProtectorBouncerElite.fbx", DEVICE);
	//FbxDx11->LoadFBX("Object/Haruko/Models/Haruko.fbx", DEVICE);
	//FbxDx11->CreateInputLayout(DEVICE, shader->GetVSBlob()->GetBufferPointer(), shader->GetVSBlob()->GetBufferSize(), &(shader->GetInputLayoutDesc()[0]), shader->GetInputLayoutDesc().size());
}


FbxLoadTest::~FbxLoadTest()
{
}

void FbxLoadTest::Update()
{
	testObj->Update();
}

void FbxLoadTest::Render()
{
	testObj->Render();
}
