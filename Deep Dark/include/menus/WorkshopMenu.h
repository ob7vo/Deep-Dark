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

struct WorkshopMenu : public Menu<UI::Workshop::BTN_COUNT> {
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
	~WorkshopMenu() final = default;

	void setup_workshop_unit(int id, int gear);
	void set_stat_texts(const nlohmann::json& unitJson);

	void draw() final;
	void check_mouse_hover() final;
	bool on_mouse_press(bool isM1) final;
	bool on_mouse_release(bool isM1) final;
	void reset_positions() final;

	void update(float deltaTime) override;
	void update_unit_animation(float dt);

	void draw_unit_hitboxs();
	void draw_unit_hurtbox();
	void create_hitbox_visualizer();

	void switch_unit_gear();

	Button& return_btn();
	Button& pause_btn();
	Button& switchGearBtn();
	Button& animationSpeedBtn();
	// Indexes 2 - 7
	Button& animation_btn(UnitAnimationState ani);
	/// <summary> Index will be incremented by 2 for adjustment </summary>
	Button& animation_btn(int i);

	///<summary>Type in the name of the stat and it will return the index</summary>
	int stat_index(const std::string& str) const;
	inline sf::Sprite& stat_icon(const std::string& str) {return statIcons[stat_index(str)];}
	inline sf::Text& stat_text(const std::string& str) { return statTexts[stat_index(str)]; }

	static std::array<sf::Sprite, 8> make_statIcons();
	static std::array<sf::Text, 8> make_statTexts();
};