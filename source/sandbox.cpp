#include "sandbox.hpp"

#include <RIP/C3D.h>
#include <citro2d.h>

#include <cmath>

#define MAX_TEXTURE_SIZE 1024

Sandbox::Sandbox(UVec2 screen_size, size_t scale)
    : screen_size(screen_size),
    scale(scale)
{
    logical_size = {
        screen_size.x / scale,
        screen_size.y / scale,
    };
    texture_size = {
        std::min((size_t) MAX_TEXTURE_SIZE, (size_t) std::pow(2, std::ceil(std::log2(logical_size.x)))),
        std::min((size_t) MAX_TEXTURE_SIZE, (size_t) std::pow(2, std::ceil(std::log2(logical_size.y)))),
    };

    texture_data = (u32*) linearAlloc(texture_size.x * texture_size.y * sizeof(u32));
    for (size_t i = 0; i < texture_size.x * texture_size.y; i++) {
        texture_data[i] = 0x000000FF;
    }

    C3D_TexInit(&texture, texture_size.x, texture_size.y, GPU_RGBA8);

    ripConvertAndLoadC3DTexImage(&texture, texture_data, (GPU_TEXFACE) 0, 0);

    subtexture = {
        (u16) texture_size.x,
        (u16) texture_size.y,
        0.0f, 1.0f, 1.0f, 0.0f
    };

    image.tex = &texture;
    image.subtex = &subtexture;

    cells = (Cell*) malloc(logical_size.x * logical_size.y * sizeof(Cell));
    for (size_t i = 0; i < logical_size.x * logical_size.y; i++) {
        if (rand() % 2 == 0) {
            if (rand() % 3 == 0) {
                cells[i] = Cell { false, CellDefinition { CellType::Liquid, 0xFFFF0000 } };
            } else {
                cells[i] = Cell { false, CellDefinition { CellType::Powder, 0xFF00FFFF } };
            }
        } else {
            cells[i] = Cell { false, CellDefinition { CellType::None, 0x00000000 } };
        }
    }
}

Sandbox::~Sandbox() {
    free(cells);
    linearFree(texture_data);
}

bool Sandbox::can_move(size_t i, size_t j) const {
    CellType type_i = cells[i].definition.type;
    CellType type_j = cells[j].definition.type;
    return type_j == CellType::None
        || (type_i == CellType::Powder && type_j == CellType::Liquid && !cells[j].moved);
}

void Sandbox::update() {
    for (size_t i = 0; i < logical_size.x * logical_size.y; i++) {
        cells[i].moved = false;
    }

    for (size_t y = 0; y < logical_size.y; y++) {
        for (size_t x = 0; x < logical_size.x; x++) {
            size_t center = y * logical_size.x + x;
            if (cells[center].definition.type == CellType::None || cells[center].moved) continue;

            CellType type = cells[center].definition.type;

            size_t down = center + logical_size.x;
            size_t down_left = down - 1;
            size_t down_right = down + 1;
            size_t left = center - 1;
            size_t right = center + 1;

            bool can_move_down_left = false;
            bool can_move_down_right = false;
            bool can_move_left = false;
            bool can_move_right = false;

            if (type & 0b1 && y < logical_size.y - 1 && can_move(center, down)) {
                std::swap(cells[center], cells[down]);
                cells[down].moved = true;
            } else if (
                (can_move_down_left = type & 0b1 && y < logical_size.y - 1 && x > 0 && can_move(center, down_left))
                || (can_move_down_right = type & 0b1 && y < logical_size.y - 1 && x < logical_size.x - 1 && can_move(center, down_right))
            ) {
                if (rand() % 2 == 0) {
                    if (can_move_down_left) {
                        std::swap(cells[center], cells[down_left]);
                        cells[down_left].moved = true;
                    } else {
                        std::swap(cells[center], cells[down_right]);
                        cells[down_right].moved = true;
                    }
                } else {
                    if (can_move_down_right) {
                        std::swap(cells[center], cells[down_right]);
                        cells[down_right].moved = true;
                    } else {
                        std::swap(cells[center], cells[down_left]);
                        cells[down_left].moved = true;
                    }
                }
            } else if (
                (can_move_left = type & 0b10 && x > 0 && can_move(center, left))
                || (can_move_right = type & 0b10 && x < logical_size.x - 1 && can_move(center, right))
            ) {
                if (rand() % 2 == 0) {
                    if (can_move_left) {
                        std::swap(cells[center], cells[left]);
                        cells[left].moved = true;
                    } else {
                        std::swap(cells[center], cells[right]);
                        cells[right].moved = true;
                    }
                } else {
                    if (can_move_right) {
                        std::swap(cells[center], cells[right]);
                        cells[right].moved = true;
                    } else {
                        std::swap(cells[center], cells[left]);
                        cells[left].moved = true;
                    }
                }
            }
        }
    }

    for (size_t y = 0; y < logical_size.y; y++) {
        for (size_t x = 0; x < logical_size.x; x++) {
            texture_data[y * texture_size.x + x] = cells[y * logical_size.x + x].definition.color;
        }
    }

    ripConvertAndLoadC3DTexImage(&texture, texture_data, (GPU_TEXFACE) 0, 0);
}

void Sandbox::draw() const {
    C2D_DrawImageAt(image, 0.0f, 0.0f, 0.0f, nullptr, scale, scale);
}
