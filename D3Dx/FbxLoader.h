#pragma once

#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API	
#endif

typedef tr1::unordered_map<string, vector<string>> TextureSet;

struct FBX_MATRIAL_ELEMENT
{
	enum MATERIAL_ELEMENT_TYPE
	{
		ELEMENT_NONE = 0,
		ELEMENT_COLOR,
		ELEMENT_TEXTURE,
		ELEMENT_BOTH,
		ELEMENT_MAX,
	};

	MATERIAL_ELEMENT_TYPE type;
	float r, g, b, a;
	TextureSet textureSetArray;

	FBX_MATRIAL_ELEMENT()
	{
		textureSetArray.clear();
	}

	~FBX_MATRIAL_ELEMENT()
	{
		Release();
	}

	void Release()
	{
		for (TextureSet::iterator it = textureSetArray.begin(); it != textureSetArray.end(); ++it)
		{
			it->second.clear();
		}

		textureSetArray.clear();
	}
};

struct FBX_MATERIAL_NODE
{
	enum eMATERIAL_TYPE
	{
		MATERIAL_LAMBERT = 0,
		MATERIAL_PHONG,
	};

	eMATERIAL_TYPE type;
	FBX_MATRIAL_ELEMENT ambient;
	FBX_MATRIAL_ELEMENT diffuse;
	FBX_MATRIAL_ELEMENT emmisive;
	FBX_MATRIAL_ELEMENT specular;

	float Shininess;
	float TransparencyFactor;		
};

struct BlendingIndexWeightPair
{
	UINT blendingIndex;
	double blendingWeight;
};

struct CtrlPoint
{
	XMFLOAT3 position;
	vector<BlendingIndexWeightPair> blendingInfo;
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 texUV;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 binormal;
	XMFLOAT4 weight;
	XMUINT4 bones;
	UINT texNum;
	UINT ctrlPtNum;
};

struct KeyFrame
{
	FbxLongLong frameNum;
	FbxAMatrix globalTransform;
	KeyFrame* next;

	KeyFrame() : next(nullptr)
	{
	}
};

struct Joint
{
	int parentIndex;
	const char* name;
	FbxAMatrix globalBindposeInverse;
	KeyFrame* animation;
	KeyFrame* currAnim;
	FbxNode* node;

	Joint() :
		node(nullptr),
		animation(nullptr)
	{
		globalBindposeInverse.SetIdentity();
		parentIndex = -1;
	}

	~Joint()
	{
		while (animation)
		{
			KeyFrame* temp = animation->next;
			delete animation;
			animation = temp;
		}
	}
};

struct Skeleton
{
	vector<Joint> joints;
};

struct FBX_CAMERA_BUFFER
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4 lightDir;
};

struct FBX_PIXEL_BUFFER
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT3 specular;
	float power;
	XMFLOAT4 emmisive;
};

struct SRVPerInstanceData
{
	XMFLOAT4X4 transform;
};

class FbxLoader
{
private:
	FbxManager*		sdkManager;
	FbxScene*		scene;
	FbxImporter*	importer;
	FbxNode*		meshNode;

	FbxAnimLayer*	currentAnimLayer;

	FbxTime start;
	FbxTime stop;
	FbxTime FrameTime;
	FbxTime timeCount;
	FbxArray<FbxString*> AnimStackNameArray;

	vector<wstring>					textureDir;
	vector<Vertex>					vertices;
	vector<WORD>					indices;
	unsigned int					indexCount;
	map<unsigned int, CtrlPoint*>	controlPoints;
	vector<FBX_MATERIAL_NODE>		materialArray;
	Skeleton						skeleton;

	Shader* shader;
	ID3D11Buffer*				cameraBuffer;
	ID3D11Buffer*				boneBuffer;
	ID3D11Buffer*				pixelBuffer;
	ID3D11Buffer*				structuredBuffer;
	ID3D11ShaderResourceView*	transformSRV;

	XMFLOAT4X4 world;
public:
	FbxLoader();
	~FbxLoader();

	void LoadFbx(string FileName);
	void InitializeSdkObjects(FbxManager*& Manager, FbxScene*& Scene);
	void TriangulateRecursive(FbxNode* Node);
	void Setup();
	void SetupNode(FbxNode* Node);

	void CopyMatrialData(FbxSurfaceMaterial* mat, FBX_MATERIAL_NODE* destMat);
	void SetFbxColor(FBX_MATRIAL_ELEMENT& destColor, const FbxDouble3 srcColor);
	FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial* Material, const char* PropertyName, const char* FactorPropertyName, FBX_MATRIAL_ELEMENT* Element);

	XMMATRIX GetAnimatedMatrix(unsigned int index);

	void ProcessControlPoints(FbxMesh* mesh);
	void ProcessMesh(FbxMesh* mesh);
	void ReadVertexCacheData(FbxMesh* mesh, FbxTime& time, FbxVector4* vertexArray);
	void ComputeShapeDeformation(FbxMesh* mesh, FbxTime& time, FbxAnimLayer* animLayer, FbxVector4* vertexArray);
	void ComputeSkinDeformation(FbxMesh* mesh);
	void ComputeLinearDeformation(FbxMesh* mesh, FbxAMatrix globalPosition);
	void ComputeClusterDeformation(FbxAMatrix& globalPosition, FbxMesh* mesh, FbxCluster* cluster, FbxAMatrix& vertexTransformMatrix, FbxTime time);
	void ComputeDualQuaternionDeformation(FbxMesh* mesh);
	FbxAMatrix GetGeometry(FbxNode* node);
	void ProcessVertex(FbxMesh* mesh);
	void ProcessJointsAndAnimations(FbxMesh* mesh);
	void ProcessAnimation();
	void ProcessAMatrix(FbxAMatrix& mat);
	unsigned int FindJointIndexUsingName(string jointName);
	void ReadTexUV(FbxMesh* mesh, unsigned int controlPointIndex, unsigned int vertexCounter, XMFLOAT2& outUV);
	void ReadNormal(FbxMesh* mesh, unsigned int controlPointIndex, unsigned int vertexCounter, XMFLOAT3& outNormal);
	void ReadTangent(FbxMesh* mesh, unsigned int controlPointIndex, unsigned int vertexCounter, XMFLOAT3& outTangent);
	void ReadBinormal(FbxMesh* mesh, unsigned int controlPointIndex, unsigned int vertexCounter, XMFLOAT3& outBinormal);
	void ProcessSkeletalHeirarchy(FbxNode* rootnode);
	void ProcessSkeletalHeirarchyRecursively(FbxNode* node, int depth, int index, int parentIndex);

	void CreateConstantBuffers(ID3D11Buffer** buffer, UINT bufferSize);
	void CreateStructuredBuffer();
	void UpdateCameraBuffer();
	void UpdatePixelBuffer();

	void Update();
	void Render();
};