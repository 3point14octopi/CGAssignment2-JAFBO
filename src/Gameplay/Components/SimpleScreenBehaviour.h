#pragma once
#include "IComponent.h"

#include <GLFW/glfw3.h>
#include "Gameplay/Scene.h"
#include "Gameplay/Material.h"

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class SimpleScreenBehaviour : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<SimpleScreenBehaviour> Sptr;

	SimpleScreenBehaviour() = default;
	Gameplay::Material::Sptr        ScreenImage;
	Gameplay::Material::Sptr        WinScreen;
	
	int objectivesAchieved = 0;
	int targetObjectives = 0;

	bool active = false;
	bool isMoving = false;

	virtual void Update(float deltaTime) override;
	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static SimpleScreenBehaviour::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(SimpleScreenBehaviour);
};

