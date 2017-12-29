#include "OctreeNode.h"

OctreeNode::OctreeNode(float baseLengthVal, float minSizeVal, XMFLOAT3 centerVal)
{
	SetValues(baseLengthVal, minSizeVal, centerVal);
}

OctreeNode::~OctreeNode()
{
}

bool OctreeNode::AddObject(OctreeItem &obj)
{
	if (!mNodeBounds.Intersects(obj.Bounds))
	{
		return false;
	}

	SubAdd(obj);
	return true;
}

bool OctreeNode::RemoveObject(OctreeItem &obj)
{
	bool objRemoved = false;

	for (int i = 0; i < mObjects.size(); i++)
	{
		if (mObjects[i].GameObject == obj.GameObject)
		{
			mObjects.erase(mObjects.begin() + i);
			objRemoved = true;
			break;
		}
	}

	if (!objRemoved && mChildNodes.size() != 0)
	{
		for (int i = 0; i < 8; i++)
		{
			objRemoved = mChildNodes[i]->RemoveObject(obj);
			if (objRemoved)
			{
				break;
			}
		}
	}

	if (objRemoved && mChildNodes.size() != 0)
	{
		if (CheckMergeNodes())
		{
			MergeNodes();
		}
	}

	return objRemoved;
}

OctreeNode * OctreeNode::ShrinkOctree(float minSideLength)
{
	if (mNodeSideLength < (2 * minSideLength))
	{
		return this;
	}

	if (mObjects.size() == 0 && mChildNodes.size() == 0)
	{
		return this;
	}

	int bestFit = -1;
	for (int i = 0; i < mObjects.size(); i++)
	{
		OctreeItem currentObj = mObjects[i];
		int newBestFit = BestFitChild(currentObj);
		if (i == 0 || newBestFit == bestFit)
		{
			if (bestFit < 0)
			{
				bestFit = newBestFit;
			}
		}
		else
		{
			return this;
		}
	}

	if (mChildNodes.size() != 0)
	{
		bool childHadContent = false;

		for (int i = 0; i < mChildNodes.size(); i++)
		{
			if (mChildNodes[i]->ContainsObjects())
			{
				if (childHadContent)
				{
					return this;
				}

				if (bestFit >= 0 && bestFit != i)
				{
					return this;
				}

				childHadContent = true;
				bestFit = i;
			}
		}
	}

	if (mChildNodes.size() == 0)
	{
		SetValues(mNodeSideLength / 2, mMinimumNodeSize, mChildNodeBounds[bestFit].Center);
		return this;
	}

	return mChildNodes[bestFit];
}

