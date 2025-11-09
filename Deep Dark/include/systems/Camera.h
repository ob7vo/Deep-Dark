#pragma once
#include "SFML\Graphics.hpp"
#include "Bounds.h"
#include <iostream>

const float MIN_ZOOM = 0.6f;
const float MAX_ZOOM = 1.4f;
const float FRICTION = 0.85f;
const float ZOOM_SPEED = 0.1f;
const int FIXED_FRAMERATE = 30;
const float FIXED_DELTA_TIME = 0.0333f;

const sf::Texture defTexture("sprites/defaults/defaultTexture.png");
const sf::Color blackTransperent(0, 0, 0, 128);
const sf::Color redTransperent(255, 0, 0, 128);

class Camera
{
	sf::RenderWindow& window;
	sf::VertexArray darkScreenOverlay = sf::VertexArray(sf::PrimitiveType::TriangleStrip, 4);
	sf::Vector2f mousePos = { 0.f, 0.f };
	sf::Vector2i mouseScreenPos = { 0,0 };

	Bounds limits;
	bool locked = false;
public:
	float zoomLevel = 1;

	float scrollSpeed = 4.f;
	sf::Vector2f pos = { 0.f,0.f };

	sf::View view;
	sf::View uiView;

	std::vector<sf::Drawable*> uiDrawQueue;
	std::vector<std::unique_ptr<sf::Drawable>> tempUIDrawQueue; // for UI created on the Stack
	std::vector<sf::Drawable*> cullingDrawQueue; // for world objects

	sf::Vector2f dragOrigin{ 0.f,0.f };
	sf::Vector2f velocity{ 0.f,0.f };
	bool dragging = false;

	Camera(sf::RenderWindow& window);
	void set_dark_overlay(sf::VertexArray& overlay, float left, float top, float width, float height, float percentage = 0.f);
	void update(float deltaTime);
	void draw_all_ui();

	void handle_events(sf::Event event);
	void on_mouse_press(sf::Event::MouseButtonPressed click);
	void on_mouse_released(sf::Event::MouseButtonReleased release);

	void zoom(sf::Event::MouseWheelScrolled scroll);
	void zoom(sf::Keyboard::Key key);
	void apply_velocity(float deltaTime);
	void move(sf::Keyboard::Key key);
	void click_and_drag();

	sf::Vector2f get_pos_within_bounds(sf::Vector2f targetPos);
	void update_pos(sf::Vector2f pos);

	inline void reset() {
		view = window.getDefaultView();
		uiView = view;
		pos = view.getCenter();
		dragging = false;
	}

	inline void queue_draw(sf::Drawable* draw, sf::FloatRect rect) {
		if (within_camera(rect)) cullingDrawQueue.emplace_back(draw);
	}
	inline void queue_ui_draw(sf::Drawable* draw) { uiDrawQueue.emplace_back(draw); }
	template<typename T>
	inline void queue_temp_ui_draw(const T& drawable) {
		static_assert(std::is_base_of<sf::Drawable, T>::value, "Must be drawable");
		tempUIDrawQueue.emplace_back(std::make_unique<T>(drawable));  // Copies T
	}
	inline void clear_queues() {
		uiDrawQueue.clear();
		tempUIDrawQueue.clear();
		cullingDrawQueue.clear();
	}

	inline void set_bounds(sf::FloatRect newBounds) { limits.set_bounds(newBounds); }
	inline Bounds get_bounds() { return limits; }
	inline bool within_camera(sf::FloatRect rect) {
		sf::Vector2f center = view.getCenter();
		sf::Vector2f size = view.getSize();

		return !(
			rect.position.x + rect.size.x < center.x - size.x * .5f ||// Too far left
			rect.position.x > center.x + size.x * .5f ||              // Too far right
			rect.position.y + rect.size.y < center.y - size.y * .5f ||// Too far up
			rect.position.y > center.y + size.y * .5f                 // Too far down
			);
	}
	inline void change_lock(bool lock) {
		if (lock) {
			locked = true;
			dragging = false;
			velocity = { 0.f, 0.f };
		}
		else
			locked = false;
	}
	inline void swap_camera_lock() { change_lock(!locked); }
	inline void set_mouse_pos(sf::Vector2f mPos, sf::Vector2i mS_Pos) { 
		mousePos = mPos; mouseScreenPos = mS_Pos;
	}
	inline bool has_velocity() {
		return std::abs(velocity.x) > 0.1f && std::abs(velocity.y) > 0.1f;
	}

	inline sf::Vector2f norm_to_pixels(sf::Vector2f relative) {
		float x = uiView.getSize().x * relative.x;
		float y = uiView.getSize().y * relative.y;
		return { x,y };
	}
	inline sf::Vector2f get_norm_sprite_scale(const sf::Sprite& sprite, sf::Vector2f normScale) {
		// Get the sprite's original pixel dimensions
		sf::FloatRect bounds = sprite.getLocalBounds();
		sf::Vector2f targetPixelSize = norm_to_pixels(normScale);

		float scaleX = targetPixelSize.x / bounds.size.x;
		float scaleY = targetPixelSize.y / bounds.size.y;

		return { scaleX, scaleY };
	}
	inline void set_sprite_params(sf::Vector2f normPos, sf::Vector2f scale,
		const std::string& path, sf::Texture& texture, sf::Sprite& sprite) {
		sf::Vector2f _pos = norm_to_pixels(normPos);
//		std::cout << "pos: (" << pos.x << ", " << pos.y << ")" << std::endl;
		if (!texture.loadFromFile(path)) std::cout << "wrong path for btn texture" << std::endl;
		sprite = sf::Sprite(texture);

		scale = get_norm_sprite_scale(sprite, scale);
		sprite.setScale(scale);
		sprite.setPosition(_pos);
		sprite.setOrigin(sprite.getLocalBounds().size * 0.5f);
	}
	inline void draw_grey_screen(float transperency) { queue_ui_draw(&darkScreenOverlay); }

	inline void close_window() { window.close(); }
	inline sf::RenderWindow& get_window() const { return window; }
	inline sf::Vector2f& getMouseWorldPos() { return mousePos; }
	inline sf::Vector2i& getMouseScreenPos() { return mouseScreenPos; }
};