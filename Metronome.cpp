#include <SDL.h>

#ifdef __linux__ 
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_ttf.h>
#elif _WIN32
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_thread.h>
#include <SDL_ttf.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MINGW
#include <windows.h>
#endif

#include "metronome.h"
#include "init.h"
#include "drawing.h"
#include "button.h"

#define KLACK "./sounds/klack.wav"
#define PLING "./sounds/pling.wav"

extern Button short_button, long_button, back_button, up_button, down_button,
	long_up_button, long_down_button, start_stop_button, pling_button, clear_button,
	info_button;

extern TTF_Font *info_font;

extern bool buttons_need_screen_update;

SDL_Thread *sound_loop_thread;
Mix_Chunk *klack;
Mix_Chunk *pling;

typedef struct {
	SDL_Surface *window;
	int sequence[MAX_SEQUENCE];
	int seq_len;
	int bpm;
	int current_beat;
	bool pling_on;
} LoopThreadData;

Uint32 next_time;

Uint32 time_left(void)
{
	Uint32 now;
	now = SDL_GetTicks();
	if(next_time <= now)
		return 0;
	else
		return next_time - now;
}

bool keys_need_screen_update = false;

void UpdateScreenIfNeeded(SDL_Window* window) {
	if (buttons_need_screen_update) {
		SDL_UpdateWindowSurface(window);
		buttons_need_screen_update = false;
	}
	if (keys_need_screen_update) {
		SDL_UpdateWindowSurface(window);
		keys_need_screen_update = false;
	}
}

SDL_Window *init_video()
{
	//SDL_Surface *window = SDL_SetVideoMode(VIDEO_WIDTH, VIDEO_HEIGHT, 16,
	//	SDL_SWSURFACE);
	SDL_Window* window = SDL_CreateWindow("OddMetronome", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, VIDEO_WIDTH,
		VIDEO_HEIGHT, SDL_WINDOW_SHOWN);

	printf("init video mode %ix%i 16bpp ... ", VIDEO_WIDTH, VIDEO_HEIGHT);
	
	if(window == NULL) {
		printf("Couldn't open video mode: %s\n", SDL_GetError());
		exit(1);
	}
	
	printf("done!\n");
	
	return window;
}

int play_sound(Mix_Chunk *sound)
{
	int channel;
 
	channel = Mix_PlayChannel(-1, sound, 0);
	if(channel == -1) {
		fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
	}

	return channel;
}

bool loop_running = false;
int sound_loop_func(void *data)
{
	SDL_Surface *window = ((LoopThreadData*)data)->window;
	int *sequence = ((LoopThreadData*)data)->sequence;
	int *seq_len = &((LoopThreadData*)data)->seq_len;
	int *bpm = &((LoopThreadData*)data)->bpm;
	int *current_beat = &((LoopThreadData*)data)->current_beat;
	bool *pling_on = &((LoopThreadData*)data)->pling_on;

	int s = 60000.0 / *bpm;
	int l = (int)(90000.0 / *bpm);  //this is just (1.5 * s)
	
	int i = 0;

	int channel = -1;
	int delay;

	Uint32 now = SDL_GetTicks();
	Uint32 next_time = now;

	int last_bpm = *bpm;
	while(loop_running) {
		if(last_bpm != *bpm) {
			s = 60000.0 / *bpm;
			l = (int)(90000.0 / *bpm);
		}

		if(*seq_len > 0)
			if(i == 0 && *pling_on)
				play_sound(pling);
			else
				play_sound(klack);

		if(((LoopThreadData*)data)->sequence[i] == 0)
			next_time += s;
		else
			next_time += l;
		*current_beat = i;

		i++;
		if(i >= *seq_len)
			i = 0;

		last_bpm = *bpm;

		Uint32 now = SDL_GetTicks();
		delay = next_time - now;

		//all that tick interval stuff is so we can continually check
		//when the loop stops and make a snappier user interface
		while(delay > TICK_INTERVAL) {
			SDL_Delay(TICK_INTERVAL);
			delay -= TICK_INTERVAL;
			if(!loop_running)
				return 0;
		}

		//in case the delay still happens to be positive after the last tick
		//interval... we're aiming for absolute precision here
		if(delay > 0)
			SDL_Delay(delay);
	}

	return 0;
}

void debug_sequence(LoopThreadData *thread_data)
{
	int i;
	printf("{ ");
	for(i = 0; i < thread_data->seq_len - 1; i++)
		printf("%i, ", thread_data->sequence[i]);
	printf("%i }\n", thread_data->sequence[i]);
}

void add_beat(SDL_Surface *window, LoopThreadData *thread_data, int length, bool loop_running)
{
	int n;
	if(thread_data->seq_len < MAX_SEQUENCE) {
		thread_data->sequence[thread_data->seq_len] = length;
		thread_data->seq_len++;
		if(loop_running)
			n = thread_data->current_beat;
		else
			n = -1;
		draw_sequence(window, thread_data->sequence,
			thread_data->seq_len, n);
	}
}

