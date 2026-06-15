#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "UILayout.h"
#include <random>
#include <format>

enum class Direction { Up, Down, Left, Right };

namespace Random {
    inline std::mt19937& generator() {
        static std::mt19937 gen(std::random_device{}());
        return gen;
    }

    inline int r_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator());
    }

    inline float r_float(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(generator());
    }

    inline bool chance(float probability) {
        return r_float(0.0f, 100.f) < probability;
    }
}
namespace Converting {
    template<typename T>
    inline float cast_f(T var) {
        return static_cast<float>(var);
    }
    template<typename T>
    inline int cast_i(T var) {
        return static_cast<int>(var);
    }
    template<typename T>
    inline sf::Vector2i cast_Vec2i(sf::Vector2<T> vec) {
        return sf::Vector2i(static_cast<int>(vec.x), static_cast<int>(vec.y));
    }
    template<typename T>
    inline sf::Vector2f cast_Vec2f(sf::Vector2<T> vec) {
        return sf::Vector2f(static_cast<float>(vec.x), static_cast<float>(vec.y));
    }
}
namespace Printing {
    // Wrap a string in square brakets []
    inline std::string wrap(std::string_view str) {
        return std::format("[{}]", str);
    }
    // Wrap a string in Parenthesis ()
    inline std::string wrapP(std::string_view str) {
        return std::format("({})", str);
    }
    template<typename T>
    inline std::string vec2(sf::Vector2<T> v2) {
        return std::format("({}, {})", v2.x, v2.y);
    }
    inline std::string word_wrap(const std::string& text, sf::Font& font, unsigned charSize, float maxWidth) {
        std::string result, line, word;
        std::istringstream stream(text);

        while (stream >> word) {
            std::string test = line.empty() ? word : line + " " + word;
            sf::Text text(font);
            text.setString(test);
            text.setCharacterSize(charSize);

            if (text.getLocalBounds().size.x > maxWidth && !line.empty()) {
                result += line + "\n";
                line = word;
            }
            else {
                line = test;
            }
        }
        return result + line;
    }
    inline void center_text(sf::Text& text) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({ bounds.position.x + (bounds.size.x * 0.5f), bounds.position.y + (bounds.size.y * 0.5f) });
    }
}
namespace FolderPaths {
    inline std::string path(std::string_view folder, std::string_view file) {
        return std::format("{}{}", folder, file);
    }

    inline constexpr std::string_view effectsPath = "sprites/effects/";
    inline constexpr std::string_view workshopPath = "sprites/ui/workshop/";
    inline constexpr std::string_view armoryPath = "sprites/ui/armory_menu/";
    inline constexpr std::string_view stageSelectPath = "sprites/ui/stage_select/";
    inline constexpr std::string_view stageNodeMenuPath = "sprites/ui/stage_select/stage_node_menu/";
    inline constexpr std::string_view stageUIPath = "sprites/ui/stage_ui/";
    inline constexpr std::string_view startMenuPath = "sprites/ui/start_menu/";
    inline constexpr std::string_view iconsPath = "sprites/icons/";
    inline constexpr std::string_view defaultsPath = "sprites/defaults/";
    inline constexpr std::string_view uiPath = "sprites/ui/";
    inline constexpr std::string_view entitySpritesPath = "sprites/entities/";
    inline constexpr std::string_view baseSpritesPath = "sprites/entities/bases/";
    inline constexpr std::string_view surgeSpritesPath = "sprites/entities/surges/";
    inline constexpr std::string_view trapSpritesPath = "sprites/entities/traps/";


}
namespace Collision {
    inline bool AABB(sf::Vector2f posA, sf::Vector2f boxA, sf::Vector2f posB, sf::Vector2f boxB) {
        return (posA.x < posB.x + boxB.x && posA.x + boxA.x > posB.x &&
            posA.y < posB.y + boxB.y && posA.y + boxA.y > posB.y);
    }
    inline bool AABB(sf::FloatRect a, sf::FloatRect b) {
        return (a.position.x < b.position.x + b.size.x &&
            a.position.x + a.size.x > b.position.x &&
            a.position.y < b.position.y + b.size.y &&
            a.position.y + a.size.y > b.position.y);
    }
    inline bool range(float posA, float posB, float minRange, float maxRange) {
        float dist = std::abs(posA - posB);
        return dist >= minRange && dist <= maxRange;
    }
}
namespace Math {
    inline sf::Vector2f v_Lerp(sf::Vector2f start, sf::Vector2f end, float t) {
        return start + (end - start) * t;
    }
}
namespace Screen {
    inline sf::Vector2u size;

    inline sf::Vector2f toPixels(sf::Vector2f norm) {
        return { norm.x * size.x, norm.y * size.y };
    }
    inline std::pair<float,float> toPixels(std::pair<float,float> norm) {
        return { norm.first * size.x, norm.second * size.y };
    }
    inline sf::Vector2f pixelsToNorm(sf::Vector2f pixels) {
        return { pixels.x / size.x, pixels.y / size.y };
    }

