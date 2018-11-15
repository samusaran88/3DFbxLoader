#include "stdafx.h"
#include "FbxLoader.h"


FbxLoader::FbxLoader()
	: importer(nullptr)
	, scene(nullptr)
	, sdkManager(nullptr)
{
	shader = new Shader;
	shader->CreateShader(L"FbxShader.hlsl");
	F4X4Identity(world);
}


FbxLoader::~FbxLoader()
{
	SAFE_DESTROY(importer);
	SAFE_DESTROY(scene);
	SAFE_DESTROY(sdkManager);

	SAFE_DELETE(shader);

	SAFE_RELEASE(cameraBuffer);
	SAFE_RELEASE(boneBuffer);
	SAFE_RELEASE(pixelBuffer);
	SAFE_RELEASE(structuredBuffer);
	SAFE_RELEASE(transformSRV);

	for (auto iter = controlPoints.begin(); iter != controlPoints.end(); ++iter)
	{
		SAFE_DELETE(iter->second);
		iter->second = nullptr;
	}
	FbxArrayDelete(AnimStackNameArray);
}

void FbxLoader::LoadFbx(string FileName)
{
	InitializeSdkObjects(sdkManager, scene);

	int FileFormat = -1;
	importer = FbxImporter::Create(sdkManager, "");

	if (!sdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(FileName.c_str(), FileFormat))
	{
		// Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
		FileFormat = sdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");;
	}

	importer->Initialize(FileName.c_str(), FileFormat);
	importer->Import(scene);

	FbxAxisSystem OurAxisSystem = FbxAxisSystem::DirectX;

	// DirectX
	FbxAxisSystem SceneAxisSystem = scene->GetGlobalSettings().GetAxisSystem();
	if (SceneAxisSystem != OurAxisSystem)
	{
		FbxAxisSystem::DirectX.ConvertScene(scene);
	}

	FbxSystemUnit SceneSystemUnit = scene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		FbxSystemUnit::cm.ConvertScene(scene);
	}

	TriangulateRecursive(scene->GetRootNode());
	ProcessSkeletalHeirarchy(scene->GetRootNode());

	Setup();

	shader->CreateBuffer(&vertices[0], sizeof(Vertex), vertices.size(), &indices[0], sizeof(WORD), indices.size());
	CreateConstantBuffers(&cameraBuffer, sizeof(FBX_CAMERA_BUFFER));
	CreateConstantBuffers(&pixelBuffer, sizeof(FBX_PIXEL_BUFFER));
	CreateStructuredBuffer();
	UpdatePixelBuffer();
}

void FbxLoader::InitializeSdkObjects(FbxManager*& Manager, FbxScene*& Scene)
{
	Manager = FbxManager::Create();

	FbxIOSettings* ios = FbxIOSettings::Create(Manager, IOSROOT);
	Manager->SetIOSettings(ios);

	FbxString Path = FbxGetApplicationDirectory();
	Manager->LoadPluginsDirectory(Path.Buffer());

	Scene = FbxScene::Create(Manager, "Fbx Scene");
}

