/* *************Interactive Square***************
 * Press w to increase square size.
 * Press s to decrease square size.
 * It's not really a square.
 * It's a rectangle that matches the aspect ratio of the game window.
 * *******************************/
/* *************Demo Poll vs Filter***************
 * Demonstrate difference between Poll events and Filter events.
 * Hold down a key:
 * Poll : responds right away once, then pause, then rapid fire
 * Filter : responds right away and immediately starts rapid fire, no pause
 *
 * Blue background means I am polling.
 * Red background means I am filtering.
 *
 * The Poll method is good when I want the key to trigger once, e.g., I am
 * making a game where I move in discrete steps. Or I want a keystroke to do a
 * thing without repetition, like in this demo I want SPACE to switch modes --
 * if SPACE is trigger+pause I can easily switch modes, but if SPACE is
 * immediate rapid fire, it is hard to intentionally switch to the mode I want.
 *
 * The Filter method is good when I want fluid motion.
 *
 * Polling is usually what I want. It gives me a pause for free; I don't have to
 * code a refractory period myself.
 *
 * Use the filter method for controlling sprite movement.
 *
 * Filtering is lower-level than polling. When I filter, I also "pump events"
 * to update the event queue. Polling does that for me under the hood.
 *
 * Be careful with polling: I cannot poll more than once. The first time I do:
 *
 *      while(  SDL_PollEvent(&e)  ) { ... }
 *
 * It consumes all the events. So if I try to split this into two while loops,
 * any events I check for in the second while loop will never trigger because
 * they were all consumed when they were polled in the first while loop.
 * *******************************/
/* *************Transparency: draw on top of a background***************
 * I make two textures:
 * - background
 * - foreground
 * I set texture blend mode to SDL_BLENDMODE_BLEND.
 * I copy both textures to the screen.
 * Default mode is the second texture completes writes over the first.
 * Using BLEND, the alpha channel determines how the second texture blends
 * with the first.
 * Be careful: clear foreground texture at start of each render loop; it does
 * not automatically clean up old artwork.
 * To clear, just set the render draw color to any color with alpha=0, then call
 * SDL_RenderClear.
 * *******************************/
/* *************Random***************
 * Demo use of stdlib rand() for good-enough random.
 *
 * I wrote "rand.h" to give me these functions:
 *
 * rand_init():
 *      Seed the random number generator.
 *      Call once during setup.
 *
 * rand_pm(pm):
 *      Get a random value in the range -pm to +pm
 * *******************************/

#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include "window_info.h"
#include "rand.h"