    inline sf::Vector2f lerpOffscreen(sf::Vector2f startNormPos, float offscreenNormDist, float t, Direction dir) {
        sf::Vector2f slide = {};

        switch (dir) {
        case Direction::Up:	slide = { startNormPos.x, 0.f - offscreenNormDist }; break;
        case Direction::Down:  slide = { startNormPos.x, 1 + offscreenNormDist }; break;
        case Direction::Left:  slide = { 0 - offscreenNormDist, startNormPos.y }; break;
        case Direction::Right: slide = { 1 + offscreenNormDist,  startNormPos.y }; break;
        }

        sf::Vector2f normTargetPosition = Math::v_Lerp(startNormPos , slide, t);
        return toPixels(normTargetPosition);
    }

    inline sf::Vector2f getSpacing(sf::Vector2f uiSize, sf::Vector2f spacing) {
        // Assumes that the UI with uiSize is centered
        sf::Vector2f newSpacing = Screen::toPixels((uiSize * 0.5f) + spacing);

        if (spacing.x == 0.f) newSpacing.x = 0;
        else if (spacing.y == 0.f) newSpacing.y = 0;

        return newSpacing;
    }

    inline sf::Vector2f getSpriteScale(const sf::Sprite& sprite, sf::Vector2f normScale) {
        // Get the sprite's original pixel dimensions
        sf::FloatRect bounds = sprite.getLocalBounds();
        sf::Vector2f targetPixelSize = normScale * (float)std::min(size.x, size.y);

        float scaleX = targetPixelSize.x / bounds.size.x;
        float scaleY = targetPixelSize.y / bounds.size.y;

        return { scaleX, scaleY };
    }
    inline void setFontSize(sf::Text& text, float normHeight) {
        float targetPixelHeight = normHeight * size.y;

        if (text.getString() == "") text.setString("Ag");

        // Start with an estimate
        auto fontSize = static_cast<unsigned int>(targetPixelHeight);
        text.setCharacterSize(fontSize);

        // Measure actual height
        float actualHeight = text.getLocalBounds().size.y;

        // Adjust if needed
        if (actualHeight > 0.f) {
            fontSize = static_cast<unsigned int>(fontSize * (targetPixelHeight / actualHeight));
            text.setCharacterSize(fontSize);
        }

        // set the final size
        text.setCharacterSize(fontSize);
    }

    template<typename T>
    //With respect to the screen's coordinate system, where y increases downwards, this checks if a is above b
    inline bool above(sf::Vector2<T> a, sf::Vector2<T> b) {
        return a.y < b.y;
    };
    template<typename T>
    //With respect to the screen's coordinate system, where y increases downwards, this checks if a is below b
    inline bool below(sf::Vector2<T> a, sf::Vector2<T> b) {
        return a.y > b.y;
    };
    //With respect to the screen's coordinate system, where y increases downwards, this checks if a is above b
    inline bool above(float a, float b) {
        return a < b;
    };
    //With respect to the screen's coordinate system, where y increases downwards, this checks if a is below b
    inline bool below(float a, float b) {
        return a > b;
    };
}
namespace Visual {
    inline void centerText(sf::Text& text) {
        text.setOrigin(text.getLocalBounds().position + text.getGlobalBounds().size * 0.5f);
    }
    inline void setupUI(sf::Vector2f uiPos, sf::Vector2f normScale, sf::Sprite& sprite,
        const sf::Texture& texture, sf::IntRect textureRect = {})
    {
        if (textureRect.size.x == 0 || textureRect.size.y == 0) textureRect.size = (sf::Vector2i)texture.getSize();
        bool worldSpace = std::abs(uiPos.x) > 1.f && std::abs(uiPos.y) > 1.f;

        sprite.setTexture(texture);
        sprite.setTextureRect(textureRect);

        sprite.setScale(Screen::getSpriteScale(sprite, normScale));
        sprite.setPosition((worldSpace ? uiPos : Screen::toPixels(uiPos)));
        sprite.setOrigin(sprite.getLocalBounds().size * 0.5f);
    }
    inline void setupText(sf::Text& text, sf::Vector2f normPos, float height, bool center = false) {
        text.setPosition(Screen::toPixels(normPos));
        Screen::setFontSize(text, height);

        if (center) centerText(text);
    }
}
namespace Arrays {
    template<int SIZE>
    inline std::array<sf::Text, SIZE> createTextArr() {
        std::array<sf::Text, SIZE> arr;
        for (auto& text : arr)
            text.setFont(baseFont);
        return arr;

        /*
        return[&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::array<sf::Text, SIZE>{ (void(I), sf::Text(baseFont))... };
        }(std::make_index_sequence<SIZE>{});
        */
    }
    template<typename T>
    inline void swap_n_pop(std::vector<T>& vec, size_t index) {
        if (index >= vec.size()) return;

        vec[index] = vec.back();
        vec.pop_back();
    }
}
namespace Vector2 {
	template<typename T>
	inline sf::Vector2<T> rotate(sf::Vector2<T> vec, float angleDegrees) {
		float radians = angleDegrees * 3.14159265f / 180.f;
		float cosA = std::cos(radians);
		float sinA = std::sin(radians);
		return sf::Vector2<T>(
			vec.x * cosA - vec.y * sinA,
			vec.x * sinA + vec.y * cosA
		);
	}
}