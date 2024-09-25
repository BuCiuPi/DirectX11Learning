#include "SkinnedData.h"

KeyFrame::KeyFrame()
	:TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

KeyFrame::~KeyFrame()
{
}

float BoneAnimation::GetStartTime() const
{
	return keyFrames.front().TimePos;
}

float BoneAnimation::GetEndTime() const
{
	if (keyFrames.size() == 0)
	{
		return 0;
	}
	float f = keyFrames.back().TimePos;

	return f;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M) const
{
	if (keyFrames.size() == 0)
	{
		XMStoreFloat4x4(&M, XMMatrixIdentity());
		return;
	}

	if (t <= keyFrames.front().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&keyFrames.front().Scale);
		XMVECTOR P = XMLoadFloat3(&keyFrames.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&keyFrames.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if (t >= keyFrames.back().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&keyFrames.back().Scale);
		XMVECTOR P = XMLoadFloat3(&keyFrames.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&keyFrames.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for (int i = 0; i < keyFrames.size() - 1; ++i)
		{
			if (t >= keyFrames[i].TimePos && t <= keyFrames[i + 1].TimePos)
			{
				float lerpPercent = (t - keyFrames[i].TimePos) / (keyFrames[i + 1].TimePos - keyFrames[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&keyFrames[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&keyFrames[i + 1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&keyFrames[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&keyFrames[i + 1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&keyFrames[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&keyFrames[i + 1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime() const
{
	float t = MathHelper::Infinity;
	for (int i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Max(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime() const
{
	float t = 0.0f;
	for (int i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Max(t, BoneAnimations[i].GetEndTime());
	}
	
	return t;
}

void AnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms) const
{
	for (int i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

UINT SkinnedData::BoneCount() const
{
	return mBoneHierarchy.size();
}

float SkinnedData::GetClipStartTime(const std::string& clipName) const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName) const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipEndTime();
}

float SkinnedData::GetFirstClipEndTime() const
{
	auto clip = mAnimations.begin();
	return clip->second.GetClipEndTime();
}

void SkinnedData::Set(std::vector<BoneInfo>& boneHierarchy,	std::map<std::string, AnimationClip>& animations)
{
	this->mBoneHierarchy = boneHierarchy;
	this->mAnimations = animations;
}

void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos,
	std::vector<XMFLOAT4X4>& finalTransforms) const
{
	UINT numBones = mBoneHierarchy.size();
	std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	auto clip = mAnimations.find(clipName);
	clip->second.Interpolate(timePos, toParentTransforms);

	std::vector<XMFLOAT4X4> toRootTransforms(numBones);
	toRootTransforms[0] = toParentTransforms[0];

	for (int i = 0; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i].HierarchyID;
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	for (int i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneHierarchy[i].boneOffset);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);

		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
	}
}

void SkinnedData::GetFirstClipFinalTransforms(float timePos,
	std::vector<XMFLOAT4X4>& finalTransforms) const
{
	UINT numBones = mBoneHierarchy.size();
	std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	auto clip = mAnimations.begin();
	clip->second.Interpolate(timePos, toParentTransforms);

	std::vector<XMFLOAT4X4> toRootTransforms(numBones);
	toRootTransforms[0] = toParentTransforms[0];

	for (int i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i].HierarchyID;
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	for (int i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneHierarchy[i].boneOffset);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);

		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
	}
}