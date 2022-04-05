#include "Bloom.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"


Bloom::Bloom() :
	PostProcessingLayer::Effect()
{
	Name = "Underwhelming Bloom";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/JAFBloom.glsl" }
	});
}

Bloom::~Bloom() = default;

void Bloom::Apply(const Framebuffer::Sptr& gBuffer)
{
	_shader->Bind();
}

void Bloom::RenderImGui()
{
	
}

Bloom::Sptr Bloom::FromJson(const nlohmann::json& data)
{
	Bloom::Sptr result = std::make_shared<Bloom>();
	result->Enabled = JsonGet(data, "enabled", true);
	result->_strength = JsonGet(data, "strength", result->_strength);
	return result;
}

nlohmann::json Bloom::ToJson() const
{
	return {
		{ "enabled", Enabled },
		{ "strength", _strength }
	};
}
