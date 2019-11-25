#pragma once
class Input
{
public:
	// mouse DX/DY refer to change in mouse motion between frames
	enum class MouseAxis : u8 { DX, DY };
	enum MouseButton : u8
	{
		LEFT = SDL_BUTTON_LEFT,
		MIDDLE,
		RIGHT,
		X1,
		X2
	};
public:
	// default constructor sets up default bindings (for now, until there is a
	//	global config database) //
	Input();
	// make sure to call this BEFORE processEvent //
	void step();
	void processEvent(SDL_Event const& e);
	void bindAction(char const* actionName, SDL_Scancode key);
	void bindAction(char const* actionName, MouseButton mButton);
	void bindAction(char const* actionName, SDL_GameControllerButton gcb);
	void bindAxis(char const* axisName, SDL_Scancode key, float factor);
	void bindAxis(char const* axisName, MouseAxis ma, float factor);
	void bindAxis(char const* axisName, SDL_GameControllerAxis gca, float factor);
	bool actionPressed(char const* actionName) const;
	bool actionHeld(char const* actionName) const;
	float axis(char const* axisName);
private:
	union InputDeviceMeta
	{
		SDL_Scancode key;
		MouseAxis mouseAxis;
		MouseButton mouseButton;
		SDL_GameControllerButton gcButton;
		SDL_GameControllerAxis gcAxis;
		InputDeviceMeta(SDL_Scancode sc)
		{
			key = sc;
		}
		InputDeviceMeta(MouseAxis ma)
		{
			mouseAxis = ma;
		}
		InputDeviceMeta(MouseButton mb)
		{
			mouseButton = mb;
		}
		InputDeviceMeta(SDL_GameControllerButton gcb)
		{
			gcButton = gcb;
		}
		InputDeviceMeta(SDL_GameControllerAxis gca)
		{
			gcAxis = gca;
		}
	};
	enum class Type : u8
	{
		KEYBOARD,
		MOUSE,
		GAMECONTROLLER
	};
	struct InputActionMeta
	{
		Type type;
		InputDeviceMeta iDevMeta;
	};
	struct InputAxisMeta
	{
		Type type;
		InputDeviceMeta iDevMeta;
		float factor;
	};
private:
	static size_t findExistingAction(vector<InputActionMeta>const& actionList,
								     Type type, InputDeviceMeta idm);
	static size_t findExistingAxis(vector<InputAxisMeta>const& axisList, 
								   Type type, InputDeviceMeta idm);
private:
	void bindAction(char const* actionName, Type type, InputDeviceMeta idm);
	void bindAxis(char const* axisName, Type type, InputDeviceMeta idm, float factor);
private:
	map<string, vector<InputActionMeta>> actionMap;
	map<string, vector<InputAxisMeta  >> axisMap;
	vector<Uint8> keyState;
	vector<Uint8> keyStatePrev;
	v2i mouseDelta;
	Uint32 mouseButtonState;
	vector<SDL_GameController*> gameControllers;
	float controllerAxisDeadzone = 0.1f;
	map<string, bool> actionStates;
	map<string, bool> actionStatesPrev;
};
