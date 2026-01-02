#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>

#include "init.h"
#include "metronome.h"
#include "button.h"

//derived constants
int MaxBpmDigits;

Button short_button, long_button, back_button, up_button, down_button,
	start_stop_button, long_up_button, long_down_button, pling_button,
	clear_button, info_button;

SDL_Surface *s_img, *l_img, *s_highlight_img, *l_highlight_img;
SDL_Surface *numbers[10];

TTF_Font *info_font;

SDL_Rect seq_rects[MAX_SEQUENCE];

SDL_Rect border_rect()
{
	SDL_Rect border;

	border.x = (VIDEO_WIDTH - SEQUENCE_AREA_WIDTH) / 2, border.y = 50;
	border.w = SEQUENCE_AREA_WIDTH;
	border.h = VIDEO_HEIGHT - 200;

	return border;
}

//constants that derive from the #define'd
//ones are initialized here
void init_constants()
{
	int n = MAX_BPM;

	MaxBpmDigits = 0;
	do {
		MaxBpmDigits++;
	} while(n /= 10);
}

void load_images()
{
	int i;
	char num_png[] = "./img/x.png";

	s_img = IMG_Load("./img/s.png");
	l_img = IMG_Load("./img/l.png");
	s_highlight_img = IMG_Load("./img/s_highlight.png");
	l_highlight_img = IMG_Load("./img/l_highlight.png");
	if(!s_img || !l_img || !s_highlight_img || !l_highlight_img) {
		printf("IMG_Load: %s\n", IMG_GetError());
		exit(1);
	}

	for(i = 0; i < 10; i++) {
		sprintf(num_png, "./img/%i.png", i);
		numbers[i] = IMG_Load(num_png);
		if(!numbers[i]) {
			printf("IMG_Load: %s\n", IMG_GetError());
			exit(1);
		}
	}
}

void load_fonts()
{
	info_font = TTF_OpenFont("./fonts/Bank Gothic Light BT.ttf", 16);
	if(!info_font) {
    	printf("TTF_OpenFont: %s\n", TTF_GetError());
    	exit(1);
	}
}
	
void create_buttons(SDL_Surface *window)
{
	SDL_Rect border = border_rect();
	
	SDL_Rect long_button_rect;
	int x = (border.x - 100) / 2;
	int y = border.y + 10;

	//deprecated something something
	create_button(window, &back_button, x, y,
		(char*)"./img/back_button.png",
		(char*)"./img/back_button_hover.png",
		(char*)"./img/back_button_pressed.png");
	create_button(window, &long_button, x, y + 55,
		(char*)"./img/long_button.png",
		(char*)"./img/long_button_hover.png",
		(char*)"./img/long_button_pressed.png");
	create_button(window, &short_button, x, y + 95,
		(char*)"./img/short_button.png",
		(char*)"./img/short_button_hover.png",
		(char*)"./img/short_button_pressed.png");

	create_toggle_button(window, &pling_button, x, y + 150,
		(char*)"./img/pling_button.png", (char*)"./img/pling_off_button.png");

	create_button(window, &clear_button, x,
		(VIDEO_HEIGHT + border.y + border.h - 34) / 2,
		(char*)"./img/clear_button.png", 
		(char*)"./img/clear_button_hover.png",
		(char*)"./img/clear_button_pressed.png");
	
	x = VIDEO_WIDTH - 110;

	create_button(window, &info_button, (VIDEO_WIDTH + 350) / 2,
		(VIDEO_HEIGHT + border.y + border.h - 34) / 2,
		(char*)"./img/info_button.png", 
		(char*)"./img/info_button_hover.png",
		(char*)"./img/info_button_pressed.png");

	create_button(window, &up_button, x - 15, y + 55,
		(char*)"./img/up_button.png",
		(char*)"./img/up_button_hover.png",
		(char*)"./img/up_button_pressed.png");
	
	create_button(window, &long_up_button, x + up_button.rect.w - 10, y + 59,
		(char*)"./img/long_up_button.png",
		(char*)"./img/long_up_button_hover.png",
		(char*)"./img/long_up_button_pressed.png");

	create_button(window, &down_button, x - 15, y + 90,
		(char*)"./img/down_button.png",
		(char*)"./img/down_button_hover.png",
		(char*)"./img/down_button_pressed.png");

	create_button(window, &long_down_button, x + down_button.rect.w - 10, y + 91,
		(char*)"./img/long_down_button.png",
		(char*)"./img/long_down_button_hover.png",
		(char*)"./img/long_down_button_pressed.png");

	//we're loading the start button image twice... might need to fix that.
	create_toggle_button(window, &start_stop_button, (VIDEO_WIDTH - 300) / 2,
		(VIDEO_HEIGHT + border.y + border.h - 50) / 2,
		(char*)"./img/start_button.png", (char*)"./img/stop_button.png");
}

void init_sequence_rects()
{
	int i;
	SDL_Rect current_rect;
	SDL_Rect area = border_rect();
	area.x += 5, area.y += 5;
	area.w -= 10, area.h -= 10;
	
	current_rect.x = area.x, current_rect.y = area.y;
	current_rect.w = s_img->w, current_rect.h = s_img->h;

	for(i = 0; i < MAX_SEQUENCE; i++) {
		seq_rects[i].x = current_rect.x, seq_rects[i].y = current_rect.y;
		seq_rects[i].w = current_rect.w, seq_rects[i].h = current_rect.h;
		current_rect.x += s_img->w + 2;
		if(current_rect.x + s_img->w + 2 >= area.x + area.w) {
			current_rect.x = area.x;
			current_rect.y += s_img->h;
		}
	}
}

void free_surfaces()
{
	int i;

	SDL_FreeSurface(s_img);
	SDL_FreeSurface(l_img);
	SDL_FreeSurface(s_highlight_img);
	SDL_FreeSurface(l_highlight_img);
	
	for(i = 0; i < 10; i++)
		SDL_FreeSurface(numbers[i]);
}
