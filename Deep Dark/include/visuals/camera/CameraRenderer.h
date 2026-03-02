#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <vector>
#include <memory>
#include "UILayout.h"

class Camera;

struct CameraRenderer {
	Camera& cam;

	std::vector<sf::Drawable*> uiDrawQueue;
	std::vector<std::unique_ptr<sf::Drawable>> tempUIDrawQueue; // for UI created on the Stack
	std::vector<sf::Drawable*> worldDrawQueue;

	sf::VertexArray create_dark_overlay(float left, float top, float width, float height, sf::Color color = UI::Colors::BLACK_TRANSPARENT, float fill = 1.f) const;

	void draw();
	void draw_overlay(const sf::Sprite& sprite, sf::Color color = UI::Colors::BLACK_TRANSPARENT, float fill = 1.f);
	void darken_screen(float opacity = 0.5f);
	void draw_cursor_ui();

	/// <summary> Will cull Drawable if a Rect in passed in for its position + size </summary>
	inline void queue_world_draw(sf::Drawable* draw, sf::FloatRect rect = {}) {
		if (rect.size.x == 0 || within_camera(rect)) worldDrawQueue.emplace_back(draw);
	}
	inline void queue_ui_draw(sf::Drawable* draw) { uiDrawQueue.emplace_back(draw); }
	template<typename T>
	inline void queue_temp_ui_draw(const T& drawable) {
		static_assert(std::is_base_of<sf::Drawable, T>::value, "Must be drawable");
		tempUIDrawQueue.emplace_back(std::make_unique<T>(drawable));  // Copies T
	}
	void clear_queues();

	bool within_camera(sf::FloatRect rect) const;

};