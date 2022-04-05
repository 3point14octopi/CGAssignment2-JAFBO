#include "Gameplay/Components/WarpBehaviour.h"
#include "Gameplay/GameObject.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/InterpolationBehaviour.h"
#include "Gameplay/InputEngine.h"


void WarpBehaviour::RenderImGui() {

}

nlohmann::json WarpBehaviour::ToJson() const {
	return {

	};
}

WarpBehaviour::Sptr WarpBehaviour::FromJson(const nlohmann::json& data) {
	WarpBehaviour::Sptr result = std::make_shared<WarpBehaviour>();

	return result;
}



void WarpBehaviour::InitList() {
	for (int i = 0; i < 3; i++) {

		listChecks.push_back(0);
	}
}


void WarpBehaviour::CrossOffItem(int index) {
	listChecks[index] = true;
	glm::vec3 pos = glm::vec3(lines[index]->GetPosition());
	std::cout << "retrieving " << index << ": " << lines[index]->Name;
	pos.z -= 100;
	lines[index]->SetPosition(pos);
}



void WarpBehaviour::PushLines(std::vector<Gameplay::GameObject::Sptr> lineVec) {
	for (int i = 0; i < 3; i++) {
		lines.push_back(lineVec[i]);
	}
}

void WarpBehaviour::Awake() {
	InitList();
}