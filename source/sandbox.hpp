#pragma once

#include <3ds.h>
#include <citro2d.h>

struct UVec2 {
    size_t x;
    size_t y;
};

enum CellType {
    None   = 0b00,
    Powder = 0b01,
    Liquid = 0b11,
};

struct CellDefinition {
    CellType type;
    u32 color;
};

struct Cell {
    bool moved = false;
    CellDefinition definition;
};

class Sandbox {
public:
    Sandbox(UVec2 screen_size = UVec2 { 400, 240 }, size_t scale = 2);
    ~Sandbox();

    bool can_move(size_t i, size_t j) const;
    void update();
    void draw() const;

private:
    UVec2 screen_size;
    size_t scale;
    UVec2 logical_size;
    UVec2 texture_size;
    u32* texture_data;
    C3D_Tex texture;
    Tex3DS_SubTexture subtexture;
    C2D_Image image;
    Cell* cells;
};