int main(int argc, char *argv[])
{
    for(int i=0; i<argc; i++) puts(argv[i]);

    // Setup
    rand_init();                                                // seed rand()
    SDL_Init(SDL_INIT_VIDEO);
    WindowInfo wI;
    Uint32 window_flags = SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_GRABBED;
    wI.x=50; wI.y=50; wI.w=600; wI.h=400; wI.flags=window_flags;
    if (argc>1) wI.x = atoi(argv[1]);
    if (argc>2) wI.y = atoi(argv[2]);
    if (argc>3) wI.w = atoi(argv[3]);
    if (argc>4) wI.h = atoi(argv[4]);

    SDL_Window *win = SDL_CreateWindow(argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);

    if(  !SDL_RenderTargetSupported(ren)  )
    {
        printf("SDL_RenderTargetSupported: %s\n", SDL_RenderTargetSupported(ren) ? "true":"false");
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Texture *tex0 = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, wI.w, wI.h); // background
    SDL_Texture *tex1 = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, wI.w, wI.h); // square
    // Blend these textures (draw square on top of background)
    SDL_SetTextureBlendMode(tex0, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(tex1, SDL_BLENDMODE_BLEND);

    /* *************Square_scale must be float***************
    //  Why float? Calculations do 1/Square_scale.
    //  If Square_scale is an int, 1/Square_scale evaluates to 0.
     * *******************************/
    /* *************Bigger Square_scale --> smaller square***************
    //  Square size is based on window size.
    //  Square_scale is how much to divide the window size by.
    //  And square is only square if window is square.
     * *******************************/
    float Square_scale = 10;                                    // Big val --> small square
    enum {POLL, FILT} mode = POLL;                              // Demo polling vs filtering

    bool quit = false;
    while(quit == false)
    {
        // UI
        { // Poll SPACE to switch modes
            // Switch modes exclusively by polling SPACE.
            // Need that pause! Hard to switch modes with rapid fire.
            SDL_Event e;
            while(  SDL_PollEvent(&e)  )
            {
                if(e.type == SDL_KEYDOWN)
                {
                    switch(e.key.keysym.sym)
                    {
                        case SDLK_SPACE:
                            switch(mode)
                            {
                                case POLL: mode=FILT; break;
                                case FILT: mode=POLL; break;
                                default: break;
                            }
                            break;
                        default: break;
                    }
                    if(  mode == POLL  )
                    { // All polling must happen in this while loop.
                        switch(e.key.keysym.sym)
                        {
                            case SDLK_q: quit=true; break;
                            case SDLK_s:
                                         Square_scale++;        // Decrease size
                                         if(  Square_scale>20  )
                                             Square_scale=20;   // Clamp at w/20
                                         break;
                            case SDLK_w:
                                         Square_scale--;        // Increase size
                                         if(  Square_scale<2  )
                                             Square_scale=2;    // Clamp at w/2
                                         break;
                            default: break;
                        }
                    }
                }
            }
        }
        // UI
        if(  mode == FILT  )
        { // Filter events
            SDL_PumpEvents();                               // Update event queue
            const Uint8 *k = SDL_GetKeyboardState(NULL);
            if(  k[SDL_SCANCODE_Q]  ) quit=true;
            if(  k[SDL_SCANCODE_W]  )
            {
                Square_scale--;                             // Increase square size
                if(Square_scale<2) Square_scale = 2;        // Clamp at w/2
            }
            if(  k[SDL_SCANCODE_S]  )
            {
                Square_scale++;                             // Decrease square size
                if(Square_scale>20) Square_scale = 20;      // Clamp at w/20
            }
        }

        // Render
        SDL_SetRenderTarget(ren, tex0);
        { // Draw background
            switch(mode)
            {
                case POLL: SDL_SetRenderDrawColor(ren, 100, 100, 200, 255); break;
                case FILT: SDL_SetRenderDrawColor(ren, 200, 100, 100, 255); break;
                default: break;
            }
            SDL_RenderClear(ren);
        }
        SDL_SetRenderTarget(ren, tex1);
        { // Draw transparent background (also erases old artwork)
            SDL_SetRenderDrawColor(ren, 0,0,0,0);
            SDL_RenderClear(ren);
        }
        { // Draw white square
            SDL_SetRenderDrawColor(ren, 255,255,255,255);
            float s = Square_scale;                             // Rename Square_scale as s
            float rw = rand_pm(wI.w/100);                       // random in range +/- window w / 200
            float rh = rand_pm(wI.h/100);                       // random in range +/- window h / 200
            SDL_Rect r = {
                .x=(wI.w - rw/2)*(1-1/s)/2,
                .y=(wI.h - rh/2)*(1-1/s)/2,
                .w=(wI.w + rw)/s,
                .h=(wI.h + rh)/s
            };
            SDL_RenderDrawRect(ren, &r);
        }
        SDL_SetRenderTarget(ren, NULL);                         // render to screen:
        SDL_RenderCopy(ren, tex0, NULL, NULL);                  // - background
        SDL_RenderCopy(ren, tex1, NULL, NULL);                  // - square
        SDL_RenderPresent(ren);                                 // show on screen
        SDL_Delay(10);
    }

    // Shutdown
    SDL_DestroyTexture(tex0);
    SDL_DestroyTexture(tex1);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}