void FbxLoader::TriangulateRecursive(FbxNode* Node)
{
	FbxNodeAttribute* NodeAttribute = Node->GetNodeAttribute();

	if (NodeAttribute)
	{
		if (NodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
			NodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
			NodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
			NodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
		{
			FbxGeometryConverter Converter(Node->GetFbxManager());
#if 0
			Converter.TriangulateInPlace(Node);
#endif // 0
			Converter.Triangulate(scene, true);
		}
	}

	const int ChildCount = Node->GetChildCount();
	for (int ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		TriangulateRecursive(Node->GetChild(ChildIndex));
	}
}

void FbxLoader::Setup()
{
	if (scene->GetRootNode())
	{
		SetupNode(scene->GetRootNode());
	}
}

void FbxLoader::SetupNode(FbxNode* Node)
{
	if (!Node)
		return;

	FbxMesh* Mesh = Node->GetMesh();

	if (Mesh)
	{
		meshNode = Mesh->GetNode();
		ProcessControlPoints(Mesh);
		scene->FillAnimStackNameArray(AnimStackNameArray);
		ProcessAnimation();
		ProcessVertex(Mesh);
	}

	const int MaterialCount = Node->GetMaterialCount();
	for (int i = 0; i < MaterialCount; ++i)
	{
		FbxSurfaceMaterial* mat = Node->GetMaterial(i);
		if (!mat)
			continue;

		FBX_MATERIAL_NODE destMat;
		CopyMatrialData(mat, &destMat);

		materialArray.push_back(destMat);
	}

	const int Count = Node->GetChildCount();
	for (int i = 0; i < Count; ++i)
	{
		SetupNode(Node->GetChild(i));
	}
}

void FbxLoader::CopyMatrialData(FbxSurfaceMaterial* mat, FBX_MATERIAL_NODE* destMat)
{
	if (!mat)
		return;

	if (mat->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		destMat->type = FBX_MATERIAL_NODE::MATERIAL_LAMBERT;
	}
	else if (mat->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		destMat->type = FBX_MATERIAL_NODE::MATERIAL_PHONG;
	}

	const FbxDouble3 Emissive = GetMaterialProperty(mat,
		FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, &destMat->emmisive);
	SetFbxColor(destMat->emmisive, Emissive);

	const FbxDouble3 Ambient = GetMaterialProperty(mat,
		FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, &destMat->ambient);
	SetFbxColor(destMat->ambient, Ambient);

	const FbxDouble3 Diffuse = GetMaterialProperty(mat,
		FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, &destMat->diffuse);
	SetFbxColor(destMat->diffuse, Diffuse);

	const FbxDouble3 Specular = GetMaterialProperty(mat,
		FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, &destMat->specular);
	SetFbxColor(destMat->specular, Specular);

	FbxProperty TransparencyFactorProperty = mat->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	if (TransparencyFactorProperty.IsValid())
	{
		double TransparencyFactor = TransparencyFactorProperty.Get<FbxDouble>();
		destMat->TransparencyFactor = static_cast<float>(TransparencyFactor);
	}

	// Specular Power
	FbxProperty ShininessProperty = mat->FindProperty(FbxSurfaceMaterial::sShininess);
	if (ShininessProperty.IsValid())
	{
		double Shininess = ShininessProperty.Get<FbxDouble>();
		destMat->Shininess = static_cast<float>(Shininess);
	}
}

void FbxLoader::SetFbxColor(FBX_MATRIAL_ELEMENT& destColor, const FbxDouble3 srcColor)
{
	destColor.a = 1.0f;
	destColor.r = static_cast<float>(srcColor[0]);
	destColor.g = static_cast<float>(srcColor[1]);
	destColor.b = static_cast<float>(srcColor[2]);
}

FbxDouble3 FbxLoader::GetMaterialProperty(const FbxSurfaceMaterial* Material, const char* PropertyName, const char* FactorPropertyName, FBX_MATRIAL_ELEMENT* Element)
{
	Element->type = FBX_MATRIAL_ELEMENT::ELEMENT_NONE;

	FbxDouble3 Result(0, 0, 0);
	const FbxProperty Property = Material->FindProperty(PropertyName);
	const FbxProperty FactorProperty = Material->FindProperty(FactorPropertyName);
	if (Property.IsValid() && FactorProperty.IsValid())
	{
		Result = Property.Get<FbxDouble3>();
		double Factor = FactorProperty.Get<FbxDouble>();
		if (Factor != 1)
		{
			Result[0] *= Factor;
			Result[1] *= Factor;
			Result[2] *= Factor;
		}

		Element->type = FBX_MATRIAL_ELEMENT::ELEMENT_COLOR;
	}

	if (Property.IsValid())
	{
		int existTextureCount = 0;
		const int TextureCount = Property.GetSrcObjectCount<FbxFileTexture>();

		for (int i = 0; i < TextureCount; ++i)
		{
			FbxFileTexture* FileTexture = Property.GetSrcObject<FbxFileTexture>(i);
			if (!FileTexture)
				continue;

			FbxString uvsetName = FileTexture->UVSet.Get();
			string uvSetString = uvsetName.Buffer();
			string filepath = FileTexture->GetFileName();

			Element->textureSetArray[uvSetString].push_back(filepath);
			existTextureCount++;
		}

		const int LayeredTextureCount = Property.GetSrcObjectCount<FbxLayeredTexture>();

		for (int i = 0; i < LayeredTextureCount; ++i)
		{
			FbxLayeredTexture* LayeredTexture = Property.GetSrcObject<FbxLayeredTexture>(i);

			const int TextureFileCount = LayeredTexture->GetSrcObjectCount<FbxFileTexture>();

			for (int j = 0; j < TextureFileCount; j++)
			{
				FbxFileTexture* FileTexture = LayeredTexture->GetSrcObject<FbxFileTexture>(j);
				if (!FileTexture)
					continue;

				FbxString uvsetName = FileTexture->UVSet.Get();
				string uvSetString = uvsetName.Buffer();
				string filepath = FileTexture->GetFileName();

				Element->textureSetArray[uvSetString].push_back(filepath);
				existTextureCount++;
			}
		}

		if (existTextureCount > 0)
		{
			if (Element->type == FBX_MATRIAL_ELEMENT::ELEMENT_COLOR)
				Element->type = FBX_MATRIAL_ELEMENT::ELEMENT_BOTH;
			else
				Element->type = FBX_MATRIAL_ELEMENT::ELEMENT_TEXTURE;
		}
	}

	return Result;
}

XMMATRIX FbxLoader::GetAnimatedMatrix(unsigned int index)
{
	XMMATRIX bonematxm;

	FbxAMatrix bonemat = skeleton.joints[index].globalBindposeInverse * skeleton.joints[index].animation->globalTransform;

	bonematxm = XMMatrixTranslation(bonemat.GetT().mData[0], bonemat.GetT().mData[1], bonemat.GetT().mData[2]);
	bonematxm *= XMMatrixRotationX(bonemat.GetR().mData[0]);
	bonematxm *= XMMatrixRotationY(bonemat.GetR().mData[1]);
	bonematxm *= XMMatrixRotationZ(bonemat.GetR().mData[2]);

	return bonematxm;
}

void FbxLoader::ProcessControlPoints(FbxMesh* mesh)
{
	unsigned int ctrlPointCount = mesh->GetControlPointsCount();

	for (unsigned int i = 0; i < ctrlPointCount; ++i)
	{
		CtrlPoint* controlPoint = new CtrlPoint();
		XMFLOAT3 currPosition;
		currPosition.x = static_cast<float>(mesh->GetControlPointAt(i).mData[0]);
		currPosition.y = static_cast<float>(mesh->GetControlPointAt(i).mData[1]);
		currPosition.z = static_cast<float>(mesh->GetControlPointAt(i).mData[2]);
		controlPoint->position = currPosition;
		controlPoints.insert(make_pair(i, controlPoint));
	}
}

void FbxLoader::ProcessMesh(FbxMesh* mesh)
{
	unsigned int vertexCount = mesh->GetControlPointsCount();

	// No vertex to draw.
	if (vertexCount == 0)
	{
		return;
	}

	// If it has some defomer connection, update the vertices position
	bool hasVertexCache = mesh->GetDeformerCount(FbxDeformer::eVertexCache) && (static_cast<FbxVertexCacheDeformer*>(mesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active.Get();
	bool hasShape = mesh->GetShapeCount() > 0;
	bool hasSkin = mesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
	bool hasDeformation = hasVertexCache || hasShape || hasSkin;

	FbxVector4* vertexArray = NULL;
	if (hasDeformation)
	{
		//vertexArray = new FbxVector4[vertexCount];
		//memcpy(vertexArray, mesh->GetControlPoints(), vertexCount * sizeof(FbxVector4));
	}

	if (hasDeformation)
	{
		// Active vertex cache deformer will overwrite any other deformer
		if (hasVertexCache)
		{
			ReadVertexCacheData(mesh, timeCount, vertexArray);
		}
		else
		{
			if (hasShape)
			{
				// Deform the vertex array with the shapes.
				ComputeShapeDeformation(mesh, timeCount, currentAnimLayer, vertexArray);
			}
			//we need to get the number of clusters
			unsigned int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
			int clusterCount = 0;
			for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
			{
				clusterCount += ((FbxSkin *)(mesh->GetDeformer(skinIndex, FbxDeformer::eSkin)))->GetClusterCount();
			}
			if (clusterCount)
			{
				// Deform the vertex array with the skin deformer.
				ComputeSkinDeformation(mesh);
			}
		}
	}
	//delete[] vertexArray;
}

void FbxLoader::ReadVertexCacheData(FbxMesh* mesh, FbxTime& time, FbxVector4* vertexArray)
{
	FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(mesh->GetDeformer(0, FbxDeformer::eVertexCache));
	FbxCache*               lCache = lDeformer->GetCache();
	int                     lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
	unsigned int            lVertexCount = (unsigned int)mesh->GetControlPointsCount();
	bool                    lReadSucceed = false;
	float*                  lReadBuf = NULL;
	unsigned int			BufferSize = 0;

	if (lDeformer->Type.Get() != FbxVertexCacheDeformer::ePositions)
		// only process positions
		return;

	unsigned int Length = 0;
	lCache->Read(NULL, Length, FBXSDK_TIME_ZERO, lChannelIndex);
	if (Length != lVertexCount * 3)
		// the content of the cache is by vertex not by control points (we don't support it here)
		return;

	lReadSucceed = lCache->Read(&lReadBuf, BufferSize, time, lChannelIndex);
	if (lReadSucceed)
	{
		unsigned int lReadBufIndex = 0;

		while (lReadBufIndex < 3 * lVertexCount)
		{
			// In statements like "pVertexArray[lReadBufIndex/3].SetAt(2, lReadBuf[lReadBufIndex++])", 
			// on Mac platform, "lReadBufIndex++" is evaluated before "lReadBufIndex/3". 
			// So separate them.
			vertexArray[lReadBufIndex / 3].mData[0] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
			vertexArray[lReadBufIndex / 3].mData[1] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
			vertexArray[lReadBufIndex / 3].mData[2] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
		}
	}
}

void FbxLoader::ComputeShapeDeformation(FbxMesh* mesh, FbxTime& time, FbxAnimLayer* animLayer, FbxVector4* vertexArray)
{
	unsigned int vertexCount = mesh->GetControlPointsCount();

	FbxVector4* lSrcVertexArray = vertexArray;
	FbxVector4* lDstVertexArray = new FbxVector4[vertexCount];
	memcpy(lDstVertexArray, vertexArray, vertexCount * sizeof(FbxVector4));

	int lBlendShapeDeformerCount = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
	for (int lBlendShapeIndex = 0; lBlendShapeIndex < lBlendShapeDeformerCount; ++lBlendShapeIndex)
	{
		FbxBlendShape* lBlendShape = (FbxBlendShape*)mesh->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

		int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
		for (int lChannelIndex = 0; lChannelIndex < lBlendShapeChannelCount; ++lChannelIndex)
		{
			FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
			if (lChannel)
			{
				// Get the percentage of influence on this channel.
				FbxAnimCurve* lFCurve = mesh->GetShapeChannel(lBlendShapeIndex, lChannelIndex, animLayer);
				if (!lFCurve) continue;
				double lWeight = lFCurve->Evaluate(time);

				/*
				If there is only one targetShape on this channel, the influence is easy to calculate:
				influence = (targetShape - baseGeometry) * weight * 0.01
				dstGeometry = baseGeometry + influence

				But if there are more than one targetShapes on this channel, this is an in-between
				blendshape, also called progressive morph. The calculation of influence is different.

				For example, given two in-between targets, the full weight percentage of first target
				is 50, and the full weight percentage of the second target is 100.
				When the weight percentage reach 50, the base geometry is already be fully morphed
				to the first target shape. When the weight go over 50, it begin to morph from the
				first target shape to the second target shape.

				To calculate influence when the weight percentage is 25:
				1. 25 falls in the scope of 0 and 50, the morphing is from base geometry to the first target.
				2. And since 25 is already half way between 0 and 50, so the real weight percentage change to
				the first target is 50.
				influence = (firstTargetShape - baseGeometry) * (25-0)/(50-0) * 100
				dstGeometry = baseGeometry + influence

				To calculate influence when the weight percentage is 75:
				1. 75 falls in the scope of 50 and 100, the morphing is from the first target to the second.
				2. And since 75 is already half way between 50 and 100, so the real weight percentage change
				to the second target is 50.
				influence = (secondTargetShape - firstTargetShape) * (75-50)/(100-50) * 100
				dstGeometry = firstTargetShape + influence
				*/

				// Find the two shape indices for influence calculation according to the weight.
				// Consider index of base geometry as -1.

				int lShapeCount = lChannel->GetTargetShapeCount();
				double* lFullWeights = lChannel->GetTargetShapeFullWeights();

				// Find out which scope the lWeight falls in.
				int lStartIndex = -1;
				int lEndIndex = -1;
				for (int lShapeIndex = 0; lShapeIndex < lShapeCount; ++lShapeIndex)
				{
					if (lWeight > 0 && lWeight <= lFullWeights[0])
					{
						lEndIndex = 0;
						break;
					}
					if (lWeight > lFullWeights[lShapeIndex] && lWeight < lFullWeights[lShapeIndex + 1])
					{
						lStartIndex = lShapeIndex;
						lEndIndex = lShapeIndex + 1;
						break;
					}
				}

				FbxShape* lStartShape = NULL;
				FbxShape* lEndShape = NULL;
				if (lStartIndex > -1)
				{
					lStartShape = lChannel->GetTargetShape(lStartIndex);
				}
				if (lEndIndex > -1)
				{
					lEndShape = lChannel->GetTargetShape(lEndIndex);
				}

				//The weight percentage falls between base geometry and the first target shape.
				if (lStartIndex == -1 && lEndShape)
				{
					double lEndWeight = lFullWeights[0];
					// Calculate the real weight.
					lWeight = (lWeight / lEndWeight) * 100;
					// Initialize the lDstVertexArray with vertex of base geometry.
					memcpy(lDstVertexArray, lSrcVertexArray, vertexCount * sizeof(FbxVector4));
					for (int j = 0; j < vertexCount; j++)
					{
						// Add the influence of the shape vertex to the mesh vertex.
						FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lSrcVertexArray[j]) * lWeight * 0.01;
						lDstVertexArray[j] += lInfluence;
					}
				}
				//The weight percentage falls between two target shapes.
				else if (lStartShape && lEndShape)
				{
					double lStartWeight = lFullWeights[lStartIndex];
					double lEndWeight = lFullWeights[lEndIndex];
					// Calculate the real weight.
					lWeight = ((lWeight - lStartWeight) / (lEndWeight - lStartWeight)) * 100;
					// Initialize the lDstVertexArray with vertex of the previous target shape geometry.
					memcpy(lDstVertexArray, lStartShape->GetControlPoints(), vertexCount * sizeof(FbxVector4));
					for (int j = 0; j < vertexCount; j++)
					{
						// Add the influence of the shape vertex to the previous shape vertex.
						FbxVector4 lInfluence = (lEndShape->GetControlPoints()[j] - lStartShape->GetControlPoints()[j]) * lWeight * 0.01;
						lDstVertexArray[j] += lInfluence;
					}
				}
			}//If lChannel is valid
		}//For each blend shape channel
	}//For each blend shape deformer

	memcpy(vertexArray, lDstVertexArray, vertexCount * sizeof(FbxVector4));

	delete[] lDstVertexArray;
}

void FbxLoader::ComputeSkinDeformation(FbxMesh* mesh)
{
	FbxSkin* skinDeformer = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType skinningType = skinDeformer->GetSkinningType();

	FbxAMatrix globalPosition = mesh->GetNode()->EvaluateGlobalTransform(timeCount);
	FbxAMatrix geometryTransform = GetGeometryTransformation(mesh->GetNode());
	FbxAMatrix geometryOffset = GetGeometry(mesh->GetNode());
	FbxAMatrix globalOffPosition = globalPosition * geometryOffset;

	if (skinningType == FbxSkin::eLinear || skinningType == FbxSkin::eRigid)
	{
		ComputeLinearDeformation(mesh, globalOffPosition);
	}
	else if (skinningType == FbxSkin::eDualQuaternion)
	{
		ComputeDualQuaternionDeformation(mesh);
	}
	else if (skinningType == FbxSkin::eBlend)
	{
		int vertexCount = mesh->GetControlPointsCount();

		FbxVector4* vertexArrayLinear = new FbxVector4[vertexCount];
		memcpy(vertexArrayLinear, mesh->GetControlPoints(), vertexCount * sizeof(FbxVector4));

		FbxVector4* vertexArrayDQ = new FbxVector4[vertexCount];
		memcpy(vertexArrayDQ, mesh->GetControlPoints(), vertexCount * sizeof(FbxVector4));

		//ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayLinear, pPose);
		//ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayDQ, pPose);

		// To blend the skinning according to the blend weights
		// Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
		// DQSVertex: vertex that is deformed by dual quaternion skinning method;
		// LinearVertex: vertex that is deformed by classic linear skinning method;
		int blendWeightsCount = skinDeformer->GetControlPointIndicesCount();
		for (int BWIndex = 0; BWIndex < blendWeightsCount; ++BWIndex)
		{
			double blendWeight = skinDeformer->GetControlPointBlendWeights()[BWIndex];
			//vertexArray[BWIndex] = vertexArrayDQ[BWIndex] * blendWeight + vertexArrayLinear[BWIndex] * (1 - blendWeight);
		}
	}
}

void FbxLoader::ComputeLinearDeformation(FbxMesh* mesh, FbxAMatrix globalPosition)
{
	int vertexCount = mesh->GetControlPointsCount();

	// All the links must have the same link mode.
	FbxCluster::ELinkMode clusterMode = ((FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	CONTEXT->Map(structuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	SRVPerInstanceData*	SrvInstanceData = (SRVPerInstanceData*)MappedResource.pData;
	for (int i = 0; i < vertexCount; ++i)
	{
		F4X4Zero(SrvInstanceData[i].transform);
	}

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
	{
		FbxSkin* skinDeformer = (FbxSkin*)mesh->GetDeformer(skinIndex, FbxDeformer::eSkin);

		int clusterCount = skinDeformer->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
		{
			FbxCluster* cluster = skinDeformer->GetCluster(clusterIndex);
			if (!cluster->GetLink())
				continue;

			FbxAMatrix vertexTransformMatrix;
			ComputeClusterDeformation(globalPosition, mesh, cluster, vertexTransformMatrix, timeCount);

			int vertexIndexCount = cluster->GetControlPointIndicesCount();
			for (int k = 0; k < vertexIndexCount; ++k)
			{
				int index = cluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (index >= vertexCount)
					continue;

				double weight = cluster->GetControlPointWeights()[k];

				if (weight == 0.0)
				{
					continue;
				}

				// Compute the influence of the link on the vertex.
				FbxAMatrix influence = vertexTransformMatrix;
				MatrixScale(influence, weight);

				if (clusterMode == FbxCluster::eAdditive)
				{
					// Multiply with the product of the deformations on the vertex.
					MatrixAddToDiagonal(influence, 1.0 - weight);
					//clusterDeformation[index] = influence * clusterDeformation[index];

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					//clusterWeight[index] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					// Add to the sum of the deformations on the vertex.
					//MatrixAdd(clusterDeformation[index], influence);
					MatrixAdd(SrvInstanceData[index].transform, influence);

					// Add to the sum of weights to either normalize or complete the vertex.
					//clusterWeight[index] += weight;
					SrvInstanceData[index].transform._14 += static_cast<float>(weight);
				}
			}//For each vertex			
		}//lClusterCount
	}
	CONTEXT->Unmap(structuredBuffer, 0);

	/*
	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < vertexCount; i++)
	{
		FbxVector4 srcVertex = vertexArray[i];
		FbxVector4& dstVertex = vertexArray[i];
		double weight = clusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (weight != 0.0)
		{
			dstVertex = clusterDeformation[i].MultT(srcVertex);
			if (clusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				dstVertex /= weight;
			}
			else if (clusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				srcVertex *= (1.0 - weight);
				dstVertex += srcVertex;
			}
		}
	}

	delete[] clusterDeformation;
	delete[] clusterWeight;
	*/
}

void FbxLoader::ComputeClusterDeformation(FbxAMatrix& globalPosition, FbxMesh* mesh, FbxCluster* cluster, FbxAMatrix& vertexTransformMatrix, FbxTime time)
{
	FbxCluster::ELinkMode clusterMode = cluster->GetLinkMode();

	FbxAMatrix referenceGlobalInitPosition;
	FbxAMatrix referenceGlobalCurrentPosition;
	FbxAMatrix associateGlobalInitPosition;
	FbxAMatrix associateGlobalCurrentPosition;
	FbxAMatrix clusterGlobalInitPosition;
	FbxAMatrix clusterGlobalCurrentPosition;

	FbxAMatrix referenceGeometry;
	FbxAMatrix associateGeometry;
	FbxAMatrix clusterGeometry;

	FbxAMatrix clusterRelativeInitPosition;
	FbxAMatrix clusterRelativeCurrentPositionInverse;

	if (clusterMode == FbxCluster::eAdditive && cluster->GetAssociateModel())
	{
		cluster->GetTransformAssociateModelMatrix(associateGlobalInitPosition);
		// Geometric transform of the model
		associateGeometry = GetGeometry(cluster->GetAssociateModel());
		associateGlobalInitPosition *= associateGeometry;
		associateGlobalCurrentPosition = globalPosition;//

		cluster->GetTransformMatrix(referenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		referenceGeometry = GetGeometry(mesh->GetNode());
		referenceGlobalInitPosition *= referenceGeometry;
		referenceGlobalCurrentPosition = globalPosition;

		// Get the link initial global position and the link current global position.
		cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);
		// Multiply lClusterGlobalInitPosition by Geometric Transformation
		clusterGeometry = GetGeometry(cluster->GetLink());
		clusterGlobalInitPosition *= clusterGeometry;
		clusterGlobalCurrentPosition = cluster->GetLink()->EvaluateGlobalTransform(time);//

		// Compute the shift of the link relative to the reference.
		//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
		vertexTransformMatrix = referenceGlobalInitPosition.Inverse() * associateGlobalInitPosition * associateGlobalCurrentPosition.Inverse() *
			clusterGlobalCurrentPosition * clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition;
	}
	else
	{
		cluster->GetTransformMatrix(referenceGlobalInitPosition);
		referenceGlobalCurrentPosition = globalPosition;
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		referenceGeometry = GetGeometry(mesh->GetNode());
		referenceGlobalInitPosition *= referenceGeometry;

		// Get the link initial global position and the link current global position.
		cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);
		clusterGlobalCurrentPosition = cluster->GetLink()->EvaluateGlobalTransform(time);//

		// Compute the initial position of the link relative to the reference.
		clusterRelativeInitPosition = clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		clusterRelativeCurrentPositionInverse = referenceGlobalCurrentPosition.Inverse() * clusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		vertexTransformMatrix = clusterRelativeCurrentPositionInverse * clusterRelativeInitPosition;
	}
}

void FbxLoader::ComputeDualQuaternionDeformation(FbxMesh* mesh)
{
	/*
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
	memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		int lClusterCount = lSkinDeformer->GetClusterCount();
		for (int lClusterIndex = 0; lClusterIndex < lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

			FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
			FbxVector4 lT = lVertexTransformMatrix.GetT();
			FbxDualQuaternion lDualQuaternion(lQ, lT);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k)
			{
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
					continue;

				// Compute the influence of the link on the vertex.
				FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
				if (lClusterMode == FbxCluster::eAdditive)
				{
					// Simply influenced by the dual quaternion.
					lDQClusterDeformation[lIndex] = lInfluence;

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					if (lClusterIndex == 0)
					{
						lDQClusterDeformation[lIndex] = lInfluence;
					}
					else
					{
						// Add to the sum of the deformations on the vertex.
						// Make sure the deformation is accumulated in the same rotation direction. 
						// Use dot product to judge the sign.
						double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());
						if (lSign >= 0.0)
						{
							lDQClusterDeformation[lIndex] += lInfluence;
						}
						else
						{
							lDQClusterDeformation[lIndex] -= lInfluence;
						}
					}
					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < lVertexCount; i++)
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeightSum = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeightSum != 0.0)
		{
			lDQClusterDeformation[i].Normalize();
			lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);

			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeightSum;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeightSum);
				lDstVertex += lSrcVertex;
			}
		}
	}

	delete[] lDQClusterDeformation;
	delete[] lClusterWeight;
	*/
}

FbxAMatrix FbxLoader::GetGeometry(FbxNode * node)
{
	const FbxVector4 T = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 R = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 S = node->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(T, R, S);
}

void FbxLoader::ProcessVertex(FbxMesh* mesh)
{
	if (!mesh)
		return;

	indexCount = 0;
	int polygonCount = mesh->GetPolygonCount();
	int maxLayer = mesh->GetLayerCount();
	FbxLayerElementMaterial* layerMaterial = mesh->GetLayer(0)->GetMaterials();
	FbxLayerElementArrayTemplate<int>* mtrlArray = &layerMaterial->GetIndexArray();

	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(0);

	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		int controlPointsCount = mesh->GetControlPointsCount();
		vertices.resize(controlPointsCount);
		indices.resize(polygonCount * 3);

		vector<Vertex>::iterator iter = vertices.begin();
		map<int, int> testMap;
		for (int index = 0; index < controlPointsCount; ++index)
		{
			Vertex tempVertex;
			//tempVertex.position.x = vertexArray[index].mData[0];
			//tempVertex.position.y = vertexArray[index].mData[1];
			//tempVertex.position.z = vertexArray[index].mData[2];
			tempVertex.position = controlPoints[index]->position;
			ReadTexUV(mesh, index, indexCount, tempVertex.texUV);
			ReadNormal(mesh, index, indexCount, tempVertex.normal);
			ReadTangent(mesh, index, indexCount, tempVertex.tangent);
			ReadBinormal(mesh, index, indexCount, tempVertex.binormal);
			tempVertex.ctrlPtNum = index;
			vertices[index] = tempVertex;

			iter++;
		}
		for (int i = 0; i < polygonCount; ++i)
		{
			int polygonsize = mesh->GetPolygonSize(i);

			for (int pol = 0; pol < polygonsize; ++pol)
			{
				UINT index = mesh->GetPolygonVertex(i, pol);
				vertices[index].texNum = mtrlArray->GetAt(i);
				indices[i * 3 + pol] = index;

				indexCount++;
			}
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		vertices.resize(polygonCount * 3);
		indices.resize(polygonCount * 3);

		vector<Vertex>::iterator iter = vertices.begin();
		for (int i = 0; i < polygonCount; ++i)
		{
			int polygonsize = mesh->GetPolygonSize(i);

			for (int pol = 0; pol < polygonsize; ++pol)
			{
				Vertex tempVertex;
				UINT index = mesh->GetPolygonVertex(i, pol);
				tempVertex.position = controlPoints[index]->position;
				ReadTexUV(mesh, index, indexCount, tempVertex.texUV);
				ReadNormal(mesh, index, indexCount, tempVertex.normal);
				ReadTangent(mesh, index, indexCount, tempVertex.tangent);
				ReadBinormal(mesh, index, indexCount, tempVertex.binormal);
				tempVertex.ctrlPtNum = index;
				tempVertex.texNum = mtrlArray->GetAt(i);
				vertices[i * 3 + pol] = tempVertex;
				indices[i * 3 + pol] = indexCount;

				iter++;
				indexCount++;
			}
		}
	}
	break;
	}
	/*
	if (!mesh)
		return;

	int polygonCount = mesh->GetPolygonCount();
	indexCount = 0;
	int maxLayer = mesh->GetLayerCount();
	FbxLayerElementMaterial* layerMaterial = mesh->GetLayer(0)->GetMaterials();
	FbxLayerElementArrayTemplate<int>* mtrlArray = &layerMaterial->GetIndexArray();

	for (int i = 0; i < polygonCount; ++i)
	{
		int polygonsize = mesh->GetPolygonSize(i);

		for (int pol = 0; pol < polygonsize; ++pol)
		{
			Vertex tempVertex;
			UINT index = mesh->GetPolygonVertex(i, pol);
			//tempVertex.position = controlPoints[index]->position;
			tempVertex.position.x = vertexArray[index].mData[0];
			tempVertex.position.y = vertexArray[index].mData[1];
			tempVertex.position.z = vertexArray[index].mData[2];
			tempVertex.texNum = mtrlArray->GetAt(i);
			if (mtrlArray->GetAt(i) > 7 || mtrlArray->GetAt(i) < 0)
			{
				int stop = 0;
			}
			tempVertex.ctrlPtNum = index;
			ReadTexUV(mesh, index, indexCount, tempVertex.texUV);
			ReadNormal(mesh, index, indexCount, tempVertex.normal);
			ReadTangent(mesh, index, indexCount, tempVertex.tangent);
			ReadBinormal(mesh, index, indexCount, tempVertex.binormal);

			vertices.push_back(tempVertex);
			indices.push_back(indexCount);

			indexCount++;
		}
	}
	int stop2 = 0;
	*/
}

void FbxLoader::ProcessJointsAndAnimations(FbxMesh* mesh)
{
	unsigned int numOfDeformers = mesh->GetDeformerCount();

	FbxAMatrix geometryTransform = GetGeometryTransformation(mesh->GetNode());

	for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
	{
		FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		if (!currSkin)
		{
			continue;
		}

		unsigned int numOfClusters = currSkin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
		{
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);

			string currJointName = currCluster->GetLink()->GetName();
			unsigned int currJointIndex = FindJointIndexUsingName(currJointName);
			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindposeInverseMatrix;

			currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
			currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;
			
			// Update the information in mSkeleton 
			skeleton.joints[currJointIndex].globalBindposeInverse = globalBindposeInverseMatrix;
			skeleton.joints[currJointIndex].node = currCluster->GetLink();

			// Associate each joint with the control points it affects
			unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
			for (unsigned int i = 0; i < numOfIndices; ++i)
			{
				BlendingIndexWeightPair currBlendingIndexWeightPair;
				currBlendingIndexWeightPair.blendingIndex = currJointIndex;
				currBlendingIndexWeightPair.blendingWeight = currCluster->GetControlPointWeights()[i];
				controlPoints[currCluster->GetControlPointIndices()[i]]->blendingInfo.push_back(currBlendingIndexWeightPair);
			}

			KeyFrame** currAnim = &skeleton.joints[currJointIndex].animation;
			
			for (FbxLongLong i = start.GetFrameCount(scene->GetGlobalSettings().GetTimeMode()); i <= stop.GetFrameCount(scene->GetGlobalSettings().GetTimeMode()); ++i)
			{
				FbxTime currTime;
				currTime.SetFrame(i, scene->GetGlobalSettings().GetTimeMode());
				*currAnim = new KeyFrame();
				(*currAnim)->frameNum = i;
				FbxAMatrix currentTransformOffset = mesh->GetNode()->EvaluateGlobalTransform(currTime) * geometryTransform;
				FbxAMatrix finalTransform = currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
				(*currAnim)->globalTransform = finalTransform;
				currAnim = &((*currAnim)->next);
			}
			skeleton.joints[currJointIndex].currAnim = skeleton.joints[currJointIndex].animation;
		}
	}

	BlendingIndexWeightPair currBlendingIndexWeightPair;
	currBlendingIndexWeightPair.blendingIndex = 0;
	currBlendingIndexWeightPair.blendingWeight = 0;
	for (auto itr = controlPoints.begin(); itr != controlPoints.end(); ++itr)
	{
		for (unsigned int i = itr->second->blendingInfo.size(); i < 4; ++i)
		{
			itr->second->blendingInfo.push_back(currBlendingIndexWeightPair);
		}
	}
}

void FbxLoader::ProcessAnimation()
{
	FbxTakeInfo *takeInfo = scene->GetTakeInfo(*(AnimStackNameArray[0]));
	start = takeInfo->mLocalTimeSpan.GetStart();
	stop = takeInfo->mLocalTimeSpan.GetStop();
	FrameTime.SetTime(0, 0, 0, 1, 0, FbxTime::EMode::eFrames60);// scene->GetGlobalSettings().GetTimeMode());
	timeCount = start;

	FbxAnimStack* AnimationStack = scene->FindMember<FbxAnimStack>(AnimStackNameArray[0]->Buffer());
	currentAnimLayer = AnimationStack->GetMember<FbxAnimLayer>();
	scene->SetCurrentAnimationStack(AnimationStack);
}

void FbxLoader::ProcessAMatrix(FbxAMatrix& mat)
{
	FbxVector4 translation = mat.GetT();
	FbxVector4 rotation = mat.GetR();
	translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]); 
	rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]); 
	mat.SetT(translation);
	mat.SetR(rotation);
}

