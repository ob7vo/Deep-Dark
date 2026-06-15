#pragma once
#include "Menu.h"
#include "Animation.h"
#include "UnitStats.h"
#include <deque>
#include <SFML/Graphics/RectangleShape.hpp>

/*
	Workshop will be a status screen dedicated to ONE unit that is selected 
	from the Armory Equip Menu. The Unit will be on Display for the play to 
	test its animations, and its stats, augments, and cores will be displayed
	below (each seperated by their won dropdown/folder /\/\ \/ /\/ \/\/
*/

const int STAT_ICONS = 8;
const std::array<float, 5> UNIT_ANIMATION_SPEEDS = { 0.5f, 0.75f, 1, 1.5f, 2.f };

struct WorkshopMenu : public Menu<UI::Workshop::BTN_COUNT> {
	UnitStats unitStats;
	int unitId = 0;

	sf::Text unitNameText = sf::Text(baseFont);
	sf::Text unitDescText = sf::Text(baseFont);
	sf::Sprite unitSprite = sf::Sprite(defaultTexture);
	std::vector<sf::RectangleShape> unitHitboxes;

	AnimationPlayer unitAnimPlayer;
	std::deque<sf::Texture> unitAnimTextures;
	UnitAnimMap unitAnimMap; // unordered map of Animations for the Unit
	UnitAnimationState currentAnimState = UnitAnimationState::MOVE;
	int unitAnimSpeedIndex = 2;

	std::array<sf::Sprite, STAT_ICONS> statIcons;
	std::array<sf::Text, STAT_ICONS> statTexts;

	sf::Sprite scrapPartsIcon = sf::Sprite(defaultTexture);
	sf::Text scrapPartsText = sf::Text(baseFont);
	sf::Text upgradeCostText = sf::Text(baseFont);

	explicit WorkshopMenu(Camera& cam);
	~WorkshopMenu() final = default;

	/// <summary>
	/// Sets up the workshop overview for a Unit.
	/// </summary>
	/// <param name="id">The unit's ID</param>
	/// <param name="enemyMagnification">Scaling multiplier for enemy stats when showcased in stage preview (default: 1.0)</param>
	void setup_workshop_unit(int id, float enemyMagnification = 1.f);
	void set_stat_texts(const nlohmann::json& unitJson);
	/// <summary>
	/// Depending on the Unit, certain UI elements are unseen
	/// </summary>
	void set_team_specific_ui_visibility();

	void draw() final;

	void reset_positions() final;
	void set_unit_anim_btn_positions();

	void update(float deltaTime) override;
	void update_unit_animation(float dt);

	void draw_unit_hitboxs();
	void draw_unit_hurtbox();
	void create_hitbox_visualizer();

	void upgrade_unit();

	Button& return_btn();
	Button& pause_btn();
	Button& animationSpeedBtn();
	Button& upgradeUnitBtn();
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

	inline bool isViewingAnEnemy() const { return unitId >= 100; }
};