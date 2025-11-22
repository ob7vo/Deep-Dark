#pragma once
#include "Menu.h"
#include "Animation.h"
#include "UnitData.h"

/*
	Workshop will be a status screen dedicated to ONE unit that is selected 
	from the Armory Equip Menu. The Unit will be on Display for the play to 
	test its animations, and its stats, augments, and cores will be displayed
	below (each seperated by their won dropdown/folder
*/
const int CONFIG_FOLDERS = 3; // The Dropdowns I mentioned.
const int ANIMATION_BTNS = 7; // To play the 5 main animations, the pause BTN, and an extra for a special animation
const int WORKSHOP_BTNS = ANIMATION_BTNS + 3;

const int STAT_ICONS = 8;
const std::array<float, 5> UNIT_ANIMATION_SPEEDS = { 0.5f, 0.75f, 1, 1.5f, 2.f };

struct WorkshopMenu : public Menu<WORKSHOP_BTNS> {
	UnitStats unitStats;
	int unitId = 0;
	int unitGear = 0;
	int numberOfForms = 0;

	sf::Text unitNameText = sf::Text(baseFont);
	sf::Text unitDescText = sf::Text(baseFont);
	sf::Sprite unitSprite = sf::Sprite(defTex);
	std::vector<sf::RectangleShape> unitHitboxes;
	UnitAniMap unitAnimations; // unordered map of Animations for the Unit
	UnitAnimationState currentAnimation = UnitAnimationState::MOVE;
	int unitAnimSpeedIndex = 2;

	std::array<sf::Sprite, STAT_ICONS> statIcons;
	std::array<sf::Text, STAT_ICONS> statTexts;

	explicit WorkshopMenu(Camera& cam);
	~WorkshopMenu() = default;
	void setup_workshop_unit(int id, int form);
	void set_stat_texts(const nlohmann::json& unitJson);

	void draw() override;
	void check_mouse_hover() override;
	bool on_mouse_press(bool isM1) override;
	bool on_mouse_release(bool isM1) override;
	void reset_positions() override;

	void update(float deltaTime) override;
	void update_unit_animation(float dt);

	void draw_unit_hitboxs();
	void draw_unit_hurtbox();
	void create_hitbox_visualizer();

	void switch_unit_gear();

	inline Button& return_btn() { return get_button(0); }
	inline Button& pause_btn() { return get_button(1); }
	inline Button& switchGearBtn() { return get_button(2); }
	inline Button& animationSpeedBtn() { return get_button(3); }
	// Indexes 2 - 7
	inline Button& animation_btn(UnitAnimationState ani) {
		auto i = static_cast<int>(ani);
		if (i < 0 || i > 4) i = 5; // if its a special animation
		return animation_btn(i);
	}
	/// <summary> Index will be incremented by 2 for adjustment </summary>
	inline Button& animation_btn(int i) { return get_button(i + 4); }

	///<summary>Type in the name of the stat and it will return the index</summary>
	inline int stat_index(const std::string& str) {
		switch (str[0]) {
		case 'a': return 0; // attack_time
		case 'c': return 1; // cost
		case 'h': return 2 + str[1] != 'i'; // hits(2) / hp(3)
		case 'k': return 4; // knockbacks
		case 'r': return 5; // recharge_time
		case 's': return 6 + str[1] != 'i'; // sight_range(7) / speed(8)
		default: 
			std::cout << "could not find stat index: " << str[0] << std::endl;
			return 0;
		}

		return 0;
	}
	inline sf::Sprite& stat_icon(const std::string& str) {return statIcons[stat_index(str)];}
	inline sf::Text& stat_text(const std::string& str) { return statTexts[stat_index(str)]; }

	static std::array<sf::Sprite, 8> make_statIcons() {
		return[]<std::size_t... Is>(std::index_sequence<Is...>) {
			return std::array{ sf::Sprite(TextureManager::t_statusIcons[Is])... };
		}(std::make_index_sequence<8>{});
	}
	static std::array<sf::Text, 8> make_statTexts() {
		std::array<sf::Text, 8> icons = {
		sf::Text(baseFont),sf::Text(baseFont),sf::Text(baseFont),
		sf::Text(baseFont),sf::Text(baseFont),sf::Text(baseFont),
		sf::Text(baseFont),sf::Text(baseFont)
		};

		return icons;
	}
};