unsigned int FbxLoader::FindJointIndexUsingName(string jointName)
{
	int index = 0;
	for (; index < skeleton.joints.size(); ++index)
	{
		if (strcmp(skeleton.joints[index].name, jointName.c_str()) == 0) break;
	}
	return index;
}

void FbxLoader::ReadTexUV(FbxMesh* mesh, unsigned int controlPointIndex, unsigned int vertexCounter, XMFLOAT2& outUV)
{
	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(0);
	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			outUV.y = 1 - static_cast<float>(vertexUV->GetDirectArray().GetAt(controlPointIndex).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(controlPointIndex);
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			outUV.y = 1 - static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(vertexCounter).mData[0]);
			outUV.y = 1 - static_cast<float>(vertexUV->GetDirectArray().GetAt(vertexCounter).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(vertexCounter);
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			outUV.y = 1 - static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FbxLoader::ReadNormal(FbxMesh* mesh, unsigned int controlPointIndex, unsigned int vertexCounter, XMFLOAT3& outNormal)
{
	FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(vertexCounter);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FbxLoader::ReadTangent(FbxMesh* mesh, unsigned int controlPointIndex, unsigned int vertexCounter, XMFLOAT3& outTangent)
{
	FbxGeometryElementTangent* vertexTangent = mesh->GetElementTangent(0);
	if (vertexTangent)
	{
		switch (vertexTangent->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(controlPointIndex).mData[0]);
				outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(controlPointIndex).mData[1]);
				outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(controlPointIndex).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexTangent->GetIndexArray().GetAt(controlPointIndex);
				outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
				outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
				outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(vertexCounter).mData[0]);
				outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(vertexCounter).mData[1]);
				outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(vertexCounter).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexTangent->GetIndexArray().GetAt(vertexCounter);
				outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
				outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
				outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;
		}
	}
}

