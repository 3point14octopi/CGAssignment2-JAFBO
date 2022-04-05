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

//components
#include "Gameplay/Components/InteractableObjectBehaviour.h"
#include "Gameplay/Components/InterpolationBehaviour.h"
#include "Gameplay/Components/WarpBehaviour.h"
#include "Gameplay/Components/SkinManager.h"
#include "Gameplay/Components/SimpleScreenBehaviour.h"
#include "Gameplay/Components/CharacterController.h"


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
		Texture2D::Sptr SoapTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/soap.png");

		Texture2D::Sptr DirtyToiletTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/ShitToilet.png");
		Texture2D::Sptr SpilledSoapTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/SpilledSoap.png");

		Texture2D::Sptr LineTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/red.jpg");
		Texture2D::Sptr ListTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/ListBathroom.png");
		Texture2D::Sptr DuckScreenTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/screens/Ducky.png");
		Texture2D::Sptr ToiletScreenTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/screens/Toilet.png");
		Texture2D::Sptr SoapScreenTex = ResourceManager::CreateAsset<Texture2D>("gameTextures/screens/Soap.png");


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
		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon1-1D.png");
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
		Texture3D::Sptr coolLut = ResourceManager::CreateAsset<Texture3D>("luts/Icy.CUBE"); 
		Texture3D::Sptr warmLut = ResourceManager::CreateAsset<Texture3D>("luts/Toasty.CUBE");
		Texture3D::Sptr customLut = ResourceManager::CreateAsset<Texture3D>("luts/Sweet.CUBE");
		 

		app.GetLayer<ImGuiDebugLayer>()->GetWindow<DebugWindow>()->SetCoolCC(coolLut);
		app.GetLayer<ImGuiDebugLayer>()->GetWindow<DebugWindow>()->SetWarmCC(warmLut);
		app.GetLayer<ImGuiDebugLayer>()->GetWindow<DebugWindow>()->SetCustomCC(customLut);

		// Configure the color correction LUT
		//HOW TO SET LUT
		scene->SetColorLUT(coolLut);

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
			HandCleanMaterial->Name = "Box";
			HandCleanMaterial->Set("u_Material.AlbedoMap", HandCleanTex);
			HandCleanMaterial->Set("u_Material.Shininess", 0.1f);
			HandCleanMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr HandDirtyMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			HandDirtyMaterial->Name = "HandDirtyMaterial";
			HandDirtyMaterial->Set("u_Material.AlbedoMap", HandDirtyTex);
			HandDirtyMaterial->Set("u_Material.Shininess", 0.1f);
			HandDirtyMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr HandDuckMaterial = ResourceManager::CreateAsset<Material>(celShader);
		{
			HandDuckMaterial->Name = "HandDuckMaterial";
			HandDuckMaterial->Set("u_Material.AlbedoMap", HandDuckTex);
			HandDuckMaterial->Set("s_ToonTerm", toonLut);
			HandDuckMaterial->Set("u_Material.Shininess", 0.1f);
			HandDuckMaterial->Set("u_Material.NormalMap", normalMapDefault);
			HandDuckMaterial->Set("u_Material.Steps", 4);
		}

		Material::Sptr DuckMaterial = ResourceManager::CreateAsset<Material>(celShader);
		{
			DuckMaterial->Name = "DuckMaterial";
			DuckMaterial->Set("u_Material.AlbedoMap", DuckTex);
			DuckMaterial->Set("u_Material.NormalMap", normalMapDefault);
			DuckMaterial->Set("s_ToonTerm", toonLut);
			DuckMaterial->Set("u_Material.Shininess", 0.1f);
			DuckMaterial->Set("u_Material.Steps", 4);
		}

		Material::Sptr ToiletMaterial = ResourceManager::CreateAsset<Material>(celShader);
		{
			ToiletMaterial->Name = "ToiletMaterial";
			ToiletMaterial->Set("u_Material.AlbedoMap", ToiletTex);
			ToiletMaterial->Set("u_Material.NormalMap", normalMapDefault);
			ToiletMaterial->Set("s_ToonTerm", toonLut);
			ToiletMaterial->Set("u_Material.Shininess", 0.1f);
			ToiletMaterial->Set("u_Material.Steps", 4);
		}

		Material::Sptr DirtyToiletMaterial = ResourceManager::CreateAsset<Material>(celShader);
		{
			DirtyToiletMaterial->Name = "DirtyToiletMaterial";
			DirtyToiletMaterial->Set("u_Material.AlbedoMap", DirtyToiletTex);
			DirtyToiletMaterial->Set("u_Material.NormalMap", normalMapDefault);
			DirtyToiletMaterial->Set("s_ToonTerm", toonLut);
			DirtyToiletMaterial->Set("u_Material.Shininess", 0.1f);
			DirtyToiletMaterial->Set("u_Material.Steps", 4);
		}

		Material::Sptr SoapMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SoapMaterial->Name = "SoapMaterial";
			SoapMaterial->Set("u_Material.AlbedoMap", SoapTex);
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

		Material::Sptr ListMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			ListMaterial->Name = "LineMaterial";
			ListMaterial->Set("u_Material.AlbedoMap", ListTex);
			ListMaterial->Set("u_Material.Shininess", 0.1f);
			ListMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr DuckScreenMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			DuckScreenMaterial->Name = "LineMaterial";
			DuckScreenMaterial->Set("u_Material.AlbedoMap", DuckScreenTex);
			DuckScreenMaterial->Set("u_Material.Shininess", 0.1f);
			DuckScreenMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr ToiletScreenMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			ToiletScreenMaterial->Name = "LineMaterial";
			ToiletScreenMaterial->Set("u_Material.AlbedoMap", ToiletScreenTex);
			ToiletScreenMaterial->Set("u_Material.Shininess", 0.1f);
			ToiletScreenMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Material::Sptr SoapScreenMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SoapScreenMaterial->Name = "LineMaterial";
			SoapScreenMaterial->Set("u_Material.AlbedoMap", SoapScreenTex);
			SoapScreenMaterial->Set("u_Material.Shininess", 0.1f);
			SoapScreenMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}



		// Create some lights for our scene
		GameObject::Sptr lightParent = scene->CreateGameObject("Lights");

		GameObject::Sptr light = scene->CreateGameObject("Light");
		light->SetPosition(glm::vec3(1.25f, 2.73f, 5.9f));
		lightParent->AddChild(light);

		Light::Sptr lightComponent = light->Add<Light>();
		lightComponent->SetColor(glm::vec3(1.0f));
		lightComponent->SetRadius(10.0f);
		lightComponent->SetIntensity(4.5f);

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
			camera->SetPosition(glm::vec3(9.15f, 9.41f, 7.85f));
			camera->SetRotation(glm::vec3(80.351f, 0.0f, 142.0f));
			camera->SetScale(glm::vec3(0.69f, 0.769f, 0.83f));
		}

		/////////////////////////////////////////////////////////////////////
		
		GameObject::Sptr floorManager = scene->CreateGameObject("Floor Manager");
		{
			WarpBehaviour::Sptr warp = floorManager->Add<WarpBehaviour>();

			TriggerVolume::Sptr volume = floorManager->Add<TriggerVolume>();
			SphereCollider::Sptr collider = SphereCollider::Create(1.8);
			collider->SetPosition(glm::vec3(3.5f, 3.5f, 0.f));
			volume->AddCollider(collider);
		}

		GameObject::Sptr extraScreen = scene->CreateGameObject("Interact Screen");
		{
			MeshResource::Sptr screenMesh = ResourceManager::CreateAsset<MeshResource>();
			screenMesh->AddParam(MeshBuilderParam::CreatePlane(glm::vec3(0.0f, 0.0f, 0.0f), UNIT_Z, UNIT_X, glm::vec2(18.0f, 10.0f), glm::vec2(1.0f)));
			screenMesh->GenerateMesh();
			extraScreen->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
			extraScreen->SetRotation(glm::vec3(80.351f, 0.0f, 142.00f));

			RenderComponent::Sptr renderer = extraScreen->Add<RenderComponent>();
			renderer->SetMesh(screenMesh);
			renderer->SetMaterial(ListMaterial);

			InterpolationBehaviour::Sptr interp = extraScreen->Add<InterpolationBehaviour>();
			interp->AddBehaviourScript("interp_scripts/menu_behaviour.txt");
			interp->ToggleBehaviour("Lowering", false);
			interp->PauseOrResumeCurrentBehaviour();

		}

		GameObject::Sptr list = scene->CreateGameObject("List");
		{
			MeshResource::Sptr listMesh = ResourceManager::CreateAsset<MeshResource>();
			listMesh->AddParam(MeshBuilderParam::CreatePlane(glm::vec3(0.0f, 0.0f, 0.0f), UNIT_Z, UNIT_X, glm::vec2(4.0f, 10.0f), glm::vec2(1.0f)));
			listMesh->GenerateMesh();
			list->SetPosition(glm::vec3(10.75f, 1.96f, 6.56f));
			list->SetRotation(glm::vec3(80.351f, 0.0f, 142.00f));

			RenderComponent::Sptr renderer = list->Add<RenderComponent>();
			renderer->SetMesh(listMesh);
			renderer->SetMaterial(ListMaterial);

			SimpleScreenBehaviour::Sptr feedbackScreen = extraScreen->Add<SimpleScreenBehaviour>();
			feedbackScreen->targetObjectives = 5;


		}

		GameObject::Sptr lineOne = scene->CreateGameObject("Line One");
		{
			MeshResource::Sptr lineMesh = ResourceManager::CreateAsset<MeshResource>();
			lineMesh->AddParam(MeshBuilderParam::CreatePlane(glm::vec3(0.0f, 0.0f, 0.0f), UNIT_Z, UNIT_X, glm::vec2(3.0f, 1.0f), glm::vec2(1.0f)));
			lineMesh->GenerateMesh();

			RenderComponent::Sptr renderer = lineOne->Add<RenderComponent>();
			renderer->SetMesh(lineMesh);
			renderer->SetMaterial(LineMaterial);

			lineOne->SetPosition(glm::vec3(10.45f, 1.72f, 108.88f));
			lineOne->SetRotation(glm::vec3(80.351f, 0.0f, 142.00f));
		}

		GameObject::Sptr lineTwo = scene->CreateGameObject("Line Two");
		{
			MeshResource::Sptr lineMesh = ResourceManager::CreateAsset<MeshResource>();
			lineMesh->AddParam(MeshBuilderParam::CreatePlane(glm::vec3(0.0f, 0.0f, 0.0f), UNIT_Z, UNIT_X, glm::vec2(3.0f, 1.0f), glm::vec2(1.0f)));
			lineMesh->GenerateMesh();

			RenderComponent::Sptr renderer = lineTwo->Add<RenderComponent>();
			renderer->SetMesh(lineMesh);
			renderer->SetMaterial(LineMaterial);

			lineTwo->SetPosition(glm::vec3(10.6f, 1.93f, 107.33f));
			lineTwo->SetRotation(glm::vec3(80.351f, 0.0f, 142.00f));
		}

		GameObject::Sptr lineThree = scene->CreateGameObject("Line Three");
		{
			MeshResource::Sptr lineMesh = ResourceManager::CreateAsset<MeshResource>();
			lineMesh->AddParam(MeshBuilderParam::CreatePlane(glm::vec3(0.0f, 0.0f, 0.0f), UNIT_Z, UNIT_X, glm::vec2(3.0f, 1.0f), glm::vec2(1.0f)));
			lineMesh->GenerateMesh();

			RenderComponent::Sptr renderer = lineThree->Add<RenderComponent>();
			renderer->SetMesh(lineMesh);
			renderer->SetMaterial(LineMaterial);

			lineThree->SetPosition(glm::vec3(10.79f, 2.18f, 105.6f));
			lineThree->SetRotation(glm::vec3(80.351f, 0.0f, 142.00f));
		}

		std::vector<GameObject::Sptr>lines{ lineOne, lineTwo, lineThree};
		floorManager->Get<WarpBehaviour>()->PushLines(lines);

		GameObject::Sptr BathroomModel = scene->CreateGameObject("BathroomModel");
		{
			// Set position in the scene
			BathroomModel->SetPosition(glm::vec3(2.0f, 2.0f, 2.0f));
			BathroomModel->SetRotation(glm::vec3(0.0f, 0.0f, -30.0f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = BathroomModel->Add<RenderComponent>();
			renderer->SetMesh(BathroomMesh);
			renderer->SetMaterial(BathroomMaterial);
		}

		GameObject::Sptr DuckModel = scene->CreateGameObject("DuckModel");
		{
			// Set position in the scene
			DuckModel->SetPosition(glm::vec3(-1.54f, 4.36f, 3.72f));
			DuckModel->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			DuckModel->SetScale(glm::vec3(0.4, 0.4, 0.4));
			RenderComponent::Sptr renderer = DuckModel->Add<RenderComponent>();
			renderer->SetMesh(DuckMesh);
			renderer->SetMaterial(DuckMaterial);

			TriggerVolume::Sptr volume = DuckModel->Add<TriggerVolume>();
			SphereCollider::Sptr collider = SphereCollider::Create(0.91f);
			volume->AddCollider(collider);

			InteractableObjectBehaviour::Sptr interactions = DuckModel->Add<InteractableObjectBehaviour>();
			interactions->AddRewardMaterial(HandDuckMaterial);

	
			interactions->AddFeedbackBehaviour((InteractionFeedback(DuckScreenMaterial, extraScreen)));
			InteractionTForm screenTF(InteractionTForm::tformt::pos, glm::vec3(5.87f, 5.79f, 6.9f));
			interactions->AddFeedbackBehaviour((InteractionFeedback(std::vector<InteractionTForm>{screenTF}, extraScreen)));
			interactions->AddFeedbackBehaviour((InteractionFeedback(FlatDuckMesh, DuckModel)));
			InteractionTForm duckPos(InteractionTForm::tformt::pos, glm::vec3(1.87f, 1.46f, 1.98f));
			InteractionTForm duckRot(InteractionTForm::tformt::rot, glm::vec3(90.0f, 0.0f, 0.0f));
			interactions->AddFeedbackBehaviour((InteractionFeedback(std::vector<InteractionTForm>{duckPos, duckRot}, DuckModel)));
			
			interactions->AddFeedbackBehaviour((InteractionFeedback(1)));
		}


		GameObject::Sptr ToiletModel = scene->CreateGameObject("ToiletModel");
		{
			// Set position in the scene
			ToiletModel->SetPosition(glm::vec3(1.13f, -0.47f, 2.97f));
			ToiletModel->SetRotation(glm::vec3(90.0f, 0.0f, 59.00f));

			RenderComponent::Sptr renderer = ToiletModel->Add<RenderComponent>();
			renderer->SetMesh(ToiletMesh);
			renderer->SetMaterial(ToiletMaterial);

			TriggerVolume::Sptr volume = ToiletModel->Add<TriggerVolume>();
			SphereCollider::Sptr collider = SphereCollider::Create(1.26f);
			volume->AddCollider(collider);

			InteractableObjectBehaviour::Sptr interactions = ToiletModel->Add<InteractableObjectBehaviour>();
			interactions->AddRewardMaterial(HandDirtyMaterial);

			interactions->AddFeedbackBehaviour((InteractionFeedback(ToiletScreenMaterial, extraScreen)));
			InteractionTForm screenTF(InteractionTForm::tformt::pos, glm::vec3(5.87f, 5.79f, 6.9f));
			interactions->AddFeedbackBehaviour((InteractionFeedback(std::vector<InteractionTForm>{screenTF}, extraScreen)));
			interactions->AddFeedbackBehaviour((InteractionFeedback(DirtyToiletMaterial, ToiletModel)));


			interactions->AddFeedbackBehaviour((InteractionFeedback(0)));
		}

		GameObject::Sptr SoapModel = scene->CreateGameObject("SoapModel");
		{
			// Set position in the scene
			SoapModel->SetPosition(glm::vec3(4.41f, -4.67f, 5.64f));
			SoapModel->SetRotation(glm::vec3(90.0f, 0.00f, -75.00f));
			SoapModel->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

	
			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = SoapModel->Add<RenderComponent>();
			renderer->SetMesh(SoapMesh);
			renderer->SetMaterial(SoapMaterial);

			TriggerVolume::Sptr volume = SoapModel->Add<TriggerVolume>();
			SphereCollider::Sptr collider = SphereCollider::Create(1.01f);
			volume->AddCollider(collider);

			InteractableObjectBehaviour::Sptr interactions = SoapModel->Add<InteractableObjectBehaviour>();
			interactions->AddRewardMaterial(HandCleanMaterial);


			interactions->AddFeedbackBehaviour((InteractionFeedback(SoapScreenMaterial, extraScreen)));
			InteractionTForm screenTF(InteractionTForm::tformt::pos, glm::vec3(5.87f, 5.79f, 6.9f));
			interactions->AddFeedbackBehaviour((InteractionFeedback(std::vector<InteractionTForm>{screenTF}, extraScreen)));
			interactions->AddFeedbackBehaviour(InteractionFeedback(SpilledMesh, SoapModel));
			interactions->AddFeedbackBehaviour(InteractionFeedback(SpilledSoapMaterial, SoapModel));
			InteractionTForm soapPos(InteractionTForm::tformt::pos, glm::vec3(4.73f, -1.13f, 2.02f));
			InteractionTForm soapRot(InteractionTForm::tformt::rot, glm::vec3(90.0f, 0.0f, 35.0f));
			interactions->AddFeedbackBehaviour(InteractionFeedback(std::vector<InteractionTForm>{soapPos, soapRot}, SoapModel));

			interactions->AddFeedbackBehaviour((InteractionFeedback(2)));
		}

		GameObject::Sptr hand = scene->CreateGameObject("Idle Hand");
		{
			hand->SetPosition(glm::vec3(5.5f, -0.16f, 2.66f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = hand->Add<RenderComponent>();
			renderer->SetMesh(HandMesh);
			renderer->SetMaterial(HandMaterial);

			RigidBody::Sptr physics = hand->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(BoxCollider::Create(glm::vec3(1.7, 0.9, 0.8)));
		

			CharacterController::Sptr controller = hand->Add<CharacterController>();

			SkinManager::Sptr skinSwapper = hand->Add<SkinManager>();
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