void remove_beat(SDL_Surface *window, LoopThreadData *thread_data, bool loop_running)
{
	int n;
	if(thread_data->seq_len > 0) {
		if(loop_running)
			n = thread_data->current_beat;
		else
			n = -1;
		thread_data->seq_len--;
		draw_sequence(window, thread_data->sequence,
			thread_data->seq_len, n);
	} else
		thread_data->seq_len = 0;
}

void toggle_loop(SDL_Surface *window, LoopThreadData *thread_data)
{
	if(loop_running) {
		loop_running = false;
		SDL_WaitThread(sound_loop_thread, NULL);
		thread_data->current_beat = -1;
		draw_sequence(window, thread_data->sequence,
			thread_data->seq_len, thread_data->current_beat);
	} else {
		loop_running = true;
		sound_loop_thread
			= SDL_CreateThread(sound_loop_func, "Carl", thread_data);
	}
}

void clear_sequence(SDL_Surface *window, LoopThreadData *thread_data)
{
	thread_data->seq_len = 0;
	draw_sequence(window, thread_data->sequence,
		thread_data->seq_len, -1);
}

void toggle_pling(LoopThreadData *thread_data)
{
	if(thread_data->pling_on == true) {
		thread_data->pling_on = false;
	} else {
		thread_data->pling_on = true;
	}
}

void cleanup()
{
	free_surfaces();
	delete_all_buttons();
	TTF_CloseFont(info_font);
	SDL_Quit();
}

void exit_program(SDL_Surface *window)
{
	SDL_Surface *closing;
	SDL_Rect r;

	closing = IMG_Load("./img/closing.png");
	if(!closing) {
		printf("IMG_Load: %s\n", IMG_GetError());
	}

	r.x = (VIDEO_WIDTH - closing->w) / 2;
	r.y = (VIDEO_HEIGHT - closing->h) / 2;
	r.w = closing->w;
	r.h = closing->h;

	SDL_BlitSurface(closing, NULL, window, &r);
	//SDL_UpdateRect(window, r.x, r.y, r.w, r.h);

	loop_running = false;
				
	Mix_FreeChunk(klack);
	Mix_FreeChunk(pling);
				
	Mix_CloseAudio();
	cleanup();
	SDL_FreeSurface(closing);
	SDL_Quit();
	exit(0);
}

void handle_keyboard(SDL_Surface *window, SDL_Event *event, LoopThreadData *thread_data)
{
	static bool space_pressed = false;
	bool shift = SDL_GetModState() | 0xff == KMOD_SHIFT;
	int bpm;

	if(event->type == SDL_KEYDOWN) {
		switch(event->key.keysym.sym) {
			case SDLK_ESCAPE:
				exit_program(window);
				break;
			case ' ':
				if(!space_pressed) {
					space_pressed = true;
					toggle_loop(window, thread_data);
					toggle_button(window, &start_stop_button);
					keys_need_screen_update = true;
				}
				break;
			case 's':
				add_beat(window, thread_data, SHORT, loop_running);
				keys_need_screen_update = true;
				break;
			case 'l':
				add_beat(window, thread_data, LONG, loop_running);
				keys_need_screen_update = true;
				break;
			case 'p':
				toggle_pling(thread_data);
				toggle_button(window, &pling_button);
				keys_need_screen_update = true;
				break;
			case 'c':
				clear_sequence(window, thread_data);
				keys_need_screen_update = true;
				break;
			case SDLK_BACKSPACE:
				remove_beat(window, thread_data, loop_running);
				keys_need_screen_update = true;
				break;
			case SDLK_UP:
			  bpm = thread_data->bpm + (shift ? 1 : 10);
				if(bpm < MAX_BPM) {
					thread_data->bpm = bpm;
				  keys_need_screen_update = true;
				}
				break;
			case SDLK_DOWN:
				bpm = thread_data->bpm - (shift ? 1 : 10);
				if(bpm > 0) {
					thread_data->bpm = bpm; 
				  keys_need_screen_update = true;
				}
				break;
		}
	} else if(event->type == SDL_KEYUP) {
		switch(event->key.keysym.sym) {
			case ' ':
				space_pressed = false;
				break;
		}
	}
}

#ifdef MINGW
int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
    return main(0, NULL);
}
#endif

