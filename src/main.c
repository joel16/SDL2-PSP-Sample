#include <pspkernel.h>
#include <pspdebug.h>
#include <SDL2/SDL.h>

/* Define the module info section */
PSP_MODULE_INFO("SDL2", 0, 1, 1);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define MUS_PATH "Roland-GR-1-Trumpet-C5.wav"

// variable declarations
static Uint8 *audio_pos; // global pointer to the audio buffer to be played
static Uint32 audio_len; // remaining length of the sample we have to play

void my_audio_callback(void *userdata, Uint8 *stream, int len) {
	
	if (audio_len == 0)
		return;
	
	len = (len > audio_len ? audio_len : len);
	SDL_memcpy(stream, audio_pos, len); 					// simply copy from one buffer into the other
	//SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);// mix from one buffer into another
	
	audio_pos += len;
	audio_len -= len;
}

void draw_rects(SDL_Renderer *renderer, int x, int y) {
    // R
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect r = {x, y, 64, 64};
    SDL_RenderFillRect(renderer, &r);

    // G
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect g = {x + 64, y, 64, 64};
    SDL_RenderFillRect(renderer, &g);

    // B
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect b = {x + 128, y, 64, 64};
    SDL_RenderFillRect(renderer, &b);
}

int main(int argc, char *argv[]) {
	SDL_Joystick *joystick = NULL;
    SDL_Event event;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    int done = 0, x = 0, w = 480, h = 272;

	// SET THIS TO ACTIVATE joystick
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    // create an SDL window (pspgl enabled)
    window = SDL_CreateWindow("sdl2_psp", 0, 0, 480, 272, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // create a renderer (OpenGL ES2)
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

	if (SDL_NumJoysticks() < 1)
		SDL_Log("No joysticks connected: %s\n", SDL_GetError());
	
    joystick = SDL_JoystickOpen(0);
	if (joystick == NULL) {
		SDL_Log("SDL_JoystickOpen: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	// Debugging
	//if (joystick)
	//	SDL_Log("Joystick found: %s\n", SDL_JoystickName(joystick));

	// local variables
	static Uint32 wav_length; // length of our sample
	static Uint8 *wav_buffer; // buffer containing our audio file
	static SDL_AudioSpec wav_spec; // the specs of our piece of music

	/* Load the WAV */
	// the specs, length and buffer of our wav are filled
	if (SDL_LoadWAV(MUS_PATH, &wav_spec, &wav_buffer, &wav_length) == NULL) {
		SDL_Log("SDL_LoadWAV failed: %s\n", SDL_GetError());
		return -1;
	}

	// set the callback function
	wav_spec.callback = my_audio_callback;
	wav_spec.userdata = NULL;
	audio_pos = wav_buffer; // copy sound buffer
	audio_len = wav_length; // copy file length
	
	/* Open the audio device */
	if (SDL_OpenAudio(&wav_spec, NULL) < 0){
		SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
		return -1;
	}
	
	/* Start playing */
	SDL_PauseAudio(0);

    while (!done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_JOYAXISMOTION:
                    SDL_Log("Joystick %d axis %d value: %d\n",
                            event.jaxis.which,
                            event.jaxis.axis, event.jaxis.value);
                    break;

                case SDL_JOYBUTTONDOWN:
                    SDL_Log("Joystick %d button %d down\n",
                            event.jbutton.which, event.jbutton.button);
                    // seek for joystick #0
                    if (event.jbutton.which == 0) {
                        if (event.jbutton.button == 2) {
							audio_pos = wav_buffer; // copy sound buffer
							audio_len = wav_length;
                        }
						else if (event.jbutton.button == 11) {
                            // (Start) button down
                            done = 1;
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // fill window bounds
        SDL_SetRenderDrawColor(renderer, 111, 111, 111, 255);
        SDL_GetWindowSize(window, &w, &h);
        SDL_Rect f = {0, 0, w, h};
        SDL_RenderFillRect(renderer, &f);

        draw_rects(renderer, x, 0);
        draw_rects(renderer, x, h - 64);

        SDL_RenderPresent(renderer);

        x++;
        if (x > w - 192) {
            x = 0;
        }
    }

	// shut everything down
	SDL_CloseAudio();
	SDL_FreeWAV(wav_buffer);
	SDL_JoystickClose(joystick);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
	SDL_Quit();
	sceKernelExitGame();

    return 0;
}
