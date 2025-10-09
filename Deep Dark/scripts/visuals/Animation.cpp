#include "Animation.h"
#include <iostream>
using json = nlohmann::json;
const float KNOCKBACK_DURATION = 1.4f;
const float FALL_DURATION = 1.25f;
const float JUMP_DURATION = 1.f;

AnimationFrame::AnimationFrame(sf::IntRect rect, float duration, int events) :
    eventsMask(events){
    this->rect = rect;
    this->duration = duration;
}
Animation::Animation(std::string spritePath, int frameCount, float rate, int textureSizes[2], 
    int cellSizes[2], std::vector<std::pair<int, AnimationEvent>> events, bool loops)
{
    this->loops = loops;
    time = 0.0f;
    currentFrame = 0;
    if (!texture.loadFromFile(spritePath)) {
        (void)texture.loadFromFile("sprites/defaultTexture.png");
        std::cerr << "Failed to load texture: " << spritePath << std::endl;
    }
	this->frameCount = frameCount;

	int textureWidth = textureSizes[0]; // 0 = texture width
	int textureHeight = textureSizes[1]; // 1 = texture height
    int cellWidth = cellSizes[0];     // Individual frame width
    int cellHeight = cellSizes[1];    // Individual frame height
    sf::Vector2i size(cellWidth, cellHeight);

    int columns = textureWidth / cellWidth;   // How many frames per row
    int rows = textureHeight / cellHeight;    // How many rows

    int frame = 0;
    frames.reserve(frameCount);  
    for (int row = 0; row < rows && frame < frameCount; row++) {
        for (int col = 0; col < columns && frame < frameCount; col++) {
            sf::IntRect rect({ col * cellWidth, row * cellHeight }, size);
            frames.emplace_back(rect, rate, 0);
            frame++; 
        }
    }

    for (int i = 0; i < events.size(); i++)
        frames[events[i].first].eventsMask |= events[i].second;
    frames[0].eventsMask |= AnimationEvent::FIRST_FRAME;
    frames[frameCount - 1].eventsMask |= AnimationEvent::FINAL_FRAME;
};
int Animation::update(float deltaTime, sf::Sprite& sprite) {
    time += deltaTime;

    if (time > frames[currentFrame].duration) {
        time = 0;

        if (loops) 
            currentFrame = (currentFrame + 1) % frameCount; 
        else if (currentFrame < frameCount - 1)
            currentFrame++;
        
        sprite.setTextureRect(frames[currentFrame].rect);
        return frames[currentFrame].eventsMask;
    }

    return 0;
}
int Animation::update(float& time, int& curFrame, float deltaTime, sf::Sprite& sprite) {
    time += deltaTime;

    if (time > frames[curFrame].duration) {
        time = 0;

        if (loops)
            curFrame = (curFrame + 1) % frameCount;
        else if (curFrame < frameCount - 1)
            curFrame++;

        sprite.setTextureRect(frames[curFrame].rect);
        return frames[curFrame].eventsMask;
    }

    return 0;
}
void Animation::reset(sf::Sprite& sprite) {
    currentFrame = 0;
    time = 0;
    sprite.setTexture(texture);
    sprite.setTextureRect(frames[0].rect);
}
void Animation::reset(float& time, int& curFrame, sf::Sprite& sprite) {
    curFrame = 0;
    time = 0;
    sprite.setTexture(texture);
    sprite.setTextureRect(frames[0].rect);
}
std::string eventToString(AnimationEvent event) {
    switch (event) {
    case AnimationEvent::FIRST_FRAME: return "FIRST_FRAME";
    case AnimationEvent::FINAL_FRAME: return "FINAL_FRAME";
    case AnimationEvent::ATTACK: return "UNIT_ATTACK";
        // Add other events here
    default: return "UNKNOWN_EVENT";
    }
}
bool Animation::check_for_event(AnimationEvent targetEvent, int events) {
    return events & targetEvent;
}
Animation Animation::create_unit_animation(const json& file, std::string ani, bool loops) {
    if (!file["animations"].contains(ani)) {
        std::cout << "unit json does not have a [" << ani << "] animation." << std::endl;
        ani = "move";
    }
    if (!file["animations"].contains(ani)) 
        throw std::runtime_error("Missing required animation: " + ani);

    std::string path = file["path"];
    std::string texture = file["animations"][ani]["texture"];
    std::string fullPath = path + texture;

    int frames = file["animations"][ani]["frames"];
   
    float rate = 1.f; // file["animations"][ani]["fps"];
    if (ani == "knockback") rate = KNOCKBACK_DURATION / frames;
    else if (ani == "falling") rate = FALL_DURATION / frames;
    else if (ani == "special") {
        if (file["animations"][ani]["type"] == "jump")
            rate = JUMP_DURATION / frames;
        else rate = 1.f / file["animations"][ani].value("fps", 1);
    }
    else rate = 1.f / file["animations"][ani].value("fps", 1);

    int texWidth = file["animations"][ani]["texture_size"][0];
    int texHeight = file["animations"][ani]["texture_size"][1];
    int cellWidth = file["animations"][ani]["cell_size"][0];
    int cellHeight = file["animations"][ani]["cell_size"][1];
    int texSizes[2] = { texWidth, texHeight };
    int cellSizes[2] = { cellWidth, cellHeight };


    std::vector <std::pair<int, AnimationEvent>> events;
    if (file["animations"][ani].contains("attack_frames")) {
        std::vector<int> attack_frames = file["animations"][ani]["attack_frames"];
        for (int i = 0; i < attack_frames.size(); i++)
            events.emplace_back(attack_frames[i], AnimationEvent::ATTACK);
    }

    return Animation(fullPath, frames, rate, texSizes, cellSizes, events, loops);
}
void Animation::create_unit_animation_array(const json& unitFile, std::array<Animation, 5>& aniMap) {
    aniMap[0] = Animation::create_unit_animation(unitFile, "move", true);
    aniMap[1] = Animation::create_unit_animation(unitFile, "attack", false);
    aniMap[2] = Animation::create_unit_animation(unitFile, "idle", true);
    aniMap[3] = Animation::create_unit_animation(unitFile, "knockback", false);
    aniMap[4] = Animation::create_unit_animation(unitFile, "falling", false);
}
