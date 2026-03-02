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


sf::Vector2f Camera::norm_to_pixels(sf::Vector2f norm) const {
	return  uiView.getSize() * norm;
}
sf::Vector2f Camera::norm_to_pixels_size(sf::Vector2f norm) const {
	return norm * std::min(uiView.getSize().x, uiView.getSize().y); // both use same dimension!
}
std::pair<float, float> Camera::norm_to_pixels_pair(sf::Vector2f norm) const {
	sf::Vector2f normalized = norm_to_pixels(norm);
	return { normalized.x, normalized.y };
}


sf::Vector2f Camera::get_norm_sprite_scale(const sf::Sprite& sprite, sf::Vector2f normScale) const {
	// Get the sprite's original pixel dimensions
	sf::FloatRect bounds = sprite.getLocalBounds();
	sf::Vector2f targetPixelSize = norm_to_pixels_size(normScale);

	float scaleX = targetPixelSize.x / bounds.size.x;
	float scaleY = targetPixelSize.y / bounds.size.y;

	return { scaleX, scaleY };
}
unsigned int Camera::get_norm_font_size(sf::Text& text, float normHeight) const {
	float targetPixelHeight = normHeight * uiView.getSize().y;

	// Start with an estimate
	auto fontSize = static_cast<unsigned int>(targetPixelHeight);
	text.setCharacterSize(fontSize);

	// Measure actual height
	float actualHeight = text.getLocalBounds().size.y;

	// Adjust if needed
	if (actualHeight > 0.f) {
		fontSize *= static_cast<unsigned int>(targetPixelHeight / actualHeight);
		text.setCharacterSize(fontSize);
	}

	return fontSize;
}
bool Camera::setup_sprite(sf::Vector2f uiPos, sf::Vector2f normScale, sf::Sprite& sprite, 
	const sf::Texture& texture, sf::IntRect textureRect) const {
	if (textureRect.size.x == 0 || textureRect.size.y == 0) textureRect.size = (sf::Vector2i)texture.getSize();
	bool isUI = std::abs(uiPos.x) <= 1.f && std::abs(uiPos.y) <= 1.f;

	sprite.setTexture(texture);
	sprite.setTextureRect(textureRect);

	sprite.setScale(get_norm_sprite_scale(sprite, normScale));
	sprite.setPosition((isUI ? norm_to_pixels(uiPos) : uiPos));
	sprite.setOrigin(sprite.getLocalBounds().size * 0.5f);

	return isUI;
}

void Camera::set_cursor_ui(const sf::Texture& uiTex, sf::Vector2f normOrigin, float opacity, sf::Vector2f normScale) {
	cursor.UITexture = uiTex;
	cursor.ui.setTexture(cursor.UITexture, true);

	cursor.ui.setScale(get_norm_sprite_scale(cursor.ui, normScale));
	sf::Vector2f origin = cursor.ui.getLocalBounds().size * normOrigin;
	cursor.ui.setOrigin(origin);

	sf::Color c = cursor.ui.getColor();
	c.a = (uint8_t)(255 * opacity);
	cursor.ui.setColor(c);
}

