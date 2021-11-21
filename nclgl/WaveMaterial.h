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

	void      SetReflectionTex(GLuint  tex) { reflectionTex = tex; }
	GLuint    GetReflectionTex()  const { return  reflectionTex; }

	void      SetRefractionTex(GLuint  tex) { refractionTex = tex; }
	GLuint    GetRefractionTex()  const { return  refractionTex; }

	void      SetDuDvTex(GLuint  tex) { dudvTex = tex; }
	GLuint    GetDuDvTe()  const { return  dudvTex; }

protected:
	float speed;
	float amplitude;
	float waveLength;

	GLuint refractionTex;
	GLuint reflectionTex;
	GLuint dudvTex;

};

