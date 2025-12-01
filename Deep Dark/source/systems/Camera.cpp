#include "pch.h"
#include "Camera.h"
#include "UILayout.h"

using namespace UI::Colors;
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

void Camera::reset() {
	worldView = window.getDefaultView();
	uiView = worldView;
	pos = worldView.getCenter();
	cursor.dragging = false;
}
sf::VertexArray Camera::create_dark_overlay(float left, float top, float width, float height, sf::Color color, float fill) const {
	auto darkOverlay = sf::VertexArray(sf::PrimitiveType::TriangleStrip, 4);

	float darkHeight = height * fill;

	darkOverlay[0].position = { left, top + height - darkHeight };
	darkOverlay[0].color = BLACK_TRANSPARENT;

	darkOverlay[1].position = { left + width, top + height - darkHeight };
	darkOverlay[1].color = BLACK_TRANSPARENT;

	darkOverlay[2].position = { left, top + height };
	darkOverlay[2].color = BLACK_TRANSPARENT;

	darkOverlay[3].position = { left + width, top + height };
	darkOverlay[3].color = BLACK_TRANSPARENT;

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

	for (const auto& u_draw : tempUIDrawQueue)
		if (u_draw) window.draw(*u_draw);
	tempUIDrawQueue.clear();

	window.setView(worldView);

	for (auto& draw : worldDrawQueue)
		if (draw) window.draw(*draw);
	worldDrawQueue.clear();
}
void Camera::clear_queues() {
	uiDrawQueue.clear();
	tempUIDrawQueue.clear();
	worldDrawQueue.clear();
}

void Camera::darken_screen(float opacity) {
	sf::VertexArray darkOverlay = create_dark_overlay(0, 0, uiView.getSize().x, uiView.getSize().y);
	for (int i = 0; i < 4; i++) darkOverlay[i].color.a = (uint8_t)(255 * opacity);
	queue_temp_ui_draw(darkOverlay);
}
void Camera::draw_overlay(const sf::Sprite& sprite, sf::Color color, float fill) {
	sf::FloatRect bounds = sprite.getGlobalBounds();
	float left = bounds.position.x;
	float top = bounds.position.y;
	float height = bounds.size.y;
	float width = bounds.size.x;

	const sf::VertexArray overlay = create_dark_overlay(left, top, width, height, color, fill);
	queue_temp_ui_draw(overlay);
}


void Camera::change_lock(bool lock) {
	if (lock) {
		locked = true;
		cursor.dragging = false;
		cursor.velocity = { 0.f, 0.f };
	}
	else
		locked = false;
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
#pragma Events
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
#pragma endregion

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

sf::Vector2f Camera::norm_to_pixels(sf::Vector2f norm) const {
	return  uiView.getSize() * norm;
}
sf::Vector2f Camera::norm_to_pixels_size(sf::Vector2f norm) const {
	return norm * std::min(uiView.getSize().x, uiView.getSize().y); // both use same dimension!
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
	const sf::Texture& texture, sf::IntRect rect) const {
	if (rect.size.x == 0 || rect.size.y == 0) rect.size = (sf::Vector2i)texture.getSize();
	bool isUI = std::abs(uiPos.x) <= 1.f || std::abs(uiPos.y) <= 1.f;

	sprite.setTexture(texture);
	sprite.setTextureRect(rect);

	sprite.setScale(get_norm_sprite_scale(sprite, normScale));
	sprite.setPosition((isUI ? norm_to_pixels(uiPos) : uiPos));
	sprite.setOrigin(sprite.getLocalBounds().size * 0.5f);

	return isUI;
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

bool Camera::within_camera(sf::FloatRect rect) const {
	sf::Vector2f center = worldView.getCenter();
	sf::Vector2f size = worldView.getSize();

	return !(
		rect.position.x + rect.size.x < center.x - size.x * .5f ||// Too far left
		rect.position.x > center.x + size.x * .5f ||              // Too far right
		rect.position.y + rect.size.y < center.y - size.y * .5f ||// Too far up
		rect.position.y > center.y + size.y * .5f                 // Too far down
		);
}

