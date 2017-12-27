#pragma once

#include "Structures.h"
#include "GameObject.h"

struct OctreeItem
{
public:
	BoundingBox Bounds;
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

	void GetGameObjectsInBounds(std::vector<GameObject*> &gameObjects, BoundingBox b);
	void GetGameObjectsInRay(std::vector<GameObject*> &gameObjects, XMFLOAT3 rayOrigin, XMFLOAT3 rayDir);
	void GetGameObjectsInFrustum(std::vector<GameObject*> &gameObjects, BoundingFrustum frustum);

	XMFLOAT3					mOrigin;
	float						mNodeSideLength;

private:
	void SetValues(float baseLengthVal, float minSizeVal, XMFLOAT3 centerVal);
	void SubAdd(OctreeItem obj);

	void Split();

	bool ContainsObjects();

	bool CheckMergeNodes();
	void MergeNodes();

	bool IntersectsBounds(BoundingBox bounds1, BoundingBox bounds2);
	bool IntersectsBounds(BoundingBox bounds1, XMFLOAT3 rayOrigin, XMFLOAT3 rayDir);
	bool IntersectsBounds(BoundingBox bounds1, std::vector<XMFLOAT4> &frustums);
	int BestFitChild(OctreeItem obj);

	float						mMinimumNodeSize;
	BoundingBox					mNodeBounds;
	std::vector<BoundingBox>	mChildNodeBounds;
	XMFLOAT3					mActualBoundsSize;

	std::vector<OctreeItem>		mObjects;
	std::vector<OctreeNode*>	mChildNodes;
};

