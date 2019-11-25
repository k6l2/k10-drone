#include "Input.h"
Input::Input()
{
	bindAxis("camForward"      , SDL_SCANCODE_E                  ,  1.f);
	bindAxis("camForward"      , SDL_SCANCODE_D                  , -1.f);
	bindAxis("camForward"      , SDL_CONTROLLER_AXIS_LEFTY       , -1.f);
	bindAxis("camRight"        , SDL_SCANCODE_S                  , -1.f);
	bindAxis("camRight"        , SDL_SCANCODE_F                  ,  1.f);
	bindAxis("camRight"        , SDL_CONTROLLER_AXIS_LEFTX       ,  1.f);
	bindAxis("camElevate"      , SDL_SCANCODE_A                  , -1.f);
	bindAxis("camElevate"      , SDL_SCANCODE_SPACE              ,  1.f);
	bindAxis("camElevate"      , SDL_CONTROLLER_AXIS_TRIGGERLEFT , -1.f);
	bindAxis("camElevate"      , SDL_CONTROLLER_AXIS_TRIGGERRIGHT,  1.f);
	bindAxis("camLookYaw"      , MouseAxis::DX                   , -1.f);
	bindAxis("camLookPitch"    , MouseAxis::DY                   , -1.f);
	bindAxis("camLookRateYaw"  , SDL_CONTROLLER_AXIS_RIGHTX      , -1.f);
	bindAxis("camLookRatePitch", SDL_CONTROLLER_AXIS_RIGHTY      , -1.f);
	bindAction("quickExitActive", SDL_SCANCODE_ESCAPE        );
	bindAction("quickExitActive", SDL_CONTROLLER_BUTTON_BACK );
	bindAction("quickExit"      , SDL_SCANCODE_ESCAPE        );
	bindAction("quickExit"      , SDL_CONTROLLER_BUTTON_START);
}
void Input::step()
{
	// update keyboard SDL_Scancode states //
	{
		int numKeys;
		SDL_GetKeyboardState(&numKeys);
		Uint8 const* keyboardState = SDL_GetKeyboardState(nullptr);
		keyStatePrev = keyState;
		keyStatePrev.resize(numKeys);
		keyState = vector<Uint8>(keyboardState, keyboardState + numKeys);
	}
	// update mouse delta state //
	mouseDelta = { 0,0 };
	mouseButtonState = SDL_GetMouseState(nullptr, nullptr);
	// update action states //
	actionStatesPrev = actionStates;
	for (auto actionMapIt = actionMap.begin(); 
		 actionMapIt != actionMap.end(); ++actionMapIt)
	{
		bool actionState = false;
		for (InputActionMeta const& iam : actionMapIt->second)
		{
			switch (iam.type)
			{
			case Type::KEYBOARD:
				actionState |= static_cast<bool>(keyState[iam.iDevMeta.key]);
				break;
			case Type::MOUSE:
				actionState |= static_cast<bool>(
					mouseButtonState & SDL_BUTTON(iam.iDevMeta.mouseButton));
				break;
			case Type::GAMECONTROLLER:
				if (gameControllers.empty())
				{
					break;
				}
				actionState |= static_cast<bool>(
					SDL_GameControllerGetButton(gameControllers.front(), 
												iam.iDevMeta.gcButton));
				break;
			}
		}
		actionStates[actionMapIt->first] = actionState;
	}
}
void Input::processEvent(SDL_Event const& e)
{
	switch (e.type)
	{
	case SDL_EventType::SDL_MOUSEMOTION: {
		if (!SDL_GetRelativeMouseMode())
		{
			break;
		}
		mouseDelta += v2i{ e.motion.xrel, e.motion.yrel };
	}break;
	case SDL_EventType::SDL_JOYDEVICEADDED: {
		//SDL_Log("SDL_JOYDEVICEADDED\n");
	}break;
	case SDL_EventType::SDL_CONTROLLERDEVICEADDED: {
		SDL_Log("SDL_CONTROLLERDEVICEADDED - joystickDeviceId=%i\n",
				e.cdevice.which);
		if (!SDL_IsGameController(e.cdevice.which))
		{
			SDL_Log("Joystick is not a game controller, skipping...\n");
			break;
		}
		SDL_GameController*const newController =
			SDL_GameControllerOpen(e.cdevice.which);
		if (!newController)
		{
			SDL_assert(false);
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						 "Failed to get game controller from joy id=%i\n",
						 e.cdevice.which);
			break;
		}
		gameControllers.push_back(newController);
		
	}break;
	case SDL_EventType::SDL_CONTROLLERDEVICEREMOVED: {
		SDL_Log("SDL_CONTROLLERDEVICEREMOVED - instanceId=%i\n",
				e.cdevice.which);
		for (size_t gc = 0; gc < gameControllers.size(); gc++)
		{
			SDL_Joystick*const gcJoy = 
				SDL_GameControllerGetJoystick(gameControllers[gc]);
			if (!gcJoy)
			{
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					"Failed to get game controller joystick for [%i]\n", gc);
				continue;
			}
			if (e.cdevice.which == SDL_JoystickInstanceID(gcJoy))
			{
				SDL_GameControllerClose(gameControllers[gc]);
				gameControllers.erase(gameControllers.begin() + gc);
			}
		}
	}break;
	}
}
void Input::bindAction(char const* actionName, SDL_Scancode key)
{
	bindAction(actionName, Type::KEYBOARD, { key });
}
void Input::bindAction(char const* actionName, MouseButton mButton)
{
	bindAction(actionName, Type::MOUSE, { mButton });
}
void Input::bindAction(char const* actionName, SDL_GameControllerButton gcb)
{
	bindAction(actionName, Type::GAMECONTROLLER, { gcb });
}
void Input::bindAxis(char const* axisName, SDL_Scancode key, float factor)
{
	bindAxis(axisName, Type::KEYBOARD, { key }, factor);
}
void Input::bindAxis(char const* axisName, MouseAxis ma, float factor)
{
	bindAxis(axisName, Type::MOUSE, { ma }, factor);
}
void Input::bindAxis(char const* axisName, SDL_GameControllerAxis gca, float factor)
{
	bindAxis(axisName, Type::GAMECONTROLLER, { gca }, factor);
}
bool Input::actionPressed(char const* actionName) const
{
	auto actionMapIt     = actionStates    .find(actionName);
	auto actionMapPrevIt = actionStatesPrev.find(actionName);
	if (actionMapIt == actionStates.end() ||
		actionMapPrevIt == actionStatesPrev.end())
	{
		return false;
	}
	return actionMapIt->second && !actionMapPrevIt->second;

}
bool Input::actionHeld(char const* actionName) const
{
	auto actionMapIt = actionStates.find(actionName);
	if (actionMapIt == actionStates.end())
	{
		return false;
	}
	return actionMapIt->second;
}
float Input::axis(char const* axisName)
{
	auto axisMapIt = axisMap.find(axisName);
	if (axisMapIt == axisMap.end())
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
			"Attempting to get unmapped axis '%s'!\n", axisName);
		return 0.f;
	}
	float retVal = 0;
	for (InputAxisMeta const& iam : axisMapIt->second)
	{
		switch (iam.type)
		{
		case Type::KEYBOARD:
			if (keyState[iam.iDevMeta.key])
			{
				retVal += iam.factor;
			}
			break;
		case Type::MOUSE:
			switch (iam.iDevMeta.mouseAxis)
			{
			case MouseAxis::DX:
				retVal += iam.factor * mouseDelta.x;
				break;
			case MouseAxis::DY:
				retVal += iam.factor * mouseDelta.y;
				break;
			}
			break;
		case Type::GAMECONTROLLER:
			if (gameControllers.empty())
			{
				break;
			}
			const Sint16 gcAxisValueRaw = 
				SDL_GameControllerGetAxis(gameControllers.front(), 
										  iam.iDevMeta.gcAxis);
			const float gcAxisNormal = iam.factor *
				(gcAxisValueRaw / static_cast<float>(0x7FFF));
			if (fabsf(gcAxisNormal) <= controllerAxisDeadzone)
			{
				break;
			}
			retVal += gcAxisNormal;
		}
	}
	return retVal;
}
size_t Input::findExistingAction(vector<InputActionMeta>const& actionList,
								 Type type, InputDeviceMeta idm)
{
	for (size_t i = 0; i < actionList.size(); i++)
	{
		if (actionList[i].type != type)
		{
			continue;
		}
		switch (type)
		{
		case Type::MOUSE:
			if (idm.mouseButton == actionList[i].iDevMeta.mouseButton)
			{
				return i;
			}
			break;
		case Type::KEYBOARD:
			if (idm.key == actionList[i].iDevMeta.key)
			{
				return i;
			}
			break;
		case Type::GAMECONTROLLER:
			if (idm.gcButton == actionList[i].iDevMeta.gcButton)
			{
				return i;
			}
			break;
		}
	}
	return actionList.size();
}
size_t Input::findExistingAxis(vector<InputAxisMeta>const& axisList,
							   Type type, InputDeviceMeta idm)
{
	for (size_t i = 0; i < axisList.size(); i++)
	{
		if (axisList[i].type != type)
		{
			continue;
		}
		switch (type)
		{
		case Type::MOUSE:
			if (idm.mouseAxis == axisList[i].iDevMeta.mouseAxis)
			{
				return i;
			}
			break;
		case Type::KEYBOARD:
			if (idm.key == axisList[i].iDevMeta.key)
			{
				return i;
			}
			break;
		case Type::GAMECONTROLLER:
			if (idm.gcAxis == axisList[i].iDevMeta.gcAxis)
			{
				return i;
			}
			break;
		}
	}
	return axisList.size();
}
void Input::bindAction(char const* actionName, Type type, InputDeviceMeta idm)
{
	auto actionMapIt = actionMap.find(actionName);
	if (actionMapIt == actionMap.end())
	{
		actionMap.insert({ actionName, {} });
		actionMapIt = actionMap.find(actionName);
		SDL_assert(actionMapIt != actionMap.end());
	}
	const size_t existingIndex =
		findExistingAction(actionMapIt->second, type, idm);
	if (existingIndex == actionMapIt->second.size())
	{
		// we have never mapped to this input yet //
		actionMapIt->second.push_back({ type, idm });
	}
	else
	{
		// this action has already been mapped //
		SDL_assert(false);
	}
}
void Input::bindAxis(char const* axisName, Type type, InputDeviceMeta idm, float factor)
{
	auto axisMapIt = axisMap.find(axisName);
	if (axisMapIt == axisMap.end())
	{
		axisMap.insert({ axisName, {} });
		axisMapIt = axisMap.find(axisName);
		SDL_assert(axisMapIt != axisMap.end());
	}
	const size_t existingIndex = 
		findExistingAxis(axisMapIt->second, type, idm);
	if (existingIndex == axisMapIt->second.size())
	{
		// we have never mapped to this input yet //
		axisMapIt->second.push_back({ type, idm, factor });
	}
	else
	{
		// this axis has already been mapped //
		SDL_assert(false);
	}
}