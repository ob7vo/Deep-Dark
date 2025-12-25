#include "pch.h"
#include "Animation.h"
#include "UnitData.h"
#include "Utils.h"

using json = nlohmann::json;
const float KNOCKBACK_DURATION = 1.4f;
const float FALL_DURATION = 1.25f;
const float JUMP_DURATION = 1.f;

AnimationFrame::AnimationFrame(sf::IntRect rect, float duration, AnimationEvent events) :
    eventsMask(events){
    this->rect = rect;
    this->duration = duration;
}

AnimationClip::AnimationClip(sf::Texture* pTexture, int frameCount, float rate,
    sf::Vector2i cellSize, sf::Vector2f og, 
    const AnimationEventsList& events, bool loops)
    :  origin(og), 
    loops(loops)
{
    texture = pTexture;
    sf::Vector2u texSize = texture->getSize();
    auto rects = Textures::createTextureRects(frameCount, texSize, cellSize);

    for (int i = 0; i < frameCount; i++) frames.emplace_back(rects[i], rate);

    // setting the events
    for (const auto& [frame, eventMask] : events) frames[frame].eventsMask = eventMask;
    frames[0].eventsMask |= AnimationEvent::FIRST_FRAME;
    frames[frameCount - 1].eventsMask |= AnimationEvent::FINAL_FRAME;
};
AnimationEvent AnimationPlayer::update(float deltaTime, sf::Sprite& sprite) {
    time += deltaTime;

    if (time > clip->frames[currentFrame].duration) {
        time = 0;

        if (clip->loops)
            currentFrame = (currentFrame + 1) % clip->frames.size();
        else if (currentFrame < clip->frames.size() - 1)
            currentFrame++;

        sprite.setTextureRect(clip->frames[currentFrame].rect);
        return clip->frames[currentFrame].eventsMask;
    }

    return AnimationEvent::EMPTY_EVENT;
}
void AnimationPlayer::start(const AnimationClip* newClip, sf::Sprite& sprite) {
    clip = newClip;
    reset(sprite);
}
void AnimationPlayer::reset(sf::Sprite& sprite) {
    currentFrame = 0;
    time = 0;

    sprite.setTexture(*clip->texture);
    sprite.setTextureRect(clip->frames[0].rect);
    sprite.setOrigin(clip->origin);
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
std::pair<UnitAnimationState, bool> get_unit_ani_state(std::string_view str) {
    switch (str[0]) {
    case 'm': return { UnitAnimationState::MOVE, true };
    case 'a': return { UnitAnimationState::ATTACK, false };
    case 'i': return { UnitAnimationState::IDLE, true };
    case 'k': return { UnitAnimationState::KNOCKBACK, false };
    case 'f': return { UnitAnimationState::FALLING, false };
    case 'j': return { UnitAnimationState::JUMPING, false };
    case 'p': return { UnitAnimationState::PHASE_WINDUP, false };
    case 'd': return { UnitAnimationState::DEATH, false };
    default:
        throw std::runtime_error("could not get pair[AniState, bool(loops)] with string: ");
        return { UnitAnimationState::WAITING, true };
    }
}

void AnimationClip::set_duration(float duration) {
    float rate = duration / (int)frames.size();

    for (auto& frame : frames) frame.duration = rate;
}

AnimationClip AnimationClip::create_unit_animation(const json& file, sf::Texture* pTexture, 
    UnitAnimationState ani, bool loops) {
    AnimationClip clip = from_json(file, pTexture, loops);

    if (ani == UnitAnimationState::KNOCKBACK) clip.set_duration(KNOCKBACK_DURATION);
    else if (ani == UnitAnimationState::FALLING) clip.set_duration(FALL_DURATION);
    else if (ani == UnitAnimationState::JUMPING) clip.set_duration(JUMP_DURATION);

    return clip;
}
AnimationClip AnimationClip::from_json(const nlohmann::json& file, sf::Texture* pTexture, bool loops) {
    int frames = file["frames"];

    // This is more lines, thohg it is more readable;
    float rate = 0.1f;
    if (file.contains("fps")) rate = 1.f / file["fps"];
    else if (file.contains("duration")) rate = file["duration"] / frames;

    sf::Vector2i cellSizes = { file["cell_size"][0], file["cell_size"][1] };
    sf::Vector2f origin = { file["origin"][0], file["origin"][1] };

    AnimationEventsList events;
    if (file.contains("events")) {
        for (const auto& [animEventStr, frame] : file["events"].get<std::vector<std::pair<std::string, int>>>())
            events.emplace_back(frame, AnimationEvents::fromString(animEventStr));
    }
    
    return AnimationClip(pTexture, frames, rate, cellSizes, origin, events, loops);
}
void AnimationClip::setup_unit_animation_map(const json& unitFile, UnitAniMap& aniMap,
    std::deque<sf::Texture>& unitTextures) {
    size_t anims = unitFile["animations"].size();

    for (size_t i = 0; i < anims; i++) unitTextures.emplace_back();

    int curAnim = 0;

    for (const auto& [animName, animData] : unitFile["animations"].items()) {
        auto [aniState, doesLoop] = get_unit_ani_state(animName);

        std::string path = UnitData::getUnitGearPath(unitFile["unit_id"], unitFile.value("gear", 1));
        std::string fullPath = path + animName + ".png";
        Textures::loadTexture(unitTextures[curAnim], fullPath);

        aniMap[aniState] = create_unit_animation(animData, 
            &unitTextures[curAnim],aniState, doesLoop);

        curAnim++;
    }
}
