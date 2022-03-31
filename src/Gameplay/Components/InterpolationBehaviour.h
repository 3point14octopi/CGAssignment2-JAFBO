#pragma once
#include "IComponent.h"

/// <summary>
/// a very simple, optional enum to make function calls a bit easier
/// </summary>
enum doesItLoop {
	Non_Repeating,
	Repeating
};

/// <summary>
/// types of transformations 
/// (translation, rotation, and scale)
/// </summary>
enum TFormType {
	TRANSLATION,
	ROTATION,
	SCALE
};

/// <summary>
/// A structure that holds the frame data for a single transformation
/// (translation, rotation, or scaling)
/// </summary>
struct TFormFrame {
	TFormType tFormType;
	float tFormLength;
	glm::vec3 tFinalPose;

	/// <summary>
	/// constructor for a single frame data structure
	/// </summary>
	/// <param name="tft: ">the type of transformation (translation, rotation, scale)</param>
	/// <param name="tfl: ">the duration of this frame in seconds</param>
	/// <param name="tfp: ">the final value of the transformation as XYZ</param>
	TFormFrame(TFormType tft, float tfl, glm::vec3 tfp) {
		tFormType = tft;
		tFormLength = tfl;
		tFinalPose = tfp;
	}
};

/// <summary>
/// A structure that makes up a singular behaviour/animation/whatever
/// </summary>
struct TFormAnim {
	std::vector<std::vector<TFormFrame>> animKeys;
	std::string animName;
	int transformIndexes[3]{ 0, 0, 0 };
	float transformTimes[3]{ 0, 0, 0 };
	bool transformFinished[3]{ 0, 0, 0 };
	bool hasFrames[3]{ 0, 0, 0 };

	/// <summary>
	/// constructor for a TFormAnim
	/// </summary>
	/// <param name="posFrames: ">a vector holding each position data frame</param>
	/// <param name="rotFrames: ">a vector holding each rotation data frame</param>
	/// <param name="sclFrames: ">a vector holding each scale data frame</param>
	/// <param name="aName: ">a string by which we can identify this behaviour</param>
	TFormAnim(std::vector<TFormFrame> posFrames, std::vector<TFormFrame> rotFrames, std::vector<TFormFrame> sclFrames, std::string aName) {
		animKeys.push_back(posFrames);
		animKeys.push_back(rotFrames);
		animKeys.push_back(sclFrames);

		animName = aName;

		for (int i = 0; i < 3; i++) { hasFrames[i] = (animKeys[i].size() > 0); }
	}
};

/// <summary>
/// A simple linear interpolation system that allows an object to follow a set path
///  or a set sequence of transformations. 
/// 
/// Multiple paths/sequences can be stored, but only one will be accessed at a time. 
/// Toggling is performed via the "ToggleBehaviour" function
/// </summary>
class InterpolationBehaviour : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<InterpolationBehaviour> Sptr;
	InterpolationBehaviour();
	~InterpolationBehaviour();

	virtual void Update(float deltaTime) override;
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static InterpolationBehaviour::Sptr FromJson(const nlohmann::json& data);

	void ToggleBehaviour(std::string newBehaviour, bool willLoop);
	void ToggleBehaviour(int, bool);
	void PauseOrResumeCurrentBehaviour();

	void InterpolationManager(float deltaTime);
	glm::vec3 PerformTransformation(float deltaTime, int index, std::vector<TFormFrame> transformKeyFrames);
	template <typename T> T LERP(T p0, T p1, float t);


	void StartPushNewBehaviour(std::string bName);
	void AddKeyFrame(TFormType, float, glm::vec3);
	void EndPushNewBehaviour();

	void AddBehaviourScript(std::string path);

	MAKE_TYPENAME(InterpolationBehaviour);
	
	bool _isRunning;
protected:

	std::vector<TFormAnim> _allTransforms;
	int _currentTransformIndex;
	int _amountOfTransforms;
	bool _loopTransform;
	
	int cooldown;

	std::vector<std::string> loadedScripts;

private:
	//these variables are used during initiation 
	//they should not be accessed after start-up is complete, as they will be empty
	std::vector<TFormFrame> posFrames;
	std::vector<TFormFrame> rotFrames;
	std::vector<TFormFrame> sclFrames;
	std::string tName;
};
