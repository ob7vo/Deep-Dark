#pragma once
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <functional>

/*
struct ITextboxSubmitHandler {
	virtual void on_text_submit(sf::Text* t) = 0;
	virtual ~ITextboxSubmitHandler() = default;
};

class TextboxManager {
public:
	sf::Text* curTextBox = nullptr;
	std::function<void(sf::Text*)> onTextSubmit = nullptr;
	int textBoxIndex = 0;

	TextboxManager();
	void create_text_box_callbacks(StateManager& stateManager);

	void type_in_text_box(sf::Event event);
	void select_text_box(sf::Text* textBox, std::function<void(sf::Text*)> onTextSubmit);
};
*/