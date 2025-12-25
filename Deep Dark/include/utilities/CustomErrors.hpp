#pragma once
#include <stdexcept>
#include <Utils.h>

class InvalidGearError : public std::runtime_error {
public:
    InvalidGearError(int gear, int id)
        : std::runtime_error(std::format("Gear must be between 1 and 3. Got gear #{} for unit #{}", gear, id)) {}
};

class InvalidCellSizeError : public std::runtime_error {
public:
    InvalidCellSizeError(sf::Vector2i cellSize, sf::Vector2u texSize)
        : std::runtime_error("Cell Size " + Printing::vec2(cellSize) +
            " did not line up with the Texture " + Printing::vec2(texSize)) {}
    InvalidCellSizeError()
        : std::runtime_error("Invalid cell size: One or mreo Component is less than 0") {}
    InvalidCellSizeError(int frames, int cells)
        : std::runtime_error(std::format("Warning: frameCount [{}] exceeds available cells [{}]", frames, cells)) {}
};
