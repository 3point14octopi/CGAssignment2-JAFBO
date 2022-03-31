#include "Gameplay/Components/SkinManager.h"
#include "Gameplay/Components/RenderComponent.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/InputEngine.h"

void SkinManager::Awake()
{
	if (!_skins.size()) {
		_skins.push_back(GetComponent<RenderComponent>()->GetMaterial());
		

		if (_skins.back() == nullptr) {
			IsEnabled = false;
		}
		else {
			_defaultSkin = _skins.back();
			_currentSkin = _defaultSkin;
			_currentIndex = 0;
			_numberOfSkins++;
		}
	}
	else {
		GetComponent<RenderComponent>()->SetMaterial(_currentSkin);
	}	
}

void SkinManager::RenderImGui() {
	
}

nlohmann::json SkinManager::ToJson() const {
	nlohmann::json result;
	result["number_ofskins"] = _numberOfSkins;
	for (int i = 0; i < _numberOfSkins; i++) {
		std::string skinName = "skin_" + std::to_string(i);
		result[skinName] = _skins[i]->GetGUID().str();
	}
	result["default_skin"] = (_defaultSkin) ? _defaultSkin->GetGUID().str() : "null";
	result["current_skin"] = (_currentSkin) ? _currentSkin->GetGUID().str() : "null";
	result["skin-dex"] = (_currentIndex);
	return result;
}

SkinManager::SkinManager() :
	IComponent(),
	_defaultSkin(nullptr),
	_currentSkin(nullptr)
{ }

SkinManager::~SkinManager() = default;

SkinManager::Sptr SkinManager::FromJson(const nlohmann::json& blob) {
	SkinManager::Sptr result = std::make_shared<SkinManager>();
	result->_numberOfSkins = blob["number_ofskins"];
	for (int i = 0; i < result->_numberOfSkins; i++) {
		std::string blobIndex = "skin_" + std::to_string(i);
		result->_skins.push_back(ResourceManager::Get<Gameplay::Material>(Guid(blob[blobIndex].get<std::string>())));
	}
	result->_defaultSkin = ResourceManager::Get<Gameplay::Material>(Guid(blob["default_skin"].get<std::string>()));
	result->_currentSkin = ResourceManager::Get<Gameplay::Material>(Guid(blob["current_skin"].get<std::string>()));
	result->_currentIndex = blob["skin-dex"];
	return result;
}

void SkinManager::Update(float deltaTime) {
	if (!cooldown) {
		if (InputEngine::GetKeyState(GLFW_KEY_RIGHT) == ButtonState::Pressed) {
			_currentIndex = (_currentIndex < (_numberOfSkins - 1)) ? _currentIndex + 1 : 0;
			ApplySkin();
		}
		else if (InputEngine::GetKeyState(GLFW_KEY_LEFT) == ButtonState::Pressed) {
			_currentIndex = (_currentIndex > 0) ? _currentIndex - 1 : (_numberOfSkins - 1);
			ApplySkin();
		}
	}
	else {
		cooldown--;
	}
}

/// <summary>
/// Gives the hand access to a new material
/// </summary>
/// <param name="newSkin">
/// :  the new material 
/// </param>
void SkinManager::AddSkin(Gameplay::Material::Sptr newSkin) {
	_skins.push_back(newSkin);
	_numberOfSkins++;
}

/// <summary>
/// Applies the currently selected material to the hand
/// </summary>
void SkinManager::ApplySkin() {
	_currentSkin = _skins[_currentIndex];
	GetComponent<RenderComponent>()->SetMaterial(_currentSkin);
	cooldown = 40;
}
