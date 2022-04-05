#include "Gameplay/Components/InterpolationBehaviour.h"
#include "Gameplay/GameObject.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/Scene.h"
#include "Gameplay/InputEngine.h"
#include "GLFW/glfw3.h"

#include <fstream>

InterpolationBehaviour::InterpolationBehaviour() :IComponent() {
	//we will assume that the first behaviour pushed is the one to use, 
	//and that it should constantly loop
	_currentTransformIndex = 0;
	_loopTransform = true;
	_isRunning = true;
	_amountOfTransforms = 0;
	cooldown = 0;
}


InterpolationBehaviour::~InterpolationBehaviour() = default;

/// <summary>
/// Allows the user to specify which interpolation behaviour to use, and whether or not it will loop. 
/// By default, the system will set the current behaviour to the first behaviour that was pushed, and will loop.
/// </summary>
/// <param name="name: ">the name of the interpolation behaviour that has already been pushed</param>
/// <param name="loops: ">if we want the behaviour to loop or run through once and stop</param>
void InterpolationBehaviour::ToggleBehaviour(std::string name, bool loops) {

	//resets current behaviour
	for (int i = 0; i < 3; i++) {
		(_allTransforms[_currentTransformIndex]).transformFinished[i] = 0;
		(_allTransforms[_currentTransformIndex]).transformIndexes[i] = 0;
		(_allTransforms[_currentTransformIndex]).transformTimes[i] = 0;
	}

	//searches for behaviour with specified name
	for (int i = 0; i < _allTransforms.size(); i++) {
		if (_allTransforms[i].animName == name) {
			_currentTransformIndex = i;
			break;
		}
	}

	//tells system whether the behaviour needs to loop or not
	_loopTransform = loops;
}

/// <summary>
/// Allows the user to specify which interpolation behaviour to use, and whether or not it will loop. 
/// By default, the system will set the current behaviour to the first behaviour that was pushed, and will loop.
/// </summary>
/// <param name="index: ">index of the behaviour in the list</param>
/// <param name="loops: ">if we want the behaviour to loop or run through once and stop</param>
void InterpolationBehaviour::ToggleBehaviour(int index, bool loops) {

	//resets current behaviour
	for (int i = 0; i < 3; i++) {
		(_allTransforms[_currentTransformIndex]).transformFinished[i] = 0;
		(_allTransforms[_currentTransformIndex]).transformIndexes[i] = 0;
		(_allTransforms[_currentTransformIndex]).transformTimes[i] = 0;
	}

	//sets to specified index
	_currentTransformIndex = index;

	//tells system whether the behaviour needs to loop or not
	_loopTransform = loops;

}

/// <summary>
/// This function will pause or resume the current interpolation behaviour, 
/// basically it just sets _isRunning to the opposite value
/// </summary>
void InterpolationBehaviour::PauseOrResumeCurrentBehaviour() {
	_isRunning = !_isRunning;
}

void InterpolationBehaviour::Update(float deltaTime) {
	if (_isRunning) {
		InterpolationManager(deltaTime);
	}
	if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Down) {
		std::cout << "You pressed da button";
		GetGameObject()->SetPosition(glm::vec3(GetGameObject()->GetPosition().x, GetGameObject()->GetPosition().y, -50));
	}
}

/// <summary>
/// Checks to see if position, rotation, or scale need to be updated
/// </summary>
/// <param name="deltaTime"></param>
void InterpolationBehaviour::InterpolationManager(float deltaTime) {
	TFormAnim currentTransform = _allTransforms[_currentTransformIndex];
	int totalTforms = 0;
	int completedTransformsTotal = 0;
	for (int i = 0; i < 3; i++) {
		if (currentTransform.hasFrames[i]) {
			completedTransformsTotal += currentTransform.transformFinished[i];
			totalTforms++;
		}
	}

	if (completedTransformsTotal < totalTforms) {
		if(currentTransform.hasFrames[0]) if (!currentTransform.transformFinished[0]) GetGameObject()->SetPosition(PerformTransformation(deltaTime, 0, currentTransform.animKeys[0]));
		if(currentTransform.hasFrames[1]) if (!currentTransform.transformFinished[1])	GetGameObject()->SetRotation(PerformTransformation(deltaTime, 1, currentTransform.animKeys[1]));
		if(currentTransform.hasFrames[2]) if (!currentTransform.transformFinished[2])	GetGameObject()->SetScale(PerformTransformation(deltaTime, 2, currentTransform.animKeys[2]));
	}
	else {
		if (!_loopTransform) {
			_isRunning = false;
		}

		for (int i = 0; i < 3; i++) {
			if (currentTransform.hasFrames[i]) {
				(_allTransforms[_currentTransformIndex]).transformFinished[i] = 0;
				(_allTransforms[_currentTransformIndex]).transformIndexes[i] = 0;
				(_allTransforms[_currentTransformIndex]).transformTimes[i] = 0;
			}
			
		}

	}

}

