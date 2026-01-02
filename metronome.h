#ifndef TETRIS_H
#define TETRIS_H

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#define VIDEO_WIDTH 600
#define VIDEO_HEIGHT 400

//must be smaller that VIDEO_WIDTH, of course
#define SEQUENCE_AREA_WIDTH 300

#define MAX_SEQUENCE 54
#define MAX_BPM 400

#define TICK_INTERVAL 10

#define SHORT 0
#define LONG 1

Uint32 time_left(void);
int sound_loop_func(void *data);
int play_sound(Mix_Chunk *sound);
SDL_Window *init_video();

int main(int argc, char *argv[]);

#endif //TETRIS_H
