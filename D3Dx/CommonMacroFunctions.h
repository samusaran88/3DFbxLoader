#include "stdafx.h"

inline void F4X4Identity(XMFLOAT4X4& f44)
{
	f44 = { 1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 };
}

inline void F4X4Zero(XMFLOAT4X4& f44)
{
	f44 = { 0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0 };
}

inline FbxAMatrix GetGeometryTransformation(FbxNode* node)
{
	const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

inline XMVECTOR XMV(XMFLOAT3& f) { return XMLoadFloat3(&f); }
inline XMMATRIX XMM(XMFLOAT4X4& m) { return XMLoadFloat4x4(&m); }

inline void FBXAMatrixToFloat4X4(FbxAMatrix* src, XMFLOAT4X4* dest)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			dest->m[i][j] = static_cast<float>(src->Get(i, j));
		}
	}
}
// Scale all the elements of a matrix.
inline void MatrixScale(FbxAMatrix& pMatrix, double pValue)
{
	int i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			pMatrix[i][j] *= pValue;
		}
	}
}

inline void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue)
{
	pMatrix[0][0] += pValue;
	pMatrix[1][1] += pValue;
	pMatrix[2][2] += pValue;
	pMatrix[3][3] += pValue;
}

// Sum two matrices element by element.
inline void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix)
{
	int i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			pDstMatrix[i][j] += pSrcMatrix[i][j];
		}
	}
}
inline void MatrixAdd(XMFLOAT4X4& pDstMatrix, FbxAMatrix& pSrcMatrix)
{
	int i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			pDstMatrix.m[i][j] += static_cast<float>(pSrcMatrix[i][j]);
		}
	}
}