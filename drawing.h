#ifndef DRAWING_H
#define DRAWING_H

#include <SDL.h>

void draw_sequence_border(SDL_Surface *window);
void draw_sequence(SDL_Surface *window, int sequence[], int seq_len, int current_beat);
void highlight_beat(SDL_Surface *window, int sequence[], int seq_len, int index);

void display_start_button(SDL_Surface *window);
void display_stop_button(SDL_Surface *window);
void display_bpm(SDL_Surface *window, int bpm);

void display_info_dlg(SDL_Window *window);

#endif //DRAWING_H
