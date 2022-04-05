#pragma once
#include "IComponent.h"
#include <GLFW/glfw3.h>
#include "Gameplay/Scene.h"
#include <ctime>
#include <cstdlib>
/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class WarpBehaviour : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<WarpBehaviour> Sptr;

	WarpBehaviour() = default;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static WarpBehaviour::Sptr FromJson(const nlohmann::json& data);

	void CrossOffItem(int);

	void PushLines(std::vector<Gameplay::GameObject::Sptr>);

	MAKE_TYPENAME(WarpBehaviour);
	 
	virtual void Awake() override;

protected:

	std::vector<bool> listChecks;
	std::vector<Gameplay::GameObject::Sptr>lines;

	void InitList();
};