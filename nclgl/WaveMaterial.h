#pragma once
#include "Material.h"
class WaveMaterial : public Material
{
public:
	WaveMaterial();
	~WaveMaterial();

	virtual void PassShaderUniforms();

	float GetSpeed() {return speed;}
	void SetSpeed(float s) { speed = s; }

	float GetAmplitude() { return amplitude; }
	void SetAmplitude(float s) { amplitude = s; }

	float GetWaveLength() { return waveLength; }
	void SetWaveLength(float s) { waveLength = s; }

	void  SetDepthTexture(bool  pass) { passDepthTexture = pass; }
	bool    GetDepthTexture()  const { return  passDepthTexture; }

protected:
	float speed;
	float amplitude;
	float waveLength;

	bool passDepthTexture;
};

