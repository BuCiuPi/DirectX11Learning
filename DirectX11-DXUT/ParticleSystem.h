#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "D3DUtil.h"
#include <string>
#include <vector>

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	float GetAge() const;

	void SetEyePos(const XMFLOAT3& eyePosW);
	void SetEmitPos(const XMFLOAT3& emitPosW);
	void SetEmitDir(const XMFLOAT3& emitDirW);
	void SetAccelerate(const XMFLOAT3& accelW);
	void SetBlendState(ID3D11BlendState* blendState);

	void Init(ID3D11Device* device, ID3D11ShaderResourceView* texArraySRV, ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles);

	void Reset();
	void Update(float dt, float gameTime);
	void Draw(ID3D11DeviceContext* dc, const Camera& cam);

	void BuildFX(ID3D11Device* device, LPCTSTR DrawShaderFileName, LPCTSTR StreamOutShaderFileName);

private:
	void BuildVB(ID3D11Device* device);
	bool BuildDrawFX(ID3D11Device* device, LPCTSTR DrawShaderFileName);
	void BuildStreamOutFX(ID3D11Device* device, LPCTSTR StreamOutShaderFileName);


	ParticleSystem(const ParticleSystem& rhs);
	ParticleSystem& operator=(const ParticleSystem& rhs);

private:

	UINT mMaxParticle;
	bool mFirstRun;

	float mGameTime;
	float mTimeStep;
	float mAge;


	XMFLOAT3 mEyePosW;
	XMFLOAT3 mEmitPosW;
	XMFLOAT3 mEmitDirW;

	XMFLOAT3 mAccelW;

	ID3D11VertexShader* StreamOutVS;
	ID3D11GeometryShader* StreamOutGS;

	ID3D11VertexShader* DrawVS;
	ID3D11GeometryShader* DrawGS;
	ID3D11PixelShader* DrawPS;
	
	ID3D11Buffer* mInitVB;
	ID3D11Buffer* mDrawVB;
	ID3D11Buffer* mStreamOutVB;

	ID3D11Buffer* mConstantBuffer;
	ID3D11Buffer* mPerFrameBuffer;

	ID3D11SamplerState* mSamplerLinear;

	ID3D11ShaderResourceView* mTexArraySRV;
	ID3D11ShaderResourceView* mRandomTexSRV;
	ID3D11BlendState* mBlendState;
};

#endif


