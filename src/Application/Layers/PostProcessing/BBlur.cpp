#include "BBlur.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Graphics/Framebuffer.h"

#include <GLM/glm.hpp>

BBlur::BBlur() :
	PostProcessingLayer::Effect()
{
	Name = "BBlur";
	_format = RenderTargetType::ColorRgb8;

	// Zero the memory, then set center pixel to 1.0
	memset(Filter, 0, sizeof(float) * 9);
	Filter[4] = 1.0f;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/JAFBBlur.glsl" }
	});
}

BBlur::~BBlur() = default;

void BBlur::Apply(const Framebuffer::Sptr& gBuffer)
{
	_shader->Bind(); 
}

void BBlur::RenderImGui()
{
	
}

BBlur::Sptr BBlur::FromJson(const nlohmann::json& data)
{
	BBlur::Sptr result = std::make_shared<BBlur>();
	result->Enabled = JsonGet(data, "enabled", true);
	std::vector<float> filter = JsonGet(data, "filter", std::vector<float>(9, 0.0f));
	for (int ix = 0; ix < 9; ix++) {
		result->Filter[ix] = filter[ix];
	}
	return result;
}

nlohmann::json BBlur::ToJson() const
{
	std::vector<float> filter;
	for (int ix = 0; ix < 9; ix++) {
		filter.push_back(Filter[ix]);
	}
	return {
		{ "enabled", Enabled },
		{ "filter", filter }
	};
}
