#ifndef BUTTON_H
#define BUTTON_H

#include <SDL.h>

#define MAX_BUTTONS 50

typedef struct {
	SDL_Rect rect;
	SDL_Surface *img;
	SDL_Surface *hover_img;
	SDL_Surface *press_img;
	SDL_Surface *off_img;
	bool hover;
	bool pressed;
	bool toggled;
	bool active;
} Button;

void set_img(SDL_Surface*, SDL_Rect*, SDL_Surface*);
bool inside_rect(SDL_Rect *rect, int x, int y);

//create a button.  button must have an image for normal mode and
//hovering, which is an SDL_Surface*.  button is preferably an empty
//Button struct which gets its value assigned here.
void create_button(SDL_Surface *dest, Button *button, int x, int y,
	char *img, char *hover_img, char *press_img);
void create_toggle_button(SDL_Surface *dest, Button *button, int x, int y,
	char *img, char *off_img);

void toggle_button(SDL_Surface *window, Button *button);
void update_button_toggle(SDL_Surface *window, SDL_Event *event);

bool buttons_equal(Button b1, Button b2);
void update_buttons(SDL_Surface *window);

void release_buttons(SDL_Surface *window);
bool is_pressed(Button *button, SDL_Event *event);

void update_button_press(SDL_Surface *window, SDL_Event *event);
void update_button_toggle(SDL_Surface *window, SDL_Event *event);

void delete_button(Button *button);
void delete_all_buttons();

void disable_all_buttons();
void enable_all_buttons();

#endif //BUTTON_H
