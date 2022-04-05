#pragma once
#include "Application/Layers/PostProcessingLayer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture1D.h"

class ToonShaderEffect : public PostProcessingLayer::Effect {
public:
	MAKE_PTRS(ToonShaderEffect);
	Texture1D::Sptr toonLut;

	ToonShaderEffect();
	ToonShaderEffect(bool defaultLut);
	virtual ~ToonShaderEffect();							 

	virtual void Apply(const Framebuffer::Sptr& gBuffer) override;
	virtual void RenderImGui() override;

	// Inherited from IResource

	ToonShaderEffect::Sptr FromJson(const nlohmann::json& data);
	virtual nlohmann::json ToJson() const override;

protected:
	ShaderProgram::Sptr _shader;
};

