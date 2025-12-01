#include "pch.h"
#include "Animation.h"
#include "UnitData.h"
#include "Utils.h"

using json = nlohmann::json;
const float KNOCKBACK_DURATION = 1.4f;
const float FALL_DURATION = 1.25f;
const float JUMP_DURATION = 1.f;

AnimationFrame::AnimationFrame(sf::IntRect rect, float duration, int events) :
    eventsMask(events){
    this->rect = rect;
    this->duration = duration;
}

Animation::Animation(const std::string_view& spritePath, int frameCount, float rate, 
    sf::Vector2i cellSize, sf::Vector2f og, const std::vector<int>& events, bool loops)
    : loops(loops), origin(og), frameCount(frameCount)
{
    if (!texture.loadFromFile(spritePath)) {
        (void)texture.loadFromFile("sprites/defaults/defaultTexture.png");
        std::cerr << "Failed to load texture: " << spritePath << std::endl;
        return;
    }

    sf::Vector2u texSize = texture.getSize();

    auto rects = TextureManager::createTextureRects(frameCount, texSize, cellSize);

    for (int i = 0; i < frameCount; i++) frames.emplace_back(rects[i], rate, events[i]);

    frames[0].eventsMask |= AnimationEvent::FIRST_FRAME;
    frames[frameCount - 1].eventsMask |= AnimationEvent::FINAL_FRAME;
};
int Animation::update(float deltaTime, sf::Sprite& sprite) {
    return update(timeElapsed, currentFrame, deltaTime, sprite);
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
    reset(timeElapsed, currentFrame, sprite);
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

Animation Animation::create_unit_animation(const json& file, const std::string_view& ani, const std::string_view& path, bool loops) {
    int frames = file["frames"];
   
    float rate = 1.f / file.value("fps", (float)frames);
    if (ani == "knockback") rate = KNOCKBACK_DURATION / (float)frames;
    else if (ani == "falling") rate = FALL_DURATION / (float)frames;
    else if (ani == "jump") rate = JUMP_DURATION / (float)frames;

    sf::Vector2i cellSizes = { file["cell_size"][0], file["cell_size"][1] };
    sf::Vector2f origin = { file["origin"][0], file["origin"][1] };

    std::vector<int> events(frames);
    if (file.contains("attack_frames"))
        for (const int& frame : file["attack_frames"])
            events[frame] = AnimationEvent::ATTACK;

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
    default:
        std::cout << "could not get pair[AniState, bool] with string: " << str << std::endl;
        return { UnitAnimationState::MOVE, true };
    }

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
