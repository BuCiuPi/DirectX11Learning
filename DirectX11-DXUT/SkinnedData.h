#ifndef SKINEDDATA_H
#define SKINEDDATA_H

#include <map>

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
	XMFLOAT4X4 boneOffset;
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

private:
	std::vector<BoneInfo> mBoneHierarchy;
	std::map<std::string, AnimationClip> mAnimations;

};

#endif

