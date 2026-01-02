#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_image.h>

#include "button.h"
#include "metronome.h"

Button *button_list[MAX_BUTTONS];
int nbuttons = 0;

bool buttons_need_screen_update = false;

void set_img(SDL_Surface *surface, SDL_Rect *rect, SDL_Surface *img)
{
	SDL_BlitSurface(img, NULL, surface, rect);
	buttons_need_screen_update = true;
	//SDL_UpdateRect(surface, rect->x, rect->y, rect->w, rect->h);
}

bool inside_rect(SDL_Rect *rect, int x, int y)
{
	if(x >= rect->x && x <= rect->x + rect->w
	  && y >= rect->y && y <= rect->y + rect->h)
		return true;
	return false;
}

bool button_pressed(SDL_Surface *window, SDL_Event *event, Button *button)
{
	if(inside_rect(&button->rect, event->button.x, event->button.y)
	  && event->type == SDL_MOUSEBUTTONDOWN) {
		if(button->press_img != NULL)
			set_img(window, &button->rect, button->press_img);
		button->pressed = true;
		return true;
	}
	return false;
}

void update_button_press(SDL_Surface *window, SDL_Event *event)
{
	int i;
	for(i = 0; i < nbuttons; i++) {
		if(!button_list[i]->active)
			continue;
		if(inside_rect(&button_list[i]->rect, event->button.x, event->button.y)
		  && event->type == SDL_MOUSEBUTTONDOWN) {
			if(button_list[i]->press_img != NULL)
				set_img(window, &button_list[i]->rect,
					button_list[i]->press_img);
			button_list[i]->pressed = true;
		}
	}
}

void toggle_button(SDL_Surface *window, Button *button)
{
	int i;
	for(i = 0; i < nbuttons; i++) {
		if(button_list[i] == button) {
			if(button->off_img != NULL)
				if(button->toggled == true) {
					set_img(window, &button->rect, button->off_img);
					button->toggled = false;
				} else {
					set_img(window, &button->rect, button->img);
					button->toggled = true;
				}
			break;
		}
	}
}

//is almost the same as update_button_press,
//but is performed at a different time
void update_button_toggle(SDL_Surface *window, SDL_Event *event)
{
	int i;
	Button *b;
	for(i = 0; i < nbuttons; i++) {
		if(!button_list[i]->active)
			continue;
		if(inside_rect(&button_list[i]->rect, event->button.x, event->button.y)
		  && event->type == SDL_MOUSEBUTTONUP) {
			b = button_list[i];
			if(b->off_img != NULL)
				if(b->toggled == true) {
					set_img(window, &b->rect, b->off_img);
					b->toggled = false;
				} else {
					set_img(window, &b->rect, b->img);
					b->toggled = true;
				}
		}
	}
}

bool is_pressed(Button *button, SDL_Event *event)
{
	if(inside_rect(&button->rect, event->button.x, event->button.y)
	  && button->pressed)
		return true;
	return false;
}

void release_buttons(SDL_Surface *window)
{
	int i;
	for(i = 0; i < nbuttons; i++) {
		if(!button_list[i]->active)
			continue;
		if(button_list[i]->pressed) {
			button_list[i]->pressed = false;
			if(button_list[i]->hover)
				set_img(window, &button_list[i]->rect, button_list[i]->hover_img);
			else
				set_img(window, &button_list[i]->rect, button_list[i]->img);
		}
	}
}

void create_button(SDL_Surface *dest, Button *button, int x, int y,
	char *img, char *hover_img, char *press_img)
{
	button->rect.x = x;
	button->rect.y = y;

	button->active = true;

	button->hover = false;
	button->pressed = false;

	button->img = IMG_Load(img);
	if(hover_img != NULL)
		button->hover_img = IMG_Load(hover_img);
	else
		button->hover_img = NULL;

	if(press_img != NULL)
		button->press_img = IMG_Load(press_img);
	else
		button->press_img = NULL;

	button->off_img = NULL;
	button->toggled = false;

	if(!button->img || (hover_img != NULL && !button->hover_img)) {
		printf("IMG_Load: %s\n", IMG_GetError());
		exit(1);
	}

	if(x < 0 || x + button->img->w > VIDEO_WIDTH || y < 0 || y + button->img->h > VIDEO_HEIGHT) {
		printf("Couldn't create button %s, bad coordinates.", img);
		exit(1);
	}
	
	button->rect.w = button->img->w;
	button->rect.h = button->img->h;
		
	SDL_BlitSurface(button->img, NULL, dest, &button->rect);
	//SDL_UpdateRect(dest, x, y, button->rect.w, button->rect.h);

	button_list[nbuttons++] = button;
}