void OctreeNode::SetValues(float baseLengthVal, float minSizeVal, XMFLOAT3 centerVal)
{
	mNodeSideLength = baseLengthVal;
	mMinimumNodeSize = minSizeVal;
	mOrigin = centerVal;

	mActualBoundsSize = XMFLOAT3(mNodeSideLength, mNodeSideLength, mNodeSideLength);
	mNodeBounds = BoundingBox(mOrigin, mActualBoundsSize);

	float quarter = mNodeSideLength / 4;
	float childActualLength = mNodeSideLength / 2;

	XMFLOAT3 childActualSize = XMFLOAT3(childActualLength, childActualLength, childActualLength);

	mChildNodeBounds.clear();

	// 0
	XMFLOAT3 temp = XMFLOAT3(-quarter, quarter, -quarter);
	XMFLOAT3 newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodeBounds.push_back(BoundingBox(newOrigin, childActualSize));

	// 1
	temp = XMFLOAT3(quarter, quarter, -quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodeBounds.push_back(BoundingBox(newOrigin, childActualSize));

	// 2
	temp = XMFLOAT3(-quarter, quarter, quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodeBounds.push_back(BoundingBox(newOrigin, childActualSize));

	// 3
	temp = XMFLOAT3(quarter, quarter, quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodeBounds.push_back(BoundingBox(newOrigin, childActualSize));

	// 4
	temp = XMFLOAT3(-quarter, -quarter, -quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodeBounds.push_back(BoundingBox(newOrigin, childActualSize));

	// 5
	temp = XMFLOAT3(quarter, -quarter, -quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodeBounds.push_back(BoundingBox(newOrigin, childActualSize));

	// 6
	temp = XMFLOAT3(-quarter, -quarter, quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodeBounds.push_back(BoundingBox(newOrigin, childActualSize));

	// 7
	temp = XMFLOAT3(quarter, -quarter, quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodeBounds.push_back(BoundingBox(newOrigin, childActualSize));
}

void OctreeNode::SetChildNodes(std::vector<OctreeNode*> childNodes)
{
	if (childNodes.size() != 8)
	{
		return;
	}

	mChildNodes.insert(mChildNodes.end(), childNodes.begin(), childNodes.end());
}

void OctreeNode::SubAdd(OctreeItem &obj)
{
	if (mObjects.size() < kOctreeNodeMaxObjects || mNodeSideLength / 2 < mMinimumNodeSize)
	{
		mObjects.push_back(obj);
	}
	else
	{
		int bestFitChild;
		if (mChildNodes.size() == 0)
		{
			Split();
			if (mChildNodes.size() == 0)
			{
				return;
			}

			for (int i = mObjects.size() - 1; i >= 0; i--)
			{
				OctreeItem existingObj = mObjects[i];
				bestFitChild = BestFitChild(existingObj);

				mChildNodes[bestFitChild]->SubAdd(existingObj);
				mObjects.erase(mObjects.begin() + i);
			}
		}

		bestFitChild = BestFitChild(obj);
		mChildNodes[bestFitChild]->SubAdd(obj);
	}
}

void OctreeNode::Split()
{
	float quarter = mNodeSideLength / 4;
	float newLength = mNodeSideLength / 2;

	mChildNodes.clear();

	// 0
	XMFLOAT3 temp = XMFLOAT3(-quarter, quarter, -quarter);
	XMFLOAT3 newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodes.push_back(new OctreeNode(newLength, mMinimumNodeSize, newOrigin));

	// 1
	temp = XMFLOAT3(quarter, quarter, -quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodes.push_back(new OctreeNode(newLength, mMinimumNodeSize, newOrigin));

	// 2
	temp = XMFLOAT3(-quarter, quarter, quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodes.push_back(new OctreeNode(newLength, mMinimumNodeSize, newOrigin));

	// 3
	temp = XMFLOAT3(quarter, quarter, quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodes.push_back(new OctreeNode(newLength, mMinimumNodeSize, newOrigin));

	// 4
	temp = XMFLOAT3(-quarter, -quarter, -quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodes.push_back(new OctreeNode(newLength, mMinimumNodeSize, newOrigin));

	// 5
	temp = XMFLOAT3(quarter, -quarter, -quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodes.push_back(new OctreeNode(newLength, mMinimumNodeSize, newOrigin));

	// 6
	temp = XMFLOAT3(-quarter, -quarter, quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodes.push_back(new OctreeNode(newLength, mMinimumNodeSize, newOrigin));

	// 7
	temp = XMFLOAT3(quarter, -quarter, quarter);
	newOrigin = XMFLOAT3(mOrigin.x + temp.x, mOrigin.y + temp.y, mOrigin.z + temp.z);
	mChildNodes.push_back(new OctreeNode(newLength, mMinimumNodeSize, newOrigin));
}

bool OctreeNode::ContainsObjects()
{
	if (mObjects.size() > 0)
	{
		return true;
	}

	if (mChildNodes.size() != 0)
	{
		for (int i = 0; i < 8; i++)
		{
			if (mChildNodes[i]->ContainsObjects())
			{
				return true;
			}
		}
	}

	return false;
}

bool OctreeNode::CheckMergeNodes()
{
	int totalObjects = mObjects.size();
	if (mChildNodes.size() != 0)
	{
		for (auto child : mChildNodes)
		{
			if (child != nullptr)
			{
				return false;
			}
			totalObjects += child->mObjects.size();
		}
	}
	return totalObjects <= kOctreeNodeMaxObjects;
}

void OctreeNode::MergeNodes()
{
	for (int i = 0; i < 8; i++)
	{
		OctreeNode* childNode = mChildNodes[i];
		int numObjects = childNode->mObjects.size();
		for (int j = numObjects - 1; j >= 0; j--)
		{
			OctreeItem item = childNode->mObjects[i];
			mObjects.push_back(item);
		}
	}

	mChildNodes.clear();
}

bool OctreeNode::IntersectsBounds(BoundingBox &bounds1, XMFLOAT3 rayOrigin, XMFLOAT3 rayDir)
{
	float tmin = (bounds1.Center.x - bounds1.Extents.x - rayOrigin.x) / rayDir.x;
	float tmax = (bounds1.Center.x + bounds1.Extents.x - rayOrigin.x) / rayDir.x;

	if (tmin > tmax) swap(tmin, tmax);

	/*float tymin = (bounds1.Min.y - rayOrigin.y) / rayDir.y;
	float tymax = (bounds1.Max.y - rayOrigin.y) / rayDir.y;

	if (tymin > tymax) swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;*/

	float tzmin = (bounds1.Center.z - bounds1.Extents.z - rayOrigin.z) / rayDir.z;
	float tzmax = (bounds1.Center.z + bounds1.Extents.z - rayOrigin.z) / rayDir.z;

	if (tzmin > tzmax) swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	return true;
}

int OctreeNode::BestFitChild(OctreeItem &obj)
{
	int xVal = (obj.GameObject->GetPosition().x <= mOrigin.x ? 0 : 1);
	int yVal = (obj.GameObject->GetPosition().y >= mOrigin.y ? 0 : 4);
	int zVal = (obj.GameObject->GetPosition().z <= mOrigin.z ? 0 : 2);
	return xVal + yVal + zVal;
}

void OctreeNode::GetGameObjectsInBounds(std::vector<GameObject*> &gameObjects, BoundingBox &b)
{
	for (auto& go : mObjects)
	{
		if (go.Bounds.Intersects(b))
		{
			gameObjects.push_back(go.GameObject);
		}
	}

	if (mChildNodes.size() != 0)
	{
		for (auto& node : mChildNodes)
		{
			if (node->mNodeBounds.Intersects(b))
			{
				node->GetGameObjectsInBounds(gameObjects, b);
			}
		}
	}
}

void OctreeNode::GetGameObjectsInRay(std::vector<GameObject*> &gameObjects, XMFLOAT3 rayOrigin, XMFLOAT3 rayDir)
{
	for (auto& go : mObjects)
	{
		if (IntersectsBounds(go.Bounds, rayOrigin, rayDir))
		{
			gameObjects.push_back(go.GameObject);
		}
	}

	if (mChildNodes.size() != 0)
	{
		for (auto& node : mChildNodes)
		{
			if (IntersectsBounds(node->mNodeBounds, rayOrigin, rayDir))
			{
				node->GetGameObjectsInRay(gameObjects, rayOrigin, rayDir);
			}
		}
	}
}

void OctreeNode::GetGameObjectsInFrustum(std::vector<GameObject*>& gameObjects, BoundingFrustum& frustum)
{
	for (auto& go : mObjects)
	{
		if (frustum.Intersects(go.Bounds))
		{
			gameObjects.push_back(go.GameObject);
		}
	}

	if (mChildNodes.size() != 0)
	{
		for (auto& node : mChildNodes)
		{
			if (frustum.Intersects(node->mNodeBounds))
			{
				node->GetGameObjectsInFrustum(gameObjects, frustum);
			}
		}
	}
}
