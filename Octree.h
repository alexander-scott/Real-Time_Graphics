#pragma once

#include "Structures.h"
#include "OctreeNode.h"

class Octree
{
public:
	Octree(float initialWorldSize, XMFLOAT3 initialWorldPos, float minNodeSize);
	~Octree();

	void Add(OctreeItem obj);
	bool Remove(OctreeItem obj);

	std::vector<GameObject*> GetGameObjectsInBounds(BoundingBox b);
	std::vector<GameObject*> GetGameObjectsInRay(XMFLOAT3 rayOrigin, XMFLOAT3 rayDir);
	std::vector<GameObject*> GetGameObjectsInBoundingFrustum(BoundingFrustum frustum);

private:
	void Grow(XMFLOAT3 direction);
	void Shrink();

	int GetRootPosIndex(int xDir, int yDir, int zDir);

	int					mObjectCount;
	OctreeNode*			mRootNode;

	float				mInitialSize;
	float				mMinimumNodeSize;
};