void create_toggle_button(SDL_Surface *dest, Button *button, int x, int y,
	char *img, char *off_img)
{
	button->rect.x = x;
	button->rect.y = y;
	
	button->active = true;

	button->hover = false;
	button->pressed = false;

	button->img = IMG_Load(img);
	if(off_img != NULL)
		button->off_img = IMG_Load(off_img);
	else
		button->off_img = NULL;

	button->hover_img = NULL;
	button->press_img = NULL;
	button->toggled = true;

	if(!button->img || (off_img != NULL && !button->off_img)) {
		printf("IMG_Load: %s\n", IMG_GetError());
		exit(1);
	}

	if(x < 0 || x + button->img->w > VIDEO_WIDTH || y < 0 || y + button->img->h > VIDEO_HEIGHT) {
		printf("Couldn't create button %s, bad coordinates.", img);
		exit(1);
	}
	
	button->rect.w = button->img->w;
	button->rect.h = button->img->h;
		
	SDL_BlitSurface(button->img, NULL, dest, &button->rect);

	button_list[nbuttons++] = button;
}

void delete_button(Button *button)
{
	int i, j;
	for(i = 0; i < nbuttons; i++)
		if(button_list[i] == button) {
			button->rect.x = button->rect.y = 0;
			button->rect.w = button->rect.h = 0;
			//button->img = button->hover_img = button->press_img = NULL;

			SDL_FreeSurface(button->img);
			SDL_FreeSurface(button->hover_img);
			SDL_FreeSurface(button->press_img);
			SDL_FreeSurface(button->off_img);
		
			for(j = i; j < nbuttons-1; j++)
				button_list[j] = button_list[j+1];
			nbuttons--;
		}
}

void delete_all_buttons()
{
	int i;
	for(i = 0; i < nbuttons; i++) {
		if(button_list[i]->img != NULL)
			SDL_FreeSurface(button_list[i]->img);
		if(button_list[i]->hover_img != NULL)
			SDL_FreeSurface(button_list[i]->hover_img);
		if(button_list[i]->press_img != NULL)
			SDL_FreeSurface(button_list[i]->press_img);
		if(button_list[i]->off_img != NULL)
			SDL_FreeSurface(button_list[i]->off_img);
	}
	nbuttons = 0;
}

void disable_all_buttons()
{
	int i;
	for(i = 0; i < nbuttons; i++)
		button_list[i]->active = false;
}

void enable_all_buttons()
{
	int i;
	for(i = 0; i < nbuttons; i++)
		button_list[i]->active = true;
}

bool buttons_equal(Button b1, Button b2)
{
	//if b1 and b2 share same location and size
	//they're 'equal'
	if(b1.rect.x == b2.rect.x && b1.rect.y == b2.rect.y
	  && b1.rect.w == b2.rect.w && b1.rect.h == b2.rect.h)
		return true;
	return false;
}

void update_buttons(SDL_Surface *window)
{
	int i;
	int x, y;
	Button *b;

	SDL_GetMouseState(&x, &y);

	for(i = 0; i < nbuttons; i++) {
		b = button_list[i];
		if(!b->active)
			continue;
		else if(inside_rect(&b->rect, x, y) && !b->hover) {
			if(!b->pressed)
				set_img(window, &button_list[i]->rect, b->hover_img);
			b->hover = true;
		} else if(!inside_rect(&button_list[i]->rect, x, y) && b->hover) {
			if(!b->pressed && (b->toggled || b->off_img == NULL))
				set_img(window, &b->rect, b->img);
			else
				set_img(window, &b->rect, b->off_img);
			b->hover = false;
		}
	}
}
