#include "Animation.h"
#include "UnitData.h"
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

Animation::Animation(std::string spritePath, int frameCount, float rate, 
    sf::Vector2i cellSize, sf::Vector2f og, ani_event_map events, bool loops)
{
    this->loops = loops;
    time = 0.0f;
    currentFrame = 0;
    origin = og;
    if (!texture.loadFromFile(spritePath)) {
        (void)texture.loadFromFile("sprites/defaults/defaultTexture.png");
        std::cerr << "Failed to load texture: " << spritePath << std::endl;
        return;
    }

    sf::Vector2u texSize = texture.getSize();
	this->frameCount = frameCount;

    int columns = texSize.x / cellSize.x;   // How many frames per row
    int rows = texSize.y / cellSize.y;    // How many rows

    int frame = 0;
    frames.reserve(frameCount);  
    for (int row = 0; row < rows && frame < frameCount; row++) {
        for (int col = 0; col < columns && frame < frameCount; col++) {
            sf::IntRect rect({ col * cellSize.x, row * cellSize.y }, cellSize);
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
    return update(time, currentFrame, deltaTime, sprite);
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
    reset(time, currentFrame, sprite);
}
void Animation::reset(float& time, int& curFrame, sf::Sprite& sprite) {
    curFrame = 0;
    time = 0;

    sprite.setTexture(texture);
    sprite.setTextureRect(frames[0].rect);
    sprite.setOrigin(origin);
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

Animation Animation::create_unit_animation(const json& file, std::string ani, std::string path, bool loops) {
    int frames = file["frames"];
   
    float rate = 1.f / file.value("fps", frames);
    if (ani == "knockback") rate = KNOCKBACK_DURATION / frames;
    else if (ani == "falling") rate = FALL_DURATION / frames;
    else if (ani == "jump") rate = JUMP_DURATION / frames;

    sf::Vector2i cellSizes = { file["cell_size"][0], file["cell_size"][1] };
    sf::Vector2f origin = { file["origin"][0], file["origin"][1] };

    ani_event_map events;
    if (file.contains("attack_frames")) {
        std::vector<int> attack_frames = file["attack_frames"];
        for (int i = 0; i < attack_frames.size(); i++)
            events.emplace_back(attack_frames[i], AnimationEvent::ATTACK);
    }

    return Animation(path, frames, rate, cellSizes, origin, events, loops);
}

std::pair<UnitAnimationState,bool> get_unit_ani_state(std::string str) {
    switch (str[0]) {
    case 'm': return { UnitAnimationState::MOVE, true };
    case 'a': return { UnitAnimationState::ATTACK, false };
    case 'i': return { UnitAnimationState::IDLE, true };
    case 'k': return { UnitAnimationState::KNOCKBACK, false };
    case 'f': return { UnitAnimationState::FALLING, false };
    case 'j': return { UnitAnimationState::JUMPING, false };
    case 'p': return { UnitAnimationState::PHASE, false };
    }

    std::cout << "could not get pair[AniState, bool] with string: " << str << std::endl;
    return { UnitAnimationState::WAITING, true };
}
void Animation::setup_unit_animation_map(const json& unitFile, UnitAniMap& aniMap) {
    for (const auto& [animName, animData] : unitFile["animations"].items()) {
        auto [aniState, loops] = get_unit_ani_state(animName);
        std::string path = UnitData::get_unit_folder_path(unitFile["unit_id"], unitFile.value("gear", 1));
        std::string fullPath = path + animName + ".png";

        aniMap[aniState] = create_unit_animation(animData, animName, fullPath, loops);
    }
}
