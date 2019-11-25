#pragma once
class Camera
{
public:
	Camera(v3f const& worldPosition, v3f const& targetWorldPosition);
	void setPosition(v3f const& worldPosition);
	void setTarget(v3f const& worldPosition);
	v3f const& getWorldPosition() const;
	v3f getWorldForward() const;
	v3f getWorldRight() const;
	glm::mat4 getTransform() const;
	float getFieldOfViewRadians() const;
	float getClipDistanceNear() const;
	float getClipDistanceFar() const;
	void pitch(float deltaRadians);
	void yaw(float deltaRadians);
private:
	v3f position;
	float pitchRadians;
	float yawRadians;
	float fieldOfViewRadians = glm::radians(45.f);
	float clipDistanceNear = 1.f;
	float clipDistanceFar = 1000.f;
};
