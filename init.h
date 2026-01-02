#ifndef INIT_H
#define INIT_H

SDL_Rect border_rect();
void load_images(void);
void load_fonts(void);
void init_sequence_rects();
void init_constants();
void create_buttons(SDL_Surface *window);
void free_surfaces();

#endif //INIT_H

