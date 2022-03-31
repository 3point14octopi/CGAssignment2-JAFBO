#include "Gameplay/Components/InteractableObjectBehaviour.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/SimpleScreenBehaviour.h"
#include "Gameplay/Components/WarpBehaviour.h"
#include "Gameplay/InputEngine.h"
InteractionFeedback::InteractionFeedback(Gameplay::Material::Sptr mat, Gameplay::GameObject::Sptr o) {
	_SWAPMAT = mat;
	_TARGET = o;
	b = TEX;
}

InteractionFeedback::InteractionFeedback(Gameplay::MeshResource::Sptr mesh, Gameplay::GameObject::Sptr o) {
	_SWAPMESH = mesh;
	_TARGET = o;
	b = MESH;
}

InteractionFeedback::InteractionFeedback(std::vector<InteractionTForm> tforms, Gameplay::GameObject::Sptr o) {
	for (int i = 0; i < tforms.size(); i++) {
		_SWAPTRANSFORM.push_back(tforms[i]);
	}

	_TARGET = o;
	b = TRANSFORM;
}

InteractionFeedback::InteractionFeedback(int listIndex) {
	_SWAPAINDEX = listIndex;
	_TARGET = nullptr;
	b = CROSSOUT;
}

void InteractionFeedback::SwapMat() {
	_TARGET->Get<RenderComponent>()->SetMaterial(_SWAPMAT);
	std::cout << "swapping material";
}

void InteractionFeedback::SwapMesh() {
	_TARGET->Get<RenderComponent>()->SetMesh(_SWAPMESH);
}

void InteractionFeedback::SwapTransforms() {

	for (int i = 0; i < _SWAPTRANSFORM.size(); i++) {
		switch (_SWAPTRANSFORM[i].trnsfrm) {
		case(InteractionTForm::pos):
			_TARGET->SetPosition(_SWAPTRANSFORM[i].tform);
			break;
		case(InteractionTForm::rot):
			_TARGET->SetRotation(_SWAPTRANSFORM[i].tform);
			break;
		default:
			_TARGET->SetScale(_SWAPTRANSFORM[i].tform);
		}
	}
}


InteractableObjectBehaviour::InteractableObjectBehaviour() :
	IComponent(), _windowPointer(nullptr)
{ 
	_hasBeenActivated = false;
}
InteractableObjectBehaviour::~InteractableObjectBehaviour() = default;

/// <summary>
/// Add a material that the player can use on the hand as a "reward" for interacting
/// </summary>
/// <param name="r">
/// : the material being used as a reward
/// </param>
void InteractableObjectBehaviour::AddRewardMaterial(Gameplay::Material::Sptr r) {
	_rewardMaterial = r;
}


void InteractableObjectBehaviour::AddFeedbackBehaviour(InteractionFeedback f) {
	feedback.push_back(f);
}


void InteractableObjectBehaviour::Update(float deltaTime) {
	if (_playerInTrigger) {

		if (InputEngine::GetKeyState(GLFW_KEY_E) == ButtonState::Pressed) {
			_hasBeenActivated = true;
			_playerInTrigger = false;
			PerformFeedback();
			_body->GetGameObject()->Get<SkinManager>()->AddSkin(_rewardMaterial);
			_body = nullptr;
			
			screen->Get<SimpleScreenBehaviour>()->objectivesAchieved += 1;
			screen->Get<SimpleScreenBehaviour>()->active = true;
		}

	}

}


void InteractableObjectBehaviour::OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body)
{
	if (!_hasBeenActivated) {
		LOG_INFO("Body has entered our trigger volume: {}", body->GetGameObject()->Name);
		_playerInTrigger = true;
		_body = body;
		
	}
	std::cout << "enter trigger";
	if (_hasBeenActivated == true) {
		std::cout << std::endl << "trigger has been activated";
	}
}

void InteractableObjectBehaviour::OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) {
	
	if (!_hasBeenActivated) {
		LOG_INFO("Body has left our trigger volume: {}", body->GetGameObject()->Name);
		_playerInTrigger = false;
		_body = nullptr;
		
	}
	std::cout << "exit trigger";
	if (_hasBeenActivated == true) {
		std::cout << std::endl << "trigger has been exited";
	}
		
}


void InteractableObjectBehaviour::PerformFeedback() {
	for (int i = 0; i < feedback.size(); i++) {
		std::cout << feedback[i].b;
		switch (feedback[i].b) {
		case(TEX):
			feedback[i].SwapMat();
			break;
		case(MESH):
			feedback[i].SwapMesh();
			break;
		case(TRANSFORM):
			feedback[i].SwapTransforms();
			break;
		default:
			(GetGameObject()->GetScene()->FindObjectByName("Floor Manager"))->Get<WarpBehaviour>()->CrossOffItem(feedback[i]._SWAPAINDEX);
		}
	}
}



void InteractableObjectBehaviour::RenderImGui() { }

nlohmann::json InteractableObjectBehaviour::ToJson() const {
	nlohmann::json result;
	return result;
}

InteractableObjectBehaviour::Sptr InteractableObjectBehaviour::FromJson(const nlohmann::json& blob) {
	InteractableObjectBehaviour::Sptr result = std::make_shared<InteractableObjectBehaviour>();
	return result;
}

