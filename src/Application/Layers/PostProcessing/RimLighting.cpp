#include "RimLighting.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"


RimLighting::RimLighting() :
	PostProcessingLayer::Effect()
{
	Name = "bad_rim";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/rim_vert.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/rim_lighting.glsl" }
	});
}

RimLighting::~RimLighting() = default;

void RimLighting::Apply(const Framebuffer::Sptr& gBuffer)
{
	_shader->Bind();
}

void RimLighting::RenderImGui()
{
	
}

RimLighting::Sptr RimLighting::FromJson(const nlohmann::json& data)
{
	RimLighting::Sptr result = std::make_shared<RimLighting>();
	result->Enabled = JsonGet(data, "enabled", true);
	result->_strength = JsonGet(data, "strength", result->_strength);
	return result;
}

nlohmann::json RimLighting::ToJson() const
{
	return {
		{ "enabled", Enabled },
		{ "strength", _strength }
	};
}
