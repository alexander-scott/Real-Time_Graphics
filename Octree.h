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

private:
	void Grow(XMFLOAT3 direction);
	void Shrink();

	int GetRootPosIndex(int xDir, int yDir, int zDir);

	int					mObjectCount;
	OctreeNode*			mRootNode;

	float				mInitialSize;
	float				mMinimumNodeSize;
};

