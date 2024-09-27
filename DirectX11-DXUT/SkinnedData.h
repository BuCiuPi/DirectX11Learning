#ifndef SKINEDDATA_H
#define SKINEDDATA_H

#include <map>
#include <assimp/scene.h>

#include "D3DUtil.h"

struct KeyFrame
{
	KeyFrame();
	~KeyFrame();

	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

struct BoneAnimation
{
	float GetStartTime() const;
	float GetEndTime() const;

	void Interpolate(float t, XMFLOAT4X4& M) const;

	std::string Name;
	std::vector<KeyFrame> keyFrames;
};

struct AnimationClip
{
	float GetClipStartTime() const;
	float GetClipEndTime() const;

	void Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms) const;

	std::vector<BoneAnimation> BoneAnimations;
};

struct BoneInfo
{
	int HierarchyID;
	std::string BoneName;

	XMMATRIX boneOffset;
	XMMATRIX localTransform;
	XMMATRIX globalTransform;
	XMMATRIX originalLocalTransform;
	XMMATRIX originalGlobalTransform;
	BoneInfo* Parent = nullptr;

	std::vector<BoneInfo> Childrents;

	XMMATRIX FinalTransform;
};

class SkinnedData
{
public:
	UINT BoneCount() const;
	float GetClipStartTime(const std::string& clipName) const;
	float GetClipEndTime(const std::string& clipName) const;
	float GetFirstClipEndTime() const;

	void Set(std::vector<BoneInfo>& boneHierarchy, std::map<std::string, AnimationClip>
	         & animations);

	void GetFinalTransforms(const std::string& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms) const;
	void GetFirstClipFinalTransforms(float timePos, std::vector<XMFLOAT4X4>& finalTransforms) const;
	void GetBoneTransforms(float TimeInSeconds);
	void ReadNodeHierarchy(float AnimationTimeTicks, int currentIndex);
	void LoadMatrix(XMFLOAT4X4& matrixOut, const aiMatrix4x4& matrixIn);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string& NodeName);
	UINT FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
	UINT FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
	KeyFrame CalcInterpolated(float t, const BoneAnimation& boneAnimation);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
	UINT FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
	int GetBoneIndexByName(std::string name, const std::vector<BoneInfo>& boneInfos);

	const aiScene* pScene = NULL;
	XMMATRIX m_GlobalInverseTransform;
	std::vector<BoneInfo> mBoneHierarchy;
private:
	std::map<std::string, AnimationClip> mAnimations;
};

#endif

