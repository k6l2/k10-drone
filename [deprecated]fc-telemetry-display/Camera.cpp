#include "Camera.h"
Camera::Camera(v3f const& worldPosition, v3f const& targetWorldPosition)
	: position(worldPosition)
{
	setTarget(targetWorldPosition);
}
void Camera::setPosition(v3f const& worldPosition)
{
	position = worldPosition;
}
void Camera::setTarget(v3f const& worldPosition)
{
	const v3f desiredNormForward = glm::normalize(worldPosition - position);
	yawRadians = k10::radiansAroundAxis(desiredNormForward, k10::WORLD_UP, k10::FORWARD);
	const v3f yawRight   = glm::rotate(k10::RIGHT  , yawRadians, k10::WORLD_UP);
	const v3f yawForward = glm::rotate(k10::FORWARD, yawRadians, k10::WORLD_UP);
	pitchRadians = k10::radiansAroundAxis(desiredNormForward, yawRight, yawForward);
}
v3f const& Camera::getWorldPosition() const
{
	return position;
}
v3f Camera::getWorldForward() const
{
	return glm::rotate(glm::rotate(
		k10::FORWARD, pitchRadians, k10::RIGHT),
		              yawRadians  , k10::WORLD_UP);
}
v3f Camera::getWorldRight() const
{
	const v3f normForward = getWorldForward();
	return glm::normalize(glm::cross(normForward, k10::WORLD_UP));
}
glm::mat4 Camera::getTransform() const
{

	return glm::lookAt(position, position + getWorldForward(), k10::WORLD_UP);
}
float Camera::getFieldOfViewRadians() const
{
	return fieldOfViewRadians;
}
float Camera::getClipDistanceNear() const
{
	return clipDistanceNear;
}
float Camera::getClipDistanceFar() const
{
	return clipDistanceFar;
}
void Camera::pitch(float deltaRadians)
{
	static const float MAX_PITCH = k10::PI * 0.49f;
	pitchRadians += deltaRadians;
	if (pitchRadians < -MAX_PITCH)
	{
		pitchRadians = -MAX_PITCH;
	}
	if (pitchRadians > MAX_PITCH)
	{
		pitchRadians = MAX_PITCH;
	}
}
void Camera::yaw(float deltaRadians)
{
	yawRadians += deltaRadians;
	yawRadians = fmodf(yawRadians, 2*k10::PI);
}