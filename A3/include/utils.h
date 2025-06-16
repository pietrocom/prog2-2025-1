#ifndef UTILS_H
#define UTILS_H

#include "player.h"

void init_allegro();

ALLEGRO_DISPLAY * create_display (int width, int height);

void split_spritesheet (const char *filename, int frame_width, int frame_height,
                      ALLEGRO_BITMAP **frames, int *frame_count);

bool soldier_supports_crouch (SoldierType type);

bool file_exists (const char* filename);

void draw_ground_line(struct GameLevel *level);

#endif