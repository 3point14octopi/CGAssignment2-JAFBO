#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "GLFW/glfw3.h"
#include "Gameplay/Material.h"

/// <summary>
/// A simple system that stores materials and can apply them to the hand
/// </summary>
class SkinManager : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<SkinManager> Sptr;

	SkinManager();
	virtual ~SkinManager();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	void AddSkin(Gameplay::Material::Sptr);
	void ApplySkin();

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(SkinManager);
	virtual nlohmann::json ToJson() const override;
	static SkinManager::Sptr FromJson(const nlohmann::json& blob);

protected:
	int cooldown = 0;
	int _currentIndex = 0;
	int _numberOfSkins = 0;

	std::vector<Gameplay::Material::Sptr> _skins;
	Gameplay::Material::Sptr _defaultSkin;
	Gameplay::Material::Sptr _currentSkin;
	
};