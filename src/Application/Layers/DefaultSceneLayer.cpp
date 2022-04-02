#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtc/random.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Gameplay/Components/Light.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"
#include "Application/Layers/ImGuiDebugLayer.h"
#include "Application/Windows/DebugWindow.h"
#include "Gameplay/Components/ShadowCamera.h"

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad;
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
}

void DefaultSceneLayer::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	} else {
		 
		// Basic gbuffer generation with no vertex manipulation
		ShaderProgram::Sptr deferredForward = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		deferredForward->SetDebugName("Deferred - GBuffer Generation");  

		// Our foliage shader which manipulates the vertices of the mesh
		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});  
		foliageShader->SetDebugName("Foliage");   

		// This shader handles our multitexturing example
		ShaderProgram::Sptr multiTextureShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },  
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});
		multiTextureShader->SetDebugName("Multitexturing"); 

		// This shader handles our displacement mapping example
		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		// This shader handles our cel shading example
		ShaderProgram::Sptr celShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/cel_shader.glsl" }
		});
		celShader->SetDebugName("Cel Shader");


		// Load in the meshes
		//MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		//MeshResource::Sptr shipMesh   = ResourceManager::CreateAsset<MeshResource>("fenrir.obj");
		
		MeshResource::Sptr BathroomMesh = ResourceManager::CreateAsset<MeshResource>("gameModels/megaBathroom.obj");
		MeshResource::Sptr HandMesh     = ResourceManager::CreateAsset<MeshResource>("gameModels/handIdleMesh-3.obj");
		MeshResource::Sptr ToiletMesh   = ResourceManager::CreateAsset<MeshResource>("gameModels/toilet.obj");
		MeshResource::Sptr SoapMesh     = ResourceManager::CreateAsset<MeshResource>("gameModels/soap.obj");
		MeshResource::Sptr SpilledMesh  = ResourceManager::CreateAsset<MeshResource>("gameModels/soapSpilled.obj");
		MeshResource::Sptr DuckMesh     = ResourceManager::CreateAsset<MeshResource>("gameModels/ducky.obj");
		MeshResource::Sptr FlatDuckMesh = ResourceManager::CreateAsset<MeshResource>("gameModels/flatDucky.obj");

		// Load in some textures
		Texture2D::Sptr HandTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/Hand.png");
		Texture2D::Sptr HandCleanTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/HandClean.png");
		Texture2D::Sptr HandDirtyTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/HandPoo.png");
		Texture2D::Sptr HandDuckTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/HandDucky.png");
		Texture2D::Sptr BathroomTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/BathroomTexture.png");
		Texture2D::Sptr DuckTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/Ducky.png");
		Texture2D::Sptr ToiletTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/Toilet.png");
		Texture2D::Sptr DirtyToiletTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/ShitToilet.png");
		Texture2D::Sptr SoapTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/soap.png");
		Texture2D::Sptr SpilledSoapTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/SpilledSoap.png");
		Texture2D::Sptr LineTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/Line.png");

		Texture2D::Sptr    boxTexture   = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec      = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");
		Texture2D::Sptr    monkeyTex    = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    leafTex      = ResourceManager::CreateAsset<Texture2D>("textures/leaves.png");
		leafTex->SetMinFilter(MinFilter::Nearest);
		leafTex->SetMagFilter(MagFilter::Nearest);

		// Load some images for drag n' drop
		ResourceManager::CreateAsset<Texture2D>("textures/flashlight.png");
		ResourceManager::CreateAsset<Texture2D>("textures/flashlight-2.png");
		ResourceManager::CreateAsset<Texture2D>("textures/light_projection.png");

		DebugWindow::Sptr debugWindow = app.GetLayer<ImGuiDebugLayer>()->GetWindow<DebugWindow>();

#pragma region Basic Texture Creation
		Texture2DDescription singlePixelDescriptor;
		singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		singlePixelDescriptor.Format = InternalFormat::RGB8;

		float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

		float solidGrey[3] = { 0.5f, 0.5f, 0.5f };
		float solidBlack[3] = { 0.0f, 0.0f, 0.0f };
		float solidWhite[3] = { 1.0f, 1.0f, 1.0f };

		Texture2D::Sptr solidBlackTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidBlackTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidBlack);

		Texture2D::Sptr solidGreyTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidGreyTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidGrey);

		Texture2D::Sptr solidWhiteTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidWhiteTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidWhite);

