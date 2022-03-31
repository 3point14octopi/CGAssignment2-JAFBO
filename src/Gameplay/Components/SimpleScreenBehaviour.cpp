#include "Gameplay/Components/SimpleScreenBehaviour.h"
#include "Gameplay/Components/InterpolationBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"

#include "Gameplay/GameObject.h"

#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/InputEngine.h"

void SimpleScreenBehaviour::Update(float deltaTime) {
	if (active && !isMoving) {
		if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed) {
			if (objectivesAchieved < targetObjectives) {
				GetGameObject()->Get<InterpolationBehaviour>()->ToggleBehaviour("Lowering", false);
				GetGameObject()->Get<InterpolationBehaviour>()->PauseOrResumeCurrentBehaviour();
				isMoving = true;
			}
			else {
				(GetGameObject()->Get<RenderComponent>())->SetMaterial(WinScreen);
				active = false;
			}
		}
	}
	else if (isMoving) {
		if (!GetGameObject()->Get<InterpolationBehaviour>()->_isRunning) {
			isMoving = false;
			active = false;
		}
	}
}

void SimpleScreenBehaviour::RenderImGui() {
	
}

nlohmann::json SimpleScreenBehaviour::ToJson() const {
	nlohmann::json result;
	result["screen_image"] = (ScreenImage != nullptr) ? ScreenImage->GetGUID().str() : "null";
	result["win_screen"] = (WinScreen != nullptr) ? WinScreen->GetGUID().str() : "null";
	result["objectives_achieved"] = objectivesAchieved;
	result["target_objectives"] = targetObjectives;
	result["screen_active"] = active;
	result["screen_is_moving"] = isMoving;
	return result;
}

SimpleScreenBehaviour::Sptr SimpleScreenBehaviour::FromJson(const nlohmann::json& blob) {
	SimpleScreenBehaviour::Sptr result = std::make_shared<SimpleScreenBehaviour>();
	result->ScreenImage = ResourceManager::Get<Gameplay::Material>(Guid(blob["screen_image"].get<std::string>()));
	result->WinScreen = ResourceManager::Get<Gameplay::Material>(Guid(blob["win_screen"].get<std::string>()));
	result->objectivesAchieved = blob["objectives_achieved"];
	result->targetObjectives = blob["target_objectives"];
	result->active = blob["screen_active"];
	result->isMoving = blob["screen_is_moving"];
	return result;
}
