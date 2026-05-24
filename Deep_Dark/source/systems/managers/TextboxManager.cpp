#include "pch.h"
#include "TextboxManager.h"
#include "StateManager.h"

using Key = sf::Keyboard::Key;
/*
TextboxManager::TextboxManager() {

}
void TextboxManager::create_text_box_callbacks(StateManager& stateManager) {
    DebugState* debug = dynamic_cast<DebugState*>(stateManager.gameStateMap[3].get());

    debug->unitCreatorMenu.textboxCallback = [debug](sf::Text* textBox) {
        select_text_box(textBox, [debug](sf::Text* t) {
            debug->unitCreatorMenu.on_text_submit(t);
            });
        };
}

void TextboxManager::type_in_text_box(sf::Event event) {
    // Will exit the text box if the players click outside of it
    if (auto click = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (!curTextBox->getGlobalBounds().contains(Converting::cast_Vec2f(click->position))) {
            if (onTextSubmit) onTextSubmit(curTextBox); // callback
            curTextBox = nullptr; // deselect
        }
    }
    else if (auto key = event.getIf<sf::Event::KeyPressed>()) {
        std::string textStr = curTextBox->getString();

        switch (key->code) {
        case Key::Backspace:
            if (!textStr.empty() && textBoxIndex > 0) {
                textStr.erase(textBoxIndex - 1, 1); // erase at cursor
                textBoxIndex--;
            }
            break;
        case Key::Left:
            textBoxIndex = std::max(textBoxIndex - 1, 0);
            break;
        case Key::Right:
            textBoxIndex = std::min(textBoxIndex + 1, (int)textStr.size());
            break;
        case Key::Enter:
            if (onTextSubmit) onTextSubmit(curTextBox);
            curTextBox = nullptr;
            break;
        default: {
            int codeNumber = static_cast<int>(key->code);
            char c = 0;

            if (codeNumber >= 0 && codeNumber <= 25)// A-Z (lowercase)
                c = 'a' + codeNumber;
            else if (codeNumber >= 26 && codeNumber <= 35) // 0-9 (adjust if your range is 25-35)
                c = '0' + (codeNumber - 26);

            if (c != 0) {
                textStr.insert(textBoxIndex, 1, c); // the 1 means insert c once
                textBoxIndex++;
            }
            break;
        }
        }

        curTextBox->setString(textStr);
    }
}
void TextboxManager::select_text_box(sf::Text* textBox, std::function<void(sf::Text*)> onTextSubmit) {
    curTextBox = textBox;
    // Sits on the index for easy inserting and erasing
    textBoxIndex = textBox->getString().getSize();

    this->onTextSubmit = onTextSubmit;
}
    */