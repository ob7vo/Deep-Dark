#include "pch.h"
#include "Camera.h"
#include "UILayout.h"

using namespace UI::Colors;
using Key = sf::Keyboard::Key;

const auto MOUSE_1 = sf::Mouse::Button::Left;
const auto MOUSE_2 = sf::Mouse::Button::Right;
const float PAN_MULTIPLIER = 3.f;

Camera::Camera(sf::RenderWindow& window) : window(window), renderer(*this), transform(*this) {
	worldView = uiView = window.getDefaultView();
	Screen::size = window.getSize();

	transform.worldPos = worldView.getCenter();
	window.setView(worldView);

	sf::Vector2f boundsSize = worldView.getSize() * 2.f;
	sf::Vector2f topLeft = { -100,-100 };
	transform.limits.set_bounds({ topLeft, boundsSize });
}

void Camera::reset() {
	worldView = window.getDefaultView();
	uiView = worldView;
	transform.worldPos = worldView.getCenter();
	cursor.dragging = false;
}

void Camera::update(float deltaTime) {
	if (cursor.dragging) transform.click_and_drag();
	else if (cursor.has_velocity())
		transform.apply_velocity(deltaTime);

	cursor.lastClickTime += deltaTime;
}
void Camera::update_pos() {
	worldView.setCenter(transform.worldPos);
	window.setView(worldView);
}

void Camera::change_lock(bool lock) {
	if (lock) {
		transform.locked = true;
		cursor.dragging = false;
		cursor.velocity = { 0.f, 0.f };
	}
	else
		transform.locked = false;
}
void Camera::handle_events(sf::Event event) {
	if (transform.locked) return;

	if (auto click = event.getIf<sf::Event::MouseButtonPressed>())
		on_mouse_press(*click);
	else if (auto release = event.getIf<sf::Event::MouseButtonReleased>())
		on_mouse_released(*release);
	else if (auto scroll = event.getIf<sf::Event::MouseWheelScrolled>()) 
		transform.zoom(*scroll);
}

void Camera::on_mouse_press(sf::Event::MouseButtonPressed click) {
	cursor.lastClickTime = 0.f;

	if (click.button == MOUSE_1) {
		cursor.velocity = { 0.f,0.f };
		cursor.dragOrigin = cursor.worldPos;
		cursor.dragging = true;
	}
}
void Camera::on_mouse_released(sf::Event::MouseButtonReleased release) {
	if (release.button == MOUSE_1) {
		cursor.velocity *= PAN_MULTIPLIER;
		cursor.dragging = false;
	}
}

void Camera::set_cursor_ui(const sf::Texture& uiTex, sf::Vector2f normOrigin, float opacity, sf::Vector2f normScale) {
	cursor.UITexture = uiTex;
	cursor.ui.setTexture(cursor.UITexture, true);

	cursor.ui.setScale(Screen::getSpriteScale(cursor.ui, normScale));
	sf::Vector2f origin = cursor.ui.getLocalBounds().size * normOrigin;
	cursor.ui.setOrigin(origin);

	sf::Color c = cursor.ui.getColor();
	c.a = (uint8_t)(255 * opacity);
	cursor.ui.setColor(c);
}