void FbxLoader::ReadBinormal(FbxMesh* mesh, unsigned int controlPointIndex, unsigned int vertexCounter, XMFLOAT3& outBinormal)
{
	FbxGeometryElementBinormal* vertexBinormal = mesh->GetElementBinormal(0);
	if (vertexBinormal)
	{
		switch (vertexBinormal->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (vertexBinormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
				outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
				outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexBinormal->GetIndexArray().GetAt(controlPointIndex);
				outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
				outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
				outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;

		case FbxGeometryElement::eByPolygonVertex:
			switch (vertexBinormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(vertexCounter).mData[0]);
				outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(vertexCounter).mData[1]);
				outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(vertexCounter).mData[2]);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexBinormal->GetIndexArray().GetAt(vertexCounter);
				outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
				outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
				outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;

			default:
				throw std::exception("Invalid Reference");
			}
			break;
		}
	}
}

void FbxLoader::ProcessSkeletalHeirarchy(FbxNode* rootnode)
{
	for (int childindex = 0; childindex < rootnode->GetChildCount(); ++childindex)
	{
		FbxNode* node = rootnode->GetChild(childindex);
		ProcessSkeletalHeirarchyRecursively(node, 0, 0, -1);
	}
}

void FbxLoader::ProcessSkeletalHeirarchyRecursively(FbxNode* node, int depth, int index, int parentIndex)
{
	if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Joint joint;
		joint.parentIndex = parentIndex;
		joint.name = node->GetName();
		skeleton.joints.push_back(joint);
	}
	for (int i = 0; i < node->GetChildCount(); i++)
	{
		ProcessSkeletalHeirarchyRecursively(node->GetChild(i), depth + 1, skeleton.joints.size(), index);
	}
}

