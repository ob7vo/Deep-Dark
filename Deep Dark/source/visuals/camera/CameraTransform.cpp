#include "pch.h"
#include "CameraTransform.h"
#include "Camera.h"
using Key = sf::Keyboard::Key;

const float MIN_ZOOM = 0.6f;
const float MAX_ZOOM = 1.4f;
const float FRICTION = 0.85f;
const float ZOOM_SPEED = 0.1f;
const int FIXED_FRAMERATE = 30;
const float FIXED_DELTA_TIME = 0.0333f;
const float PAN_MULTIPLIER = 3.f;
const float SCROLL_SPEED = 5.f;

CameraTransform::CameraTransform(Camera& cam) : cam(cam) {}

void CameraTransform::move(Key key) {
	if (cam.cursor.dragging) return;

	cam.cursor.velocity = { 0.f,0.f };
	sf::Vector2f newPos = worldPos;

	if (key == Key::Up) newPos.y -= SCROLL_SPEED;
	else if (key == Key::Down) newPos.y += SCROLL_SPEED;
	if (key == Key::Right) newPos.x += SCROLL_SPEED;
	else if (key == Key::Left) newPos.x -= SCROLL_SPEED;

	update_pos(newPos);
}
void CameraTransform::click_and_drag() {
	// main.cpp will call this while dragging == true
	sf::Vector2f diff = cam.cursor.worldPos - worldPos;
	sf::Vector2f newPos = (cam.cursor.dragOrigin - diff);

	sf::Vector2f instantVelocity = (newPos - worldPos) * (float)FIXED_FRAMERATE;
	cam.cursor.velocity = (cam.cursor.velocity * 0.8f) + (instantVelocity * 0.2f);

	update_pos(newPos);
}

// Zoom
void CameraTransform::zoom(Key key) {
	float oldZoom = zoomLevel;

	if (key == Key::O)
		zoomLevel = std::min(zoomLevel + ZOOM_SPEED, MAX_ZOOM);
	else if (key == Key::I)
		zoomLevel = std::max(zoomLevel - ZOOM_SPEED, MIN_ZOOM);

	if (oldZoom == zoomLevel) return;

	sf::Vector2f newSize = (sf::Vector2f)cam.window.getSize() * zoomLevel;

	cam.worldView.setSize(newSize);
	update_pos(worldPos);
}
void CameraTransform::zoom(sf::Event::MouseWheelScrolled scroll) {
	float oldZoom = zoomLevel;
	zoomLevel -= scroll.delta * FIXED_DELTA_TIME;
	zoomLevel = std::clamp(zoomLevel, MIN_ZOOM, MAX_ZOOM);
	float zoomFactor = zoomLevel / oldZoom;

	sf::Vector2f offset = cam.cursor.worldPos - worldPos;
	sf::Vector2f newOffset = offset * zoomFactor;
	sf::Vector2f newPos = cam.cursor.worldPos - newOffset;

	sf::Vector2f newSize = (sf::Vector2f)cam.window.getSize() * zoomLevel;

	cam.worldView.setSize(newSize);
	update_pos(newPos);
}
void CameraTransform::set_zoom(float newZoomLevel) {
	zoomLevel = std::clamp(newZoomLevel, MIN_ZOOM, MAX_ZOOM);

	sf::Vector2f newSize = (sf::Vector2f)cam.window.getSize() * zoomLevel;

	cam.worldView.setSize(newSize);
	update_pos(worldPos);
}

// Moving
void CameraTransform::update_pos(sf::Vector2f newPos) {
	worldPos = get_pos_within_bounds(newPos);
	cam.update_pos();
}

sf::Vector2f CameraTransform::get_pos_within_bounds(sf::Vector2f targetPos) const {
	sf::Vector2f halfSize = cam.worldView.getSize() * 0.5f;

	//bounds.he
	float x = std::clamp(targetPos.x, (limits.left + halfSize.x), (limits.right - halfSize.x));
	float y = std::clamp(targetPos.y, limits.top + halfSize.y, limits.bottom - halfSize.y);

	return sf::Vector2f(x, y);
}
void CameraTransform::apply_velocity(float deltaTime) {
	sf::Vector2f newPos = worldPos + (cam.cursor.velocity * deltaTime);
	update_pos(newPos);

	cam.cursor.velocity *= std::pow(FRICTION, deltaTime * FIXED_FRAMERATE);
}
