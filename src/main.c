#include <math.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <SDL2/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PSD
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_PNM
#define STBI_ONLY_TGA
#include "stb_image.h"

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272

#define TZ_H  512/4
#define TZ_V  252/2
#define WW    TZ_H * 2.5
#define WH    TZ_V * 2.5

#define printf pspDebugScreenPrintf

extern unsigned char cheetah_png_start[], forest_png_start[];
extern unsigned int cheetah_png_size, forest_png_size;

/* Define the module info section */
PSP_MODULE_INFO("SDL2", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

static inline SDL_Color SDL_MakeColor(Uint8 r, Uint8 g, Uint8 b) {
	SDL_Color colour = {r, g, b};
	return colour;
}

static void SDL_ClearScreen(SDL_Renderer *renderer, SDL_Color colour) {
	SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, 255);
	SDL_RenderClear(renderer);
}

static void SDL_LoadImageMem(SDL_Renderer *renderer, SDL_Texture **texture, void *data, int size) {
	int width = 0, height = 0;
	unsigned char *image = stbi_load_from_memory((stbi_uc const *)data, size, &width, &height, NULL, STBI_rgb_alpha);
	if (image == NULL)
		printf("Loading image failed: %s", stbi_failure_reason());
	
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void *)image, width, height, 32, 4 * width, 0x000000ff, 0x0000ff00,
		0x00ff0000, 0xff000000);
	if (surface == NULL) {
		printf("Creating surface failed: %s", SDL_GetError());
		stbi_image_free(image);
	}
	
	*texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	stbi_image_free(image);
}

static void SDL_DrawImage(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y) {
	SDL_Rect position;
	position.x = x; position.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
	SDL_RenderCopy(renderer, texture, NULL, &position);
}

static float SDL_GetTime(void) {
	static Uint64 start = 0;
	static float frequency = 0;
	
	if (start == 0) {
		start = SDL_GetPerformanceCounter();
		frequency = (float)SDL_GetPerformanceFrequency();
		return 0.0f;
	}
	
	Uint64 counter = 0;
	counter = SDL_GetPerformanceCounter();
	return (((float)counter - (float)start) /frequency);
}

int main(int argc, char *argv[]) {
	SDL_Joystick *joystick = NULL;

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
		printf("SDL_Init() failed\n");
		
	if (SDL_NumJoysticks() < 1)
		printf("No joysticks connected!\n");
	else {
		SDL_JoystickEventState(SDL_ENABLE);
		joystick = SDL_JoystickOpen(0);
		if (joystick == NULL)
			printf("Unable to open game joystick! SDL Error: %s\n", SDL_GetError());
	}

	SDL_Renderer *renderer = NULL;
	SDL_Window *window = NULL;

	window = SDL_CreateWindow("SDL2-PSP", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_FULLSCREEN);

	if (window == NULL)
		printf("SDL_CreateWindow() failed\n");
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	SDL_Texture *forest = NULL;
	SDL_LoadImageMem(renderer, &forest, forest_png_start, forest_png_size);
	if (forest == NULL)
		printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
		
	float current_time = 0.0f, accumulator = 0.0f;
	SDL_Event event;
	int running = 1, ww = WW, wh = WH;
	
	SDL_Texture *cheetah = NULL;
	SDL_Rect cheetah_src, cheetah_dst;
	SDL_LoadImageMem(renderer, &cheetah, cheetah_png_start, cheetah_png_size);
	if (cheetah == NULL)
		printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
		
	cheetah_dst.w = TZ_H * 2;
	cheetah_dst.h = TZ_V * 2;
	cheetah_dst.x = ww / 2 - cheetah_dst.w / 2;
	cheetah_dst.y = wh / 2 - cheetah_dst.h / 2;
	
	cheetah_src.w = TZ_H;
	cheetah_src.h = TZ_V;
	cheetah_src.x = 0;
	cheetah_src.y = 0;
	
	while (running) {
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					running = 0;
					break;
				
				case SDL_JOYBUTTONDOWN:
					if (event.jbutton.which == 0) {
						if (event.jbutton.button == 7)
							cheetah_dst.x -= 5;
						else if (event.jbutton.button == 9)
							cheetah_dst.x += 5;
						else if (event.jbutton.button == 11) // Start button pressed
							running = 0;
					}
					break;
			}
		}

		float new_time = SDL_GetTime();
		float delta_time = new_time - current_time;
		current_time = new_time;
		accumulator += delta_time;
		if (accumulator >= 0.1) {
			cheetah_src.x += TZ_H;
			
			if (cheetah_src.x > TZ_H * 3) {
				cheetah_src.x = 0;
				
				if (cheetah_src.y == 0)
					cheetah_src.y = TZ_V;
				else
					cheetah_src.y = 0;
			}
			
			accumulator = 0;
		}
		
		SDL_ClearScreen(renderer, SDL_MakeColor(255, 255, 255));
		SDL_DrawImage(renderer, forest, 0, 0);
		SDL_RenderCopy(renderer, cheetah, &cheetah_src, &cheetah_dst);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(cheetah);
	SDL_DestroyTexture(forest);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_JoystickClose(joystick);
	SDL_Quit();
	sceKernelExitGame();
	return 0;
}