void FbxLoader::CreateConstantBuffers(ID3D11Buffer** buffer, UINT bufferSize)
{
	D3D11_BUFFER_DESC bd = {};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = bufferSize;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DEVICE->CreateBuffer(&bd, NULL, buffer);
}

void FbxLoader::CreateStructuredBuffer()
{
	const uint32_t count = static_cast<uint32_t>(controlPoints.size());
	const uint32_t stride = static_cast<uint32_t>(sizeof(SRVPerInstanceData));

	// Create StructuredBuffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = stride * count;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = stride;
	DEVICE->CreateBuffer(&bd, NULL, &structuredBuffer);

	// Create ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.NumElements = count;

	DEVICE->CreateShaderResourceView(structuredBuffer, &srvDesc, &transformSRV);
}

void FbxLoader::UpdateCameraBuffer()
{
	FBX_CAMERA_BUFFER fcb;
	fcb.world = world;
	fcb.lightDir = XMFLOAT4(-1, -1, 1, 0);
	XMStoreFloat4x4(&fcb.view, XMMatrixTranspose(CAMERA->GetView()));
	XMStoreFloat4x4(&fcb.projection, XMMatrixTranspose(CAMERA->GetProjection()));
	CONTEXT->UpdateSubresource(cameraBuffer, 0, NULL, &fcb, 0, 0);
}

