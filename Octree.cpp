#include "Octree.h"



Octree::Octree(float initialWorldSize, XMFLOAT3 initialWorldPos, float minNodeSize)
{
	if (minNodeSize > initialWorldSize)
	{
		minNodeSize = initialWorldSize;
	}

	mObjectCount = 0;
	mInitialSize = initialWorldSize;
	mMinimumNodeSize = minNodeSize;
	mRootNode = new OctreeNode(mInitialSize, mMinimumNodeSize, initialWorldPos);
}


Octree::~Octree()
{
}

void Octree::Add(OctreeItem obj)
{
	int count = 0;

	while (!mRootNode->AddObject(obj))
	{
		// Dir: obj to rootnode
		Grow(XMFLOAT3(obj.GameObject->GetPosition().x - mRootNode->mOrigin.x, obj.GameObject->GetPosition().y - mRootNode->mOrigin.y, obj.GameObject->GetPosition().z - mRootNode->mOrigin.z));

		if (count++ > 20)
		{
			return;
		}
	}

	mObjectCount++;
}

bool Octree::Remove(OctreeItem obj)
{
	bool removed = mRootNode->RemoveObject(obj);

	if (removed)
	{
		mObjectCount--;
		Shrink();
	}

	return removed;
}

void Octree::Grow(XMFLOAT3 direction)
{
	int xDirection = direction.x >= 0 ? 1 : -1;
	int yDirection = direction.y >= 0 ? 1 : -1;
	int zDirection = direction.z >= 0 ? 1 : -1;

	OctreeNode* oldRoot = mRootNode;
	float half = mRootNode->mNodeSideLength / 2;
	float newLength = mRootNode->mNodeSideLength * 2;

	XMFLOAT3 moveDir = XMFLOAT3(xDirection * half, yDirection * half, zDirection * half);
	XMFLOAT3 newCenter = XMFLOAT3(mRootNode->mOrigin.x + moveDir.x, mRootNode->mOrigin.y + moveDir.y, mRootNode->mOrigin.z + moveDir.z);

	mRootNode = new OctreeNode(newLength, mMinimumNodeSize, newCenter);

	int rootPos = GetRootPosIndex(xDirection, yDirection, zDirection);
	std::vector<OctreeNode*> children;

	for (int i = 0; i < 8; i++)
	{
		if (i == rootPos)
		{
			children.push_back(oldRoot);
		}
		else
		{
			xDirection = i % 2 == 0 ? -1 : 1;
			yDirection = i > 3 ? -1 : 1;
			zDirection = (i < 2 || (i > 3 && i < 6)) ? -1 : 1;
			XMFLOAT3 moveDir = XMFLOAT3(xDirection * half, yDirection * half, zDirection * half);
			XMFLOAT3 newCenter2 = XMFLOAT3(mRootNode->mOrigin.x + moveDir.x, mRootNode->mOrigin.y + moveDir.y, mRootNode->mOrigin.z + moveDir.z);

			children.push_back(new OctreeNode(mRootNode->mNodeSideLength, mMinimumNodeSize, XMFLOAT3(newCenter.x + newCenter2.x, newCenter.y + newCenter2.y, newCenter.z + newCenter2.z)));
		}
	}

	mRootNode->SetChildNodes(children);
}

void Octree::Shrink()
{
	mRootNode = mRootNode->ShrinkOctree(mInitialSize);
}

int Octree::GetRootPosIndex(int xDir, int yDir, int zDir)
{
	int result = xDir > 0 ? 1 : 0;
	if (yDir < 0) result += 4;
	if (zDir > 0) result += 2;
	return result;
}

std::vector<GameObject*> Octree::GetGameObjectsInBounds(BoundingBox b)
{
	std::vector<GameObject*> returnVector;
	mRootNode->GetGameObjectsInBounds(returnVector, b);
	return returnVector;
}

std::vector<GameObject*> Octree::GetGameObjectsInRay(XMFLOAT3 rayOrigin, XMFLOAT3 rayDir)
{
	std::vector<GameObject*> returnVector;
	mRootNode->GetGameObjectsInRay(returnVector, rayOrigin, rayDir);
	return returnVector;
}

std::vector<GameObject*> Octree::GetGameObjectsInBoundingFrustum(BoundingFrustum &frustum)
{
	std::vector<GameObject*> returnVector;
	mRootNode->GetGameObjectsInFrustum(returnVector, frustum);
	return returnVector;
}
