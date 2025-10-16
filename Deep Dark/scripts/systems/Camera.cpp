#include "Camera.h"
#include <math.h>
#include <iostream>

using Key = sf::Keyboard::Key;
auto MOUSE_1 = sf::Mouse::Button::Left;
auto MOUSE_2 = sf::Mouse::Button::Right;
const int FIXED_FRAMERATE = 30;
const float PAN_MULTIPLIER = 3.f;

Camera::Camera(sf::RenderWindow& window) : window(window) {
	view = window.getDefaultView();
	uiView = view;
	pos = view.getCenter();
	sf::Vector2f size = view.getSize() * 2.f;
	sf::Vector2f topLeft = { -100,-100 };//{ -ASPECT_WIDTH * 0.5f, -ASPECT_HEIGHT * 0.5f };
	sf::FloatRect bound(topLeft, size);
	set_bounds(bound);
}
void Camera::update(float deltaTime) {
	draw_all_ui();
	if (has_velocity())
		apply_velocity(deltaTime);
}
void Camera::draw_all_ui() {
	window.setView(uiView);

	for (auto& p_Draw : uiDrawQueue) 
		if (p_Draw) window.draw(*p_Draw);
	uiDrawQueue.clear();

	for (auto& u_draw : tempUIDrawQueue)
		if (u_draw) window.draw(*u_draw);
	tempUIDrawQueue.clear();

	window.setView(view);
}

void Camera::handle_events(sf::Event event) {
	if (auto key = event.getIf<sf::Event::KeyPressed>()) {
		move(key->code);
		zoom(key->code);
	}
	else if (auto release = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (release->button == MOUSE_1) {
			velocity *= PAN_MULTIPLIER;
			//std::cout << "released camera drag. Velocity = (" << velocity.x
			//	<< ", " << velocity.y << ")" << std::endl;
			dragging = false;
		}
	}
	else if (auto scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
		zoom(*scroll);
	}
}
void Camera::register_click(sf::Event event) {
	if (auto click = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (click->button == MOUSE_1) {
			velocity = { 0.f,0.f };
			dragOrigin = mousePos;
			dragging = true;
		}
	}
}

// Zoom
void Camera::zoom(Key key) {
	if (key == Key::O)
		zoomLevel = std::min(zoomLevel + ZOOM_SPEED, MAX_ZOOM);
	else if (key == Key::I)
		zoomLevel = std::max(zoomLevel - ZOOM_SPEED, MIN_ZOOM);

	sf::Vector2f newSize = (sf::Vector2f)window.getSize() * zoomLevel;
	view.setSize(newSize);
	update_pos(pos);
}
void Camera::zoom(sf::Event::MouseWheelScrolled scroll) {
	float oldZoom = zoomLevel;
	zoomLevel -= scroll.delta * FIXED_DELTA_TIME;
	zoomLevel = std::clamp(zoomLevel, MIN_ZOOM, MAX_ZOOM);
	float zoomFactor = zoomLevel / oldZoom;

	sf::Vector2f offset = mousePos - pos;
	sf::Vector2f newOffset = offset * zoomFactor;
	sf::Vector2f newPos = mousePos - newOffset;

	sf::Vector2f newSize = (sf::Vector2f)window.getSize() * zoomLevel;
	view.setSize(newSize);
	update_pos(newPos);
}

// Moving
void Camera::update_pos(sf::Vector2f newPos) {
	pos = get_pos_within_bounds(newPos);
	view.setCenter(pos);

	window.setView(view);
}
void Camera::apply_velocity(float deltaTime) {
	sf::Vector2f newPos = pos + (velocity * deltaTime);
	update_pos(newPos);

 	velocity *= std::pow(FRICTION, deltaTime * FIXED_FRAMERATE);
}
void Camera::move(Key key) {
	if ((int)key < 71 || (int)key > 74 || dragging) return;
	velocity = { 0.f,0.f };
	sf::Vector2f newPos = pos;

	if (key == Key::Up) newPos.y -= scrollSpeed;
	else if (key == Key::Down) newPos.y += scrollSpeed;
	if (key == Key::Right) newPos.x += scrollSpeed;
	else if (key == Key::Left) newPos.x -= scrollSpeed;

	update_pos(newPos);
}
void Camera::click_and_drag() {
	// main.cpp will call this while dragging == true
	sf::Vector2f diff = mousePos - pos;
	sf::Vector2f newPos = (dragOrigin - diff);

	sf::Vector2f instantVelocity = (newPos - pos) * (float)FIXED_FRAMERATE;
	velocity = (velocity * 0.8f) + (instantVelocity * 0.2f);

	update_pos(newPos);
}
sf::Vector2f Camera::get_pos_within_bounds(sf::Vector2f targetPos) {
	sf::Vector2f halfSize = view.getSize() * 0.5f;

	//bounds.he
	float x = std::clamp(targetPos.x, limits.left + halfSize.x, limits.right - halfSize.x);
	float y = std::clamp(targetPos.y, limits.top + halfSize.y, limits.bottom - halfSize.y);

	return sf::Vector2f(x, y);
}