void FbxLoader::UpdatePixelBuffer()
{
	FBX_PIXEL_BUFFER fpb;
	fpb.ambient.x = materialArray[0].ambient.r;
	fpb.ambient.y = materialArray[0].ambient.g;
	fpb.ambient.z = materialArray[0].ambient.b;
	fpb.ambient.w = materialArray[0].ambient.a;
	fpb.diffuse.x = materialArray[0].diffuse.r;
	fpb.diffuse.y = materialArray[0].diffuse.g;
	fpb.diffuse.z = materialArray[0].diffuse.b;
	fpb.diffuse.w = materialArray[0].diffuse.a;
	fpb.specular.x = materialArray[0].specular.r;
	fpb.specular.y = materialArray[0].specular.g;
	fpb.specular.z = materialArray[0].specular.b;
	fpb.power = materialArray[0].Shininess;
	fpb.emmisive.x = materialArray[0].emmisive.r;
	fpb.emmisive.y = materialArray[0].emmisive.g;
	fpb.emmisive.z = materialArray[0].emmisive.b;
	fpb.emmisive.w = materialArray[0].emmisive.a;
	CONTEXT->UpdateSubresource(pixelBuffer, 0, NULL, &fpb, 0, 0);
	int count = materialArray.size();
	for (int i = 0; i < materialArray.size(); ++i)
	{
		TextureSet::const_iterator it = materialArray[i].diffuse.textureSetArray.begin();
		if (it->second.size())
		{
			string path = it->second[0];
			WCHAR	wstr[512];
			size_t wLen = 0;
			mbstowcs_s(&wLen, wstr, path.size() + 1, path.c_str(), _TRUNCATE);
			textureDir.push_back(wstr);
			TEXTURE(wstr);
		}
	}
}

void FbxLoader::Update()
{
	timeCount += FrameTime;
	if (timeCount > stop) timeCount = start;
	if (KEYBOARD->KeyDown(VK_RIGHT))
	{
		timeCount += FrameTime;
		if (timeCount > stop) timeCount = start;
	}

	UpdateCameraBuffer();
	//UpdateBoneBuffer();
	ProcessMesh(meshNode->GetMesh());
}

void FbxLoader::Render()
{
	for (int i = 0; i < textureDir.size(); ++i)
	{
		CONTEXT->PSSetShaderResources(i, 1, TEXTURE(textureDir[i]));
	}
	CONTEXT->VSSetShaderResources(10, 1, &transformSRV);
	CONTEXT->PSSetSamplers(0, 1, SAMPLER);
	CONTEXT->VSSetConstantBuffers(0, 1, &cameraBuffer);
	//CONTEXT->VSSetConstantBuffers(1, 1, &boneBuffer);
	CONTEXT->PSSetConstantBuffers(0, 1, &cameraBuffer);
	CONTEXT->PSSetConstantBuffers(1, 1, &pixelBuffer);
	shader->RenderFbx(indexCount);
}