/// <summary>
/// Finds the current keyframes for a transformation, calculates LERP, and returns the result
/// </summary>
/// <param name="deltaTime: ">the time since the last update</param>
/// <param name="index: ">tells us if we want to use translation(0), rotation(1), or scale(2)</param>
/// <param name="transformKeyFrames: ">the frames we are interpolating between</param>
/// <returns>the interpolated result</returns>
glm::vec3 InterpolationBehaviour::PerformTransformation(float deltaTime, int index, std::vector<TFormFrame> transformKeyFrames) {
	//making things slightly less ugly
	TFormAnim* currentTransform = &_allTransforms[_currentTransformIndex];
	//variables
	float t;
	glm::vec3 p0, p1;

	//add delta time to our current time value
	currentTransform->transformTimes[index] += deltaTime;

	//if we are over the duration of the current frame, update the index
	if (currentTransform->transformTimes[index] >= (transformKeyFrames[currentTransform->transformIndexes[index]].tFormLength)) {
		currentTransform->transformTimes[index] = 0;
		currentTransform->transformIndexes[index]++;
	}


	if (currentTransform->transformIndexes[index] >= transformKeyFrames.size() - 1) {
		//if we have reached the end of the keyframes and our behaviour isn't looping, we will say that the current transformation is complete
		if (!_loopTransform) {
			currentTransform->transformFinished[index] = true;
			//return the current value
			return transformKeyFrames[currentTransform->transformIndexes[index]].tFinalPose;
		}//if we have reached the end of the keyframes and our behaviour is looping, we will set the index back to the beginning
		else if (currentTransform->transformIndexes[index] > transformKeyFrames.size() - 1) {
			currentTransform->transformIndexes[index] = 0;
		}

	}

	//divide time by current duration
	t = currentTransform->transformTimes[index] / transformKeyFrames[currentTransform->transformIndexes[index]].tFormLength;
	//get p0 and p1
	p0 = transformKeyFrames[currentTransform->transformIndexes[index]].tFinalPose;
	p1 = (currentTransform->transformIndexes[index] != transformKeyFrames.size() - 1) ? transformKeyFrames[currentTransform->transformIndexes[index] + 1].tFinalPose : transformKeyFrames[0].tFinalPose;

	//interpolate and return result
	return LERP(p0, p1, t);
}

/// <summary>
/// performs linear interpolation
/// </summary>
/// <returns>interpolated result between p0 and p1, in respect to t</returns>
template <typename T> T InterpolationBehaviour::LERP(T p0, T p1, float t) {
	return((1.f - t) * p0) + (t * p1);
}


/// <summary>
/// The function that you need to call every time you want to start pushing a new set of transformations
/// </summary>
/// <param name="transformationName: ">An identifiable name for the transformation</param>
void InterpolationBehaviour::StartPushNewBehaviour(std::string transformationName) {
	tName = transformationName;
	_amountOfTransforms++;
}


/// <summary>
/// Pushes back a new keyframe of a specific type of transformation.
/// [NOTE: if you are interpolating between keyframes p0 and p1, the transformDuration of p0 will be used] 
/// </summary>
/// <param name="transformType: ">the type of transformation (translation, rotation, scale)</param>
/// <param name="transformDuration: ">the length of time the transformation will take</param>
/// <param name="transformPose: ">the position, rotation, or scale of the object</param>
void InterpolationBehaviour::AddKeyFrame(TFormType transformType, float transformDuration, glm::vec3 transformPose) {
	TFormFrame frame(transformType, transformDuration, transformPose);

	switch (transformType) {
	case(TRANSLATION):
		posFrames.push_back(frame);
		break;
	case(ROTATION):
		rotFrames.push_back(frame);
		break;
	default:
		sclFrames.push_back(frame);
	}
}

/// <summary>
/// Tells the class that we are done pushing the current animation.
/// More animations can be pushed by calling StartPushNewBehaviour again
/// </summary>
void InterpolationBehaviour::EndPushNewBehaviour() {
	_allTransforms.push_back(TFormAnim(posFrames, rotFrames, sclFrames, tName));
	posFrames.clear();
	rotFrames.clear();
	sclFrames.clear();
}


void InterpolationBehaviour::AddBehaviourScript(std::string path) {
	std::fstream script;
	std::string command;
	std::string n;
	int t;
	float d;
	glm::vec3 p{ 0.f };

	script.open(path);
	if (script.is_open()) {
		while (script.good()) {
			script >> command;

			if (command == "s") {
				script >> n;
				StartPushNewBehaviour(n);
			}
			else if (command == "f") {
				script >> t >> d >> p.x >> p.y >> p.z;
				AddKeyFrame((TFormType)t, d, p);
			}
			else if (command == "e") {
				EndPushNewBehaviour();
 
			}
		}
	}
	else {
		std::cout << "Could not locate " << path << " in the system. Canceling processes...\n";
		exit(-1);
	}
}


void InterpolationBehaviour::RenderImGui() {

}

nlohmann::json InterpolationBehaviour::ToJson() const {
	nlohmann::json result;

	result["is_running"] = _isRunning;
	result["is_looping"] = _loopTransform;
	result["current_index"] = _currentTransformIndex;
	result["num_of_transforms"] = _amountOfTransforms;

	result["interpolation_scripts_used"] = loadedScripts.size();
	for (int i = 0; i < loadedScripts.size(); i++) {
		result["interpolation_script_" + std::to_string(i)] = loadedScripts[i];
	}

	return result;
}

InterpolationBehaviour::Sptr InterpolationBehaviour::FromJson(const nlohmann::json & blob) {
	InterpolationBehaviour::Sptr result = std::make_shared<InterpolationBehaviour>();
	result->_isRunning = blob["is_running"];
	result->_loopTransform = blob["is_looping"];
	result->_currentTransformIndex = blob["current_index"];
	result->_amountOfTransforms = blob["num_of_transforms"];

	int numOfFiles = blob["interpolation_scripts_used"];
	for (int i = 0; i < numOfFiles; i++) {
		result->AddBehaviourScript(blob["interpolation_script_" + std::to_string(i)]);
	}

	return result;
}