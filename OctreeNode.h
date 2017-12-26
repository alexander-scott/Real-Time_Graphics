#pragma once

#include "Structures.h"
#include "GameObject.h"

struct OctreeItem
{
public:
	Bounds Bounds;
	GameObject* GameObject;
};

class OctreeNode
{
public:
	OctreeNode(float baseLengthVal, float minSizeVal, XMFLOAT3 centerVal);
	~OctreeNode();

	bool AddObject(OctreeItem obj);
	bool RemoveObject(OctreeItem obj);

	void SetChildNodes(std::vector<OctreeNode*> childNodes);
	OctreeNode* ShrinkOctree(float minSideLength);

	XMFLOAT3					mOrigin;
	float						mNodeSideLength;

private:
	void SetValues(float baseLengthVal, float minSizeVal, XMFLOAT3 centerVal);
	void SubAdd(OctreeItem obj);

	void Split();

	bool ContainsObjects();

	bool CheckMergeNodes();
	void MergeNodes();

	bool IntersectsBounds(Bounds bounds1, Bounds bounds2);
	int BestFitChild(OctreeItem obj);

	float						mMinimumNodeSize;
	Bounds						mNodeBounds;
	std::vector<Bounds>			mChildNodeBounds;
	XMFLOAT3					mActualBoundsSize;

	std::vector<OctreeItem>		mObjects;
	std::vector<OctreeNode*>	mChildNodes;
};

