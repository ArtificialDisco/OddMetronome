#include <string.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "drawing.h"
#include "metronome.h"
#include "init.h"
#include "button.h"

extern int MaxBpmDigits;

extern SDL_Surface *s_img, *l_img, *s_highlight_img, *l_highlight_img;
extern SDL_Surface *numbers[10];
extern SDL_Rect seq_rects[];
extern Button start_stop_button, pling_button;

extern TTF_Font *info_font;

extern Uint32 next_time;

void draw_white_border(SDL_Surface *window, SDL_Rect all)
{
	SDL_Rect top, bottom, left, right;
	Uint32 white = SDL_MapRGB(window->format, 255, 255, 255);

	top.x = all.x, top.y = all.y;
	top.w = all.w, top.h = 1;

	left.x = all.x, left.y = all.y;
	left.w = 1, left.h = all.h;

	bottom.x = all.x, bottom.y = all.y + all.h - 1;
	bottom.w = all.w, bottom.h = 1;

	right.x = all.x + all.w - 1, right.y = all.y;
	right.w = 1, right.h = all.h;

	SDL_FillRect(window, &top, white);
	SDL_FillRect(window, &left, white);
	SDL_FillRect(window, &bottom, white);
	SDL_FillRect(window, &right, white);
}

void draw_sequence_border(SDL_Surface *window)
{
	SDL_Rect all = border_rect();
	draw_white_border(window, all);
}

//none of the following functions should be called before load_images()

void draw_sequence(SDL_Surface *window, int sequence[], int seq_len, int current_beat)
{
	Uint32 black = SDL_MapRGB(window->format, 0, 0, 0);
	int i;

	SDL_Rect area = border_rect();
	area.x += 5, area.y += 5;
	area.w -= 10, area.h -= 10;

	SDL_FillRect(window, &area, black);

	for(i = 0; i < seq_len; i++) {
		if(current_beat == i)
			if(sequence[i] == SHORT)
				SDL_BlitSurface(s_highlight_img, NULL, window, &seq_rects[i]);
			else
				SDL_BlitSurface(l_highlight_img, NULL, window, &seq_rects[i]);
		else
			if(sequence[i] == SHORT)
				SDL_BlitSurface(s_img, NULL, window, &seq_rects[i]);
			else
				SDL_BlitSurface(l_img, NULL, window, &seq_rects[i]);
	}
}

void highlight_beat(SDL_Surface *window, int sequence[], int seq_len, int index)
{
	if(index > seq_len - 1)
		return;

	int last = index - 1;

	if(last < 0)
		last = seq_len - 1;

	if(sequence[index] == SHORT)
		SDL_BlitSurface(s_highlight_img, NULL, window, &seq_rects[index]);
	else
		SDL_BlitSurface(l_highlight_img, NULL, window, &seq_rects[index]);

	if(sequence[last] == SHORT)
		SDL_BlitSurface(s_img, NULL, window, &seq_rects[last]);
	else
		SDL_BlitSurface(l_img, NULL, window, &seq_rects[last]);
}

void display_bpm(SDL_Surface *window, int bpm)
{
	int digits[10], i, length, height, ndigits = 0;

	SDL_Rect digit_rect;
	SDL_Rect update_rect;
	SDL_Rect border = border_rect();

	Uint32 black = SDL_MapRGB(window->format, 0, 0, 0);

	if(bpm > MAX_BPM || bpm < 0)
		return;

	while(bpm) {
		digits[ndigits++] = bpm % 10;
		bpm /= 10;
	}

	length = numbers[0]->w;
	height = numbers[0]->h;

	digit_rect.x = VIDEO_WIDTH - ndigits*length - 30;
	digit_rect.y = border.y + 10;

	update_rect.x = VIDEO_WIDTH - MaxBpmDigits*length - 30;
	update_rect.y = digit_rect.y;
	update_rect.w = MaxBpmDigits*length;
	update_rect.h = height;

	SDL_FillRect(window, &update_rect, black);
	for(i = ndigits - 1; i >= 0; i--) {
		SDL_BlitSurface(numbers[digits[i]], NULL, window, &digit_rect);
		digit_rect.x += length;
	}
}

void render_text(SDL_Surface *window_surface, SDL_Rect *area)
{
	SDL_Rect text_r;
	text_r.x = area->x + 10;
	text_r.y = area->y + 10;
	text_r.w = area->w;
	text_r.h = area->h;

	SDL_Color white;
	white.r = white.g = white.b  = 255;

	char text[9][100];
	strcpy(text[0], "Welcome to the odd metronome!");
	strcpy(text[1], "Press S or L to add short and long beats,");
	strcpy(text[2], "where the short is 1/4 and the long is 3/8.");
	strcpy(text[3], "");
	strcpy(text[4], "SPACE: start / stop");
	strcpy(text[5], "BACKSPACE: delete last");
	strcpy(text[6], "P: toggle 'pling' sound on first beat");
	strcpy(text[7], "C: clear sequence");
	strcpy(text[8], "(SHIFT) UP / DOWN: change tempo");

	SDL_Surface *text_surface;
	for(int i = 0; i < 9; i++) {
		if(strcmp(text[i], "") == 0) {
			text_r.y += 20;
			continue;
		}
		text_surface = TTF_RenderText_Solid(info_font, text[i],
			white);
		SDL_BlitSurface(text_surface, NULL, window_surface, &text_r);
	
		SDL_FreeSurface(text_surface);
		text_r.y += 28;
	}
}

void display_info_dlg(SDL_Window *window)
{
	SDL_Surface* window_surface = SDL_GetWindowSurface(window);
	SDL_Rect area;
	SDL_Surface *backup_window = SDL_ConvertSurface(window_surface, window_surface->format, 0);
	bool done = false;
	Button ok_button;
	SDL_Event event;

	area.x = 75, area.y = 50;
	area.w = VIDEO_WIDTH - 150;
	area.h = VIDEO_HEIGHT - 150;

	disable_all_buttons();

	SDL_FillRect(window_surface, &area, SDL_MapRGB(window_surface->format, 0, 0, 0));
	create_button(window_surface, &ok_button, area.x + area.w - 55, area.y + area.h - 39,
		(char*)"./img/ok_button.png", (char*)"./img/ok_button_hover.png",
		(char*)"./img/ok_button_pressed.png");

	draw_white_border(window_surface, area);

	render_text(window_surface, &area);

	SDL_UpdateWindowSurface(window);

	bool quit = false;
	while(!done) {
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				SDL_PushEvent(&event);
				quit = true;
				continue;
			}
			if(quit) { done = true; break; }
			
			if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_ESCAPE
				  || event.key.keysym.sym == SDLK_RETURN) {
					done = true;
				}
			} else if(event.type == SDL_MOUSEMOTION) {
				update_buttons(window_surface);
			} else if(event.type == SDL_MOUSEBUTTONDOWN) {
				update_button_press(window_surface, &event);
			} else if(event.type == SDL_MOUSEBUTTONUP) {
				if(is_pressed(&ok_button, &event)) {
					done = true;
				}
				release_buttons(window_surface);
			}
		}
		next_time += TICK_INTERVAL;
		SDL_Delay(time_left());
	}
	if(!quit) {
		SDL_BlitSurface(backup_window, &area, window_surface, &area);
		enable_all_buttons();
	}

	SDL_FreeSurface(backup_window);
	delete_button(&ok_button);
}
