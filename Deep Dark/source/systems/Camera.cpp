#include "Camera.h"
#include <math.h>
#include <iostream>

using Key = sf::Keyboard::Key;
const auto MOUSE_1 = sf::Mouse::Button::Left;
const auto MOUSE_2 = sf::Mouse::Button::Right;
const float PAN_MULTIPLIER = 3.f;
const float SCROLL_SPEED = 5.f;

Camera::Camera(sf::RenderWindow& window) : window(window) {
	worldView = uiView = window.getDefaultView();

	pos = worldView.getCenter();
	window.setView(worldView);

	sf::Vector2f boundsSize = worldView.getSize() * 2.f;
	sf::Vector2f topLeft = { -100,-100 };
	limits.set_bounds({ topLeft, boundsSize });
}

sf::VertexArray Camera::create_dark_overlay(float left, float top, float width, float height, float percentage) const {
	auto darkOverlay = sf::VertexArray(sf::PrimitiveType::TriangleStrip, 4);

	float darkHeight = height * (1.0f - percentage);
	
	darkOverlay[0].position = { left, top + height - darkHeight };
	darkOverlay[0].color = blackTransperent;

	darkOverlay[1].position = { left + width, top + height - darkHeight };
	darkOverlay[1].color = blackTransperent;

	darkOverlay[2].position = { left, top + height };
	darkOverlay[2].color = blackTransperent;

	darkOverlay[3].position = { left + width, top + height };
	darkOverlay[3].color = blackTransperent;

	return darkOverlay;
}
void Camera::update_projection() {
	float aspect = (float)window.getSize().x / (float)window.getSize().y;
	float worldWidth = worldHeight * aspect;
	
	worldView.setSize({ worldWidth, worldHeight });

	window.setView(worldView);
}

void Camera::update(float deltaTime) {
	draw_all_ui();

	if (cursor.dragging) click_and_drag();
	else if (cursor.has_velocity())
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

	window.setView(worldView);

	for (auto& draw : worldDrawQueue)
		if (draw) window.draw(*draw);
	worldDrawQueue.clear();
}
void Camera::draw_grey_screen(float opacity) {
	sf::VertexArray darkOverlay = create_dark_overlay(0, 0, worldView.getSize().x, worldView.getSize().y);
	for (int i = 0; i < 4; i++) darkOverlay[i].color.a = (uint8_t)(255 * opacity);
	queue_ui_draw(&darkOverlay);
}

void Camera::handle_events(sf::Event event) {
	if (locked) return;

	if (auto click = event.getIf<sf::Event::MouseButtonPressed>())
		on_mouse_press(*click);
	else if (auto release = event.getIf<sf::Event::MouseButtonReleased>())
		on_mouse_released(*release);
	else if (auto scroll = event.getIf<sf::Event::MouseWheelScrolled>()) 
		zoom(*scroll);
	else if (auto key = event.getIf<sf::Event::KeyPressed>()) {
		move(key->code);
		zoom(key->code);
	}
}
void Camera::on_mouse_press(sf::Event::MouseButtonPressed click) {
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

// Zoom
void Camera::zoom(Key key) {
	float oldZoom = zoomLevel;
	if (key == Key::O)
		zoomLevel = std::min(zoomLevel + ZOOM_SPEED, MAX_ZOOM);
	else if (key == Key::I)
		zoomLevel = std::max(zoomLevel - ZOOM_SPEED, MIN_ZOOM);
	if (oldZoom == zoomLevel) return;

	sf::Vector2f newSize = (sf::Vector2f)window.getSize() * zoomLevel;
	worldView.setSize(newSize);
	update_pos(pos);
}
void Camera::zoom(sf::Event::MouseWheelScrolled scroll) {
	float oldZoom = zoomLevel;
	zoomLevel -= scroll.delta * FIXED_DELTA_TIME;
	zoomLevel = std::clamp(zoomLevel, MIN_ZOOM, MAX_ZOOM);
	float zoomFactor = zoomLevel / oldZoom;

	sf::Vector2f offset = cursor.worldPos - pos;
	sf::Vector2f newOffset = offset * zoomFactor;
	sf::Vector2f newPos = cursor.worldPos - newOffset;

	sf::Vector2f newSize = (sf::Vector2f)window.getSize() * zoomLevel;

	worldView.setSize(newSize);
	update_pos(newPos);
}

// Moving
void Camera::update_pos(sf::Vector2f newPos) {
	pos = get_pos_within_bounds(newPos);
	worldView.setCenter(pos);
	window.setView(worldView);
}
sf::Vector2f Camera::get_pos_within_bounds(sf::Vector2f targetPos) const {
	sf::Vector2f halfSize = worldView.getSize() * 0.5f;

	//bounds.he
	float x = std::clamp(targetPos.x, (limits.left + halfSize.x), (limits.right - halfSize.x));
	float y = std::clamp(targetPos.y, limits.top + halfSize.y, limits.bottom - halfSize.y);

	return sf::Vector2f(x, y);
}

void Camera::apply_velocity(float deltaTime) {
	sf::Vector2f newPos = pos + (cursor.velocity * deltaTime);
	update_pos(newPos);

 	cursor.velocity *= std::pow(FRICTION, deltaTime * FIXED_FRAMERATE);
}
void Camera::move(Key key) {
	if ((int)key < 71 || (int)key > 74 || cursor.dragging) return;
	cursor.velocity = { 0.f,0.f };
	sf::Vector2f newPos = pos;

	if (key == Key::Up) newPos.y -= SCROLL_SPEED;
	else if (key == Key::Down) newPos.y += SCROLL_SPEED;
	if (key == Key::Right) newPos.x += SCROLL_SPEED;
	else if (key == Key::Left) newPos.x -= SCROLL_SPEED;

	update_pos(newPos);
}
void Camera::click_and_drag() {
	// main.cpp will call this while dragging == true
	sf::Vector2f diff = cursor.worldPos - pos;
	sf::Vector2f newPos = (cursor.dragOrigin - diff);

	sf::Vector2f instantVelocity = (newPos - pos) * (float)FIXED_FRAMERATE;
	cursor.velocity = (cursor.velocity * 0.8f) + (instantVelocity * 0.2f);

	update_pos(newPos);
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
void Camera::set_sprite_params(sf::Vector2f normPos, sf::Vector2f scale,
	const std::string& path, sf::Texture& texture, sf::Sprite& sprite) const {
	sf::Vector2f _pos = norm_to_pixels(normPos);

	if (!texture.loadFromFile(path)) std::cout << "wrong path for btn texture" << std::endl;
	sprite = sf::Sprite(texture);

	scale = get_norm_sprite_scale(sprite, scale);
	sprite.setScale(scale);
	sprite.setPosition(_pos);
	sprite.setOrigin(sprite.getLocalBounds().size * 0.5f);
}

void Camera::set_cursor_ui(sf::Texture& uiTex, sf::Vector2f normOrigin, float opacity, sf::Vector2f normScale) {
	cursor.UITexture = uiTex;
	cursor.ui.setTexture(cursor.UITexture, true);

	cursor.ui.setScale(get_norm_sprite_scale(cursor.ui, normScale));
	sf::Vector2f origin = cursor.ui.getLocalBounds().size * normOrigin;
	cursor.ui.setOrigin(origin);

	sf::Color c = cursor.ui.getColor();
	c.a = (uint8_t)(255 * opacity);
	cursor.ui.setColor(c);
}