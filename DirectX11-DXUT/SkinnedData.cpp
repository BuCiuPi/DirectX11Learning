#include "SkinnedData.h"

using namespace std;

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

void SkinnedData::Set(std::vector<BoneInfo>& boneHierarchy, std::map<std::string, AnimationClip>& animations)
{
	this->mBoneHierarchy = boneHierarchy;
	this->mAnimations = animations;
}

//void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos,
//	std::vector<XMFLOAT4X4>& finalTransforms) const
//{
//	UINT numBones = mBoneHierarchy.size();
//	std::vector<XMFLOAT4X4> toParentTransforms(numBones);
//
//	auto clip = mAnimations.find(clipName);
//	clip->second.Interpolate(timePos, toParentTransforms);
//
//	std::vector<XMFLOAT4X4> toRootTransforms(numBones);
//	toRootTransforms[0] = toParentTransforms[0];
//
//	for (int i = 0; i < numBones; ++i)
//	{
//		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
//
//		int parentIndex = mBoneHierarchy[i].HierarchyID;
//		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);
//
//		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);
//
//		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
//	}
//
//	for (int i = 0; i < numBones; ++i)
//	{
//		XMMATRIX offset = mBoneHierarchy[i].boneOffset;
//		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
//
//		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
//	}
//}
//
//void SkinnedData::GetFirstClipFinalTransforms(float timePos,
//	std::vector<XMFLOAT4X4>& finalTransforms) const
//{
//	UINT numBones = mBoneHierarchy.size();
//	std::vector<XMFLOAT4X4> toParentTransforms(numBones);
//
//	//auto clip = mAnimations.begin();
//	//clip->second.Interpolate(timePos, toParentTransforms);
//
//	for (int i = 0; i < numBones; ++i)
//	{
//		toParentTransforms[i] = mBoneHierarchy[i].transformMatrix;
//	}
//
//	std::vector<XMFLOAT4X4> toRootTransforms(numBones);
//	toRootTransforms[0] = toParentTransforms[0];
//
//	for (int i = 1; i < numBones; ++i)
//	{
//		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
//
//		int parentIndex = mBoneHierarchy[i].HierarchyID;
//		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);
//
//		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);
//
//		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
//	}
//
//	for (int i = 0; i < numBones; ++i)
//	{
//		XMMATRIX offset = mBoneHierarchy[i].boneOffset;
//		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
//
//		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
//	}
//}

void SkinnedData::GetBoneTransforms(float TimeInSeconds)
{
	ReadNodeHierarchy(TimeInSeconds, 0);
}

void SkinnedData::ReadNodeHierarchy(float AnimationTimeTicks, int currentIndex)
{
	BoneInfo* curBoneInfo = &mBoneHierarchy[currentIndex];

	AnimationClip pNodeAnim = mAnimations.begin()->second;
	BoneAnimation boneAnim = pNodeAnim.BoneAnimations[currentIndex];

	XMMATRIX AnimTransMat;
	KeyFrame keyFrame;
	if (boneAnim.keyFrames.size() > 0) {
		// Interpolate scaling and generate scaling transformation matrix
		keyFrame = CalcInterpolated(AnimationTimeTicks, boneAnim);
		XMMATRIX ScalingM = XMMatrixScaling(keyFrame.Scale.x, keyFrame.Scale.y, keyFrame.Scale.z);
		// Interpolate rotation and generate rotation transformation matrix

		XMVECTOR quat = XMLoadFloat4(&keyFrame.RotationQuat);
		quat = XMQuaternionNormalize(quat);
		//XMMATRIX RotationM = XMMatrixRotationQuaternion(XMVectorSet(keyFrame.RotationQuat.x, keyFrame.RotationQuat.y, keyFrame.RotationQuat.z, keyFrame.RotationQuat.w));
		XMMATRIX RotationM = XMMatrixRotationQuaternion(quat);
		// Interpolate translation and generate translation transformation matrix
		XMMATRIX TranslationM = XMMatrixTranslation(keyFrame.Translation.x, keyFrame.Translation.y, keyFrame.Translation.z);
		// Combine the above transformations
		AnimTransMat = XMMatrixMultiply(XMMatrixMultiply(RotationM, ScalingM), TranslationM);
	}

	//XMVECTOR det = XMMatrixDeterminant((AnimTransMat));
	//XMMATRIX inv = XMMatrixInverse(&det, XMMatrixTranspose(AnimTransMat));
	curBoneInfo->localTransform = (AnimTransMat);

	int curParentIndex = curBoneInfo->HierarchyID;
	curBoneInfo->globalTransform = curBoneInfo->localTransform;

	if (currentIndex == 0)
	{
		curBoneInfo->globalTransform = curBoneInfo->originalLocalTransform;
	}

	while (curParentIndex > -1)
	{
		curBoneInfo->globalTransform *= mBoneHierarchy[curParentIndex].localTransform;
		curParentIndex = mBoneHierarchy[curParentIndex].HierarchyID;
	}

	curBoneInfo->FinalTransform = curBoneInfo->boneOffset * curBoneInfo->globalTransform * m_GlobalInverseTransform;

	for (UINT i = 0; i < mBoneHierarchy.size(); i++) {
		if (mBoneHierarchy[i].HierarchyID == currentIndex)
		{
			ReadNodeHierarchy(AnimationTimeTicks, i);
		}
	}
}