#pragma endregion 

		// Loading in a 1D LUT
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png"); 
		toonLut->SetWrap(WrapMode::ClampToEdge);

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/NightSky/NightSky.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" } 
		});
		  
		// Create an empty scene
		Scene::Sptr scene = std::make_shared<Scene>();  

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap); 
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Loading in a color lookup table
		Texture3D::Sptr lut = ResourceManager::CreateAsset<Texture3D>("luts/cool.CUBE");   
		 
		// Configure the color correction LUT
		scene->SetColorLUT(lut);

		// Create our materials
		// This will be our box material, with no environment reflections
		Material::Sptr boxMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			boxMaterial->Name = "Box";
			boxMaterial->Set("u_Material.AlbedoMap", boxTexture);
			boxMaterial->Set("u_Material.Shininess", 0.1f);
			boxMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		////////////////////////////////////////////////////////////////////
		Material::Sptr BathroomMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			BathroomMaterial->Name = "BathroomMaterial";
			BathroomMaterial->Set("u_Material.AlbedoMap", BathroomTex);
			BathroomMaterial->Set("u_Material.Shininess", 0.1f);
			BathroomMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr HandMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			HandMaterial->Name = "HandMaterial";
			HandMaterial->Set("u_Material.AlbedoMap", HandTex);
			HandMaterial->Set("u_Material.Shininess", 0.1f);
			HandMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr HandCleanMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			boxMaterial->Name = "Box";
			boxMaterial->Set("u_Material.AlbedoMap", HandCleanTex);
			boxMaterial->Set("u_Material.Shininess", 0.1f);
			boxMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr HandDirtyMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			HandDirtyMaterial->Name = "HandDirtyMaterial";
			HandDirtyMaterial->Set("u_Material.AlbedoMap", HandDirtyTex);
			HandDirtyMaterial->Set("u_Material.Shininess", 0.1f);
			HandDirtyMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr HandDuckMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			HandDuckMaterial->Name = "HandDuckMaterial";
			HandDuckMaterial->Set("u_Material.AlbedoMap", HandDuckTex);
			HandDuckMaterial->Set("u_Material.Shininess", 0.1f);
			HandDuckMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr DuckMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			DuckMaterial->Name = "DuckMaterial";
			DuckMaterial->Set("u_Material.AlbedoMap", DuckTex);
			DuckMaterial->Set("u_Material.Shininess", 0.1f);
			DuckMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr ToiletMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			ToiletMaterial->Name = "ToiletMaterial";
			ToiletMaterial->Set("u_Material.AlbedoMap", ToiletTex);
			ToiletMaterial->Set("u_Material.Shininess", 0.1f);
			ToiletMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr DirtyToiletMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			DirtyToiletMaterial->Name = "DirtyToiletMaterial";
			DirtyToiletMaterial->Set("u_Material.AlbedoMap", DirtyToiletTex);
			DirtyToiletMaterial->Set("u_Material.Shininess", 0.1f);
			DirtyToiletMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr SoapMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SoapMaterial->Name = "SoapMaterial";
			SoapMaterial->Set("u_Material.AlbedoMap", boxTexture);
			SoapMaterial->Set("u_Material.Shininess", 0.1f);
			SoapMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr SpilledSoapMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SpilledSoapMaterial->Name = "SpilledSoapMaterial";
			SpilledSoapMaterial->Set("u_Material.AlbedoMap", SpilledSoapTex);
			SpilledSoapMaterial->Set("u_Material.Shininess", 0.1f);
			SpilledSoapMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr LineMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			LineMaterial->Name = "LineMaterial";
			LineMaterial->Set("u_Material.AlbedoMap", LineTex);
			LineMaterial->Set("u_Material.Shininess", 0.1f);
			LineMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}



		////////////////////////////////////////////////////////////////////

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr monkeyMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			monkeyMaterial->Name = "Monkey";
			monkeyMaterial->Set("u_Material.AlbedoMap", monkeyTex);
			monkeyMaterial->Set("u_Material.NormalMap", normalMapDefault);
			monkeyMaterial->Set("u_Material.Shininess", 0.5f);
		}

		// This will be the reflective material, we'll make the whole thing 50% reflective
		Material::Sptr testMaterial = ResourceManager::CreateAsset<Material>(deferredForward); 
		{
			testMaterial->Name = "Box-Specular";
			testMaterial->Set("u_Material.AlbedoMap", boxTexture); 
			testMaterial->Set("u_Material.Specular", boxSpec);
			testMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		// Our foliage vertex shader material 
		Material::Sptr foliageMaterial = ResourceManager::CreateAsset<Material>(foliageShader);
		{
			foliageMaterial->Name = "Foliage Shader";
			foliageMaterial->Set("u_Material.AlbedoMap", leafTex);
			foliageMaterial->Set("u_Material.Shininess", 0.1f);
			foliageMaterial->Set("u_Material.DiscardThreshold", 0.1f);
			foliageMaterial->Set("u_Material.NormalMap", normalMapDefault);

			foliageMaterial->Set("u_WindDirection", glm::vec3(1.0f, 1.0f, 0.0f));
			foliageMaterial->Set("u_WindStrength", 0.5f);
			foliageMaterial->Set("u_VerticalScale", 1.0f);
			foliageMaterial->Set("u_WindSpeed", 1.0f);
		}

		// Our toon shader material
		Material::Sptr toonMaterial = ResourceManager::CreateAsset<Material>(celShader);
		{
			toonMaterial->Name = "Toon"; 
			toonMaterial->Set("u_Material.AlbedoMap", boxTexture);
			toonMaterial->Set("u_Material.NormalMap", normalMapDefault);
			toonMaterial->Set("s_ToonTerm", toonLut);
			toonMaterial->Set("u_Material.Shininess", 0.1f); 
			toonMaterial->Set("u_Material.Steps", 8);
		}


		Material::Sptr displacementTest = ResourceManager::CreateAsset<Material>(displacementShader);
		{
			Texture2D::Sptr displacementMap = ResourceManager::CreateAsset<Texture2D>("textures/displacement_map.png");
			Texture2D::Sptr normalMap       = ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png");
			Texture2D::Sptr diffuseMap      = ResourceManager::CreateAsset<Texture2D>("textures/bricks_diffuse.png");

			displacementTest->Name = "Displacement Map";
			displacementTest->Set("u_Material.AlbedoMap", diffuseMap);
			displacementTest->Set("u_Material.NormalMap", normalMap);
			displacementTest->Set("s_Heightmap", displacementMap);
			displacementTest->Set("u_Material.Shininess", 0.5f);
			displacementTest->Set("u_Scale", 0.1f);
		}

		Material::Sptr grey = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			grey->Name = "Grey";
			grey->Set("u_Material.AlbedoMap", solidGreyTex);
			grey->Set("u_Material.Specular", solidBlackTex);
			grey->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr whiteBrick = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			whiteBrick->Name = "White Bricks";
			whiteBrick->Set("u_Material.AlbedoMap", ResourceManager::CreateAsset<Texture2D>("textures/displacement_map.png"));
			whiteBrick->Set("u_Material.Specular", solidGrey);
			whiteBrick->Set("u_Material.NormalMap", ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png"));
		}

		Material::Sptr normalmapMat = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Texture2D::Sptr normalMap       = ResourceManager::CreateAsset<Texture2D>("textures/normal_map.png");
			Texture2D::Sptr diffuseMap      = ResourceManager::CreateAsset<Texture2D>("textures/bricks_diffuse.png");

			normalmapMat->Name = "Tangent Space Normal Map";
			normalmapMat->Set("u_Material.AlbedoMap", diffuseMap);
			normalmapMat->Set("u_Material.NormalMap", normalMap);
			normalmapMat->Set("u_Material.Shininess", 0.5f);
			normalmapMat->Set("u_Scale", 0.1f);
		}

		Material::Sptr multiTextureMat = ResourceManager::CreateAsset<Material>(multiTextureShader);
		{
			Texture2D::Sptr sand  = ResourceManager::CreateAsset<Texture2D>("textures/terrain/sand.png");
			Texture2D::Sptr grass = ResourceManager::CreateAsset<Texture2D>("textures/terrain/grass.png");

			multiTextureMat->Name = "Multitexturing";
			multiTextureMat->Set("u_Material.DiffuseA", sand);
			multiTextureMat->Set("u_Material.DiffuseB", grass);
			multiTextureMat->Set("u_Material.NormalMapA", normalMapDefault);
			multiTextureMat->Set("u_Material.NormalMapB", normalMapDefault);
			multiTextureMat->Set("u_Material.Shininess", 0.5f);
			multiTextureMat->Set("u_Scale", 0.1f); 
		}

		// Create some lights for our scene
		GameObject::Sptr lightParent = scene->CreateGameObject("Lights");

		for (int ix = 0; ix < 50; ix++) {
			GameObject::Sptr light = scene->CreateGameObject("Light");
			light->SetPosition(glm::vec3(glm::diskRand(25.0f), 1.0f));
			lightParent->AddChild(light);

			Light::Sptr lightComponent = light->Add<Light>();
			lightComponent->SetColor(glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
			lightComponent->SetRadius(glm::linearRand(0.1f, 10.0f));
			lightComponent->SetIntensity(glm::linearRand(1.0f, 2.0f));
		}

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		MeshResource::Sptr sphere = ResourceManager::CreateAsset<MeshResource>();
		sphere->AddParam(MeshBuilderParam::CreateIcoSphere(ZERO, ONE, 5));
		sphere->GenerateMesh();

		// Set up the scene's camera
		GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPosition({ -2, -4.5, -8 });
			camera->SetRotation({ -180, 0, 0 });
			camera->SetScale({ 1, 1, 1 });
			//camera->LookAt(glm::vec3(0.0f));

			camera->Add<SimpleCameraControl>();

		}

		/////////////////////////////////////////////////////////////////////

		GameObject::Sptr BathroomModel = scene->CreateGameObject("BathroomModel");
		{
			// Set position in the scene
			BathroomModel->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

			// Add some behaviour that relies on the physics body
			BathroomModel->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = BathroomModel->Add<RenderComponent>();
			renderer->SetMesh(BathroomMesh);
			renderer->SetMaterial(BathroomMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			TriggerVolume::Sptr trigger = BathroomModel->Add<TriggerVolume>();
			trigger->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);
			trigger->AddCollider(BoxCollider::Create(glm::vec3(1.0f)));

			BathroomModel->Add<TriggerVolumeEnterBehaviour>();
		}

		GameObject::Sptr DuckModel = scene->CreateGameObject("DuckModel");
		{
			// Set position in the scene
			DuckModel->SetPosition(glm::vec3(4.0f, -2.0f, 0.0f));

			// Add some behaviour that relies on the physics body
			DuckModel->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = DuckModel->Add<RenderComponent>();
			renderer->SetMesh(DuckMesh);
			renderer->SetMaterial(DuckMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			TriggerVolume::Sptr trigger = DuckModel->Add<TriggerVolume>();
			trigger->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);
			trigger->AddCollider(BoxCollider::Create(glm::vec3(1.0f)));

			DuckModel->Add<TriggerVolumeEnterBehaviour>();
		}

		GameObject::Sptr FlatDuckModel = scene->CreateGameObject("FlatDuckModel");
		{
			// Set position in the scene
			FlatDuckModel->SetPosition(glm::vec3(-0.5f, 0.0f, 0.0f));

			// Add some behaviour that relies on the physics body
			FlatDuckModel->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = FlatDuckModel->Add<RenderComponent>();
			renderer->SetMesh(FlatDuckMesh);
			renderer->SetMaterial(DuckMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			TriggerVolume::Sptr trigger = FlatDuckModel->Add<TriggerVolume>();
			trigger->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);
			trigger->AddCollider(BoxCollider::Create(glm::vec3(1.0f)));

			FlatDuckModel->Add<TriggerVolumeEnterBehaviour>();
		}

		GameObject::Sptr ToiletModel = scene->CreateGameObject("ToiletModel");
		{
			// Set position in the scene
			ToiletModel->SetPosition(glm::vec3(-0.5f, -1.0f, 2.5f));

			// Add some behaviour that relies on the physics body
			ToiletModel->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = ToiletModel->Add<RenderComponent>();
			renderer->SetMesh(ToiletMesh);
			renderer->SetMaterial(ToiletMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			TriggerVolume::Sptr trigger = ToiletModel->Add<TriggerVolume>();
			trigger->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);
			trigger->AddCollider(BoxCollider::Create(glm::vec3(1.0f)));

			ToiletModel->Add<TriggerVolumeEnterBehaviour>();
		}

		GameObject::Sptr SoapModel = scene->CreateGameObject("SoapModel");
		{
			// Set position in the scene
			SoapModel->SetPosition(glm::vec3(-3.2f, -3.50f, 4.72f));

			// Add some behaviour that relies on the physics body
			SoapModel->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = SoapModel->Add<RenderComponent>();
			renderer->SetMesh(SoapMesh);
			renderer->SetMaterial(SoapMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			TriggerVolume::Sptr trigger = SoapModel->Add<TriggerVolume>();
			trigger->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);
			trigger->AddCollider(BoxCollider::Create(glm::vec3(1.0f)));

			SoapModel->Add<TriggerVolumeEnterBehaviour>();
		}

		GameObject::Sptr SpilledSoapModel = scene->CreateGameObject("SpilledSoapModel");
		{
			// Set position in the scene
			SpilledSoapModel->SetPosition(glm::vec3(-4.5f, -0.2f, 0.3f));

			// Add some behaviour that relies on the physics body
			SpilledSoapModel->Add<JumpBehaviour>();

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = SpilledSoapModel->Add<RenderComponent>();
			renderer->SetMesh(SpilledMesh);
			renderer->SetMaterial(SpilledSoapMaterial);

			// Example of a trigger that interacts with static and kinematic bodies as well as dynamic bodies
			TriggerVolume::Sptr trigger = SpilledSoapModel->Add<TriggerVolume>();
			trigger->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);
			trigger->AddCollider(BoxCollider::Create(glm::vec3(1.0f)));

			SpilledSoapModel->Add<TriggerVolumeEnterBehaviour>();
		}


		///////////////////////////////////////////////////////////////////////

		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);
	}
}