int main(int argc, char *argv[])
{
	SDL_Window* window;
	SDL_Event event;
	int current_bpm = 180;
	int last_beat = -1;
	int sequence[MAX_SEQUENCE] = { SHORT, SHORT, SHORT, SHORT };
	int seq_len = 4;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	if(TTF_Init() == -1) {
    	printf("TTF_Init: %s\n", TTF_GetError());
    	exit(2);
	}
	
	window = init_video();
	SDL_Surface* window_surface = SDL_GetWindowSurface(window);

	load_images();
	load_fonts();
	init_sequence_rects();
	init_constants();
	draw_sequence_border(window_surface);
	draw_sequence(window_surface, sequence, seq_len, -1);
	create_buttons(window_surface);
	display_bpm(window_surface, current_bpm);

	int audio_rate = 44100;
	Uint16 audio_format = AUDIO_S16SYS;
	int audio_channels = 2;
	int audio_buffers = 1024;
 
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
		fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
		exit(1);
	}

	klack = pling = NULL;
	klack = Mix_LoadWAV(KLACK);
	if(klack == NULL) {
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
		exit(1);
	}

	pling = Mix_LoadWAV(PLING);
	if(pling == NULL) {
		fprintf(stderr, "Unable to load WAV file: %s\n", Mix_GetError());
		exit(1);
	}

	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	LoopThreadData thread_data;
	thread_data.window = window_surface;
	thread_data.bpm = current_bpm;
	thread_data.current_beat = 0;
	thread_data.pling_on = true;

	for(int i = 0; i < seq_len; i++)
		thread_data.sequence[i] = sequence[i];
	thread_data.seq_len = seq_len;

	int up_button_timer = 0, down_button_timer = 0;
	int long_up_button_timer = 0, long_down_button_timer = 0;
	bool up_button_pressed = false, down_button_pressed = false;
	bool long_up_button_pressed = false, long_down_button_pressed = false;

	SDL_UpdateWindowSurface(window);
	
	while(1) {
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				exit_program(window_surface);
			} else if(event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
				handle_keyboard(window_surface, &event, &thread_data);
			} else if(event.type == SDL_MOUSEMOTION) {
				update_buttons(window_surface);
			} else if(event.type == SDL_MOUSEBUTTONDOWN) {
				update_button_press(window_surface, &event);
				
				if(is_pressed(&up_button, &event)) {
					up_button_pressed = true;
					up_button_timer = 0;
					if(thread_data.bpm + 10 < MAX_BPM)
						thread_data.bpm += 1;
				}
				
				else if(is_pressed(&down_button, &event)) {
					down_button_pressed = true;
					down_button_timer = 0;
					if(thread_data.bpm > 0)
						thread_data.bpm -= 1;
					if(thread_data.bpm <= 0)
						thread_data.bpm = 1;
				}
				
				else if(is_pressed(&long_up_button, &event)) {
					long_up_button_pressed = true;
					long_up_button_timer = 0;
					if(thread_data.bpm < MAX_BPM)
						thread_data.bpm += 10;

				}
				
				else if(is_pressed(&long_down_button, &event)) {
					long_down_button_pressed = true;
					long_down_button_timer = 0;
					if(thread_data.bpm > 0)
						thread_data.bpm -= 10;
					if(thread_data.bpm <= 0)
						thread_data.bpm = 10;
				}

			} else if(event.type == SDL_MOUSEBUTTONUP) {
				update_button_toggle(window_surface, &event);

				up_button_pressed = down_button_pressed = false;
				long_up_button_pressed = long_down_button_pressed = false;

				if(is_pressed(&short_button, &event))
					add_beat(window_surface, &thread_data, SHORT, loop_running);
				
				else if(is_pressed(&long_button, &event))
					add_beat(window_surface, &thread_data, LONG, loop_running);

				else if(is_pressed(&back_button, &event))
					remove_beat(window_surface, &thread_data, loop_running);
				
				else if(is_pressed(&start_stop_button, &event))
					toggle_loop(window_surface, &thread_data);
				
				else if(is_pressed(&pling_button, &event))
					toggle_pling(&thread_data);
				
				else if(is_pressed(&clear_button, &event))
					clear_sequence(window_surface, &thread_data);
				
				else if(is_pressed(&info_button, &event)) {
					if(loop_running) {
						toggle_loop(window_surface, &thread_data);
						toggle_button(window_surface, &start_stop_button);
					}
					display_info_dlg(window);
				}
				release_buttons(window_surface);
			}
		}

		if(up_button_pressed) {
			up_button_timer += TICK_INTERVAL;
			if(up_button_timer > 100) {
				if(thread_data.bpm < MAX_BPM)
					thread_data.bpm += 1;
				up_button_timer = 0;
			}
		}

		if(down_button_pressed) {
			down_button_timer += TICK_INTERVAL;
			if(down_button_timer > 100) {
				if(thread_data.bpm > 0)
					thread_data.bpm -= 1;
				down_button_timer = 0;
			}
		}

		if(long_up_button_pressed) {
			long_up_button_timer += TICK_INTERVAL;
			if(long_up_button_timer > 100) {
				if(thread_data.bpm < MAX_BPM)
					thread_data.bpm += 10;
				long_up_button_timer = 0;
			}
		}

		if(long_down_button_pressed) {
			long_down_button_timer += TICK_INTERVAL;
			if(long_down_button_timer > 100) {
				if(thread_data.bpm > 0)
					thread_data.bpm -= 10;
				long_down_button_timer = 0;
			}
		}

		if(current_bpm != thread_data.bpm) {
			current_bpm = thread_data.bpm;
			display_bpm(window_surface, current_bpm);
		}

		if(loop_running) {
			if(last_beat != thread_data.current_beat && loop_running) {
				draw_sequence(window_surface, thread_data.sequence,
					thread_data.seq_len, thread_data.current_beat);
				SDL_UpdateWindowSurface(window);
			}
			last_beat = thread_data.current_beat;
		}
		UpdateScreenIfNeeded(window);
		next_time += TICK_INTERVAL;
		SDL_Delay(time_left());
	}
}