void SkinnedData::LoadMatrix(XMFLOAT4X4& matrixOut, const aiMatrix4x4& matrixIn)
{
	matrixOut(0, 0) = matrixIn.a1;
	matrixOut(0, 1) = matrixIn.a2;
	matrixOut(0, 2) = matrixIn.a3;
	matrixOut(0, 3) = matrixIn.a4;

	matrixOut(1, 0) = matrixIn.b1;
	matrixOut(1, 1) = matrixIn.b2;
	matrixOut(1, 2) = matrixIn.b3;
	matrixOut(1, 3) = matrixIn.b4;

	matrixOut(2, 0) = matrixIn.c1;
	matrixOut(2, 1) = matrixIn.c2;
	matrixOut(2, 2) = matrixIn.c3;
	matrixOut(2, 3) = matrixIn.c4;

	matrixOut(3, 0) = matrixIn.d1;
	matrixOut(3, 1) = matrixIn.d2;
	matrixOut(3, 2) = matrixIn.d3;
	matrixOut(3, 3) = matrixIn.d4;
}

const aiNodeAnim* SkinnedData::FindNodeAnim(const aiAnimation* pAnimation, const string& NodeName)
{
	for (int i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}

UINT SkinnedData::FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (UINT i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
		if (AnimationTimeTicks < t) {
			return i;
		}
	}

	return 0;
}

UINT SkinnedData::FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (UINT i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		float t = (float)pNodeAnim->mRotationKeys[i + 1].mTime;
		if (AnimationTimeTicks < t) {
			return i;
		}
	}

	return 0;
}

KeyFrame SkinnedData::CalcInterpolated(float t, const BoneAnimation& boneAnimation)
{

	KeyFrame keyFrame;
	if (boneAnimation.keyFrames.size() == 0)
	{
		return keyFrame;
	}

	//if (t <= boneAnimation.keyFrames.front().TimePos)
	//{
	return boneAnimation.keyFrames.front();
	//}
	//else if (t >= boneAnimation.keyFrames.back().TimePos)
	//{
	//	return boneAnimation.keyFrames.back();
	//}
	//else
	//{
	//	for (int i = 0; i < boneAnimation.keyFrames.size() - 1; ++i)
	//	{
	//		if (t >= boneAnimation.keyFrames[i].TimePos && t <= boneAnimation.keyFrames[i + 1].TimePos)
	//		{
	//			float lerpPercent = (t - boneAnimation.keyFrames[i].TimePos) / (boneAnimation.keyFrames[i + 1].TimePos - boneAnimation.keyFrames[i].TimePos);

	//			XMVECTOR s0 = XMLoadFloat3(&boneAnimation.keyFrames[i].Scale);
	//			XMVECTOR s1 = XMLoadFloat3(&boneAnimation.keyFrames[i + 1].Scale);

	//			XMVECTOR p0 = XMLoadFloat3(&boneAnimation.keyFrames[i].Translation);
	//			XMVECTOR p1 = XMLoadFloat3(&boneAnimation.keyFrames[i + 1].Translation);

	//			XMVECTOR q0 = XMLoadFloat4(&boneAnimation.keyFrames[i].RotationQuat);
	//			XMVECTOR q1 = XMLoadFloat4(&boneAnimation.keyFrames[i + 1].RotationQuat);

	//			XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
	//			XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
	//			XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

	//			XMStoreFloat3(&keyFrame.Scale, S);
	//			XMStoreFloat3(&keyFrame.Translation, P);
	//			XMStoreFloat4(&keyFrame.RotationQuat, Q);

	//			return keyFrame;
	//		}
	//	}
	//}
}
void SkinnedData::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	UINT RotationIndex = FindRotation(AnimationTimeTicks, pNodeAnim);
	UINT NextRotationIndex = RotationIndex + 1;
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float t1 = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
	float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
	float DeltaTime = t2 - t1;
	float Factor = (AnimationTimeTicks - t1) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out.Normalize();
}

UINT SkinnedData::FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
	for (UINT i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
		if (AnimationTimeTicks < t) {
			return i;
		}
	}

	return 0;
}

void SkinnedData::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	UINT PositionIndex = FindPosition(AnimationTimeTicks, pNodeAnim);
	UINT NextPositionIndex = PositionIndex + 1;
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
	float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
	float DeltaTime = t2 - t1;
	float Factor = (AnimationTimeTicks - t1) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

int SkinnedData::GetBoneIndexByName(std::string name, const std::vector<BoneInfo>& boneInfos)
{
	for (int i = 0; i < boneInfos.size(); ++i)
	{
		if (boneInfos[i].BoneName == name)
		{
			return i;
		}
	}

	return -1;
}