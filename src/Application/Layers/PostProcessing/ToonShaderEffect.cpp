//Our attempt at trying to implment a toon shader through post processing.
#include "ToonShaderEffect.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

ToonShaderEffect::ToonShaderEffect() :
	ToonShaderEffect(true) { }

ToonShaderEffect::ToonShaderEffect(bool defaultLut) :
	PostProcessingLayer::Effect(),
	_shader(nullptr),
	toonLut(nullptr)
{
	Name = "Toon Shader";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/cel_shader.glsl" }
	});
	
	if (defaultLut) {
		toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon1-1D.png");
		toonLut->SetWrap(WrapMode::ClampToEdge);
	}
}

ToonShaderEffect::~ToonShaderEffect() = default;

void ToonShaderEffect::Apply(const Framebuffer::Sptr & gBuffer)
{
	_shader->Bind();
	toonLut->Bind(1);
}

void ToonShaderEffect::RenderImGui()
{
	LABEL_LEFT(ImGui::LabelText, "LUT", toonLut ? toonLut->GetDebugName().c_str() : "none");
}

ToonShaderEffect::Sptr ToonShaderEffect::FromJson(const nlohmann::json & data)
{
	ToonShaderEffect::Sptr result = std::make_shared<ToonShaderEffect>(false);
	result->Enabled = JsonGet(data, "enabled", true);
	result->toonLut = ResourceManager::Get<Texture1D>(Guid(data["lut"].get<std::string>()));
	return result;
}

nlohmann::json ToonShaderEffect::ToJson() const
{
	return {
		{ "enabled", Enabled },
		{ "lut", toonLut != nullptr ? toonLut->GetGUID().str() : "null" }
	};
}