#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <psp2/ctrl.h>

#define SCREEN_W 480
#define SCREEN_H 272
#define PLAYER_W 18
#define PLAYER_H 30
#define ENEMY_W 18
#define ENEMY_H 28

typedef struct {
    float x, y;
    int hp;
    int facing;
    int attack_timer;
    int invuln;
} Fighter;

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_GameController *pad;
static SDL_Surface *scott_surface;
static SDL_Texture *scott_texture;
static Fighter player = {92.0f, 176.0f, 100, 1, 0, 0};
static Fighter enemy  = {330.0f, 178.0f, 100, -1, 0, 0};
static int score = 0;
static bool running = true;
static bool game_over = false;
static Uint32 last_tick = 0;

static void color(Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}
static void rect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b) {
    color(r,g,b);
    SDL_Rect q = {x,y,w,h};
    SDL_RenderFillRect(renderer, &q);
}
static void outline(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b) {
    color(r,g,b);
    SDL_Rect q = {x,y,w,h};
    SDL_RenderDrawRect(renderer, &q);
}

/* Tiny 3x5 bitmap font for the HUD. */
static const unsigned char glyphs[37][5] = {
    {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},
    {7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},{7,5,7,5,7},{7,5,7,1,7},
    {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},{7,4,6,4,7},
    {7,4,6,4,4},{3,4,5,5,3},{5,5,7,5,5},{7,2,2,2,7},{1,1,1,5,2},
    {5,5,6,5,5},{4,4,4,4,7},{5,7,7,5,5},{5,7,7,7,5},{2,5,5,5,2},
    {6,5,6,4,4},{2,5,5,3,1},{6,5,6,5,5},{3,4,2,1,6},{7,2,2,2,2},
    {5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},{5,5,2,2,2},
    {7,1,2,4,7},
    {0,0,0,0,0}
};
static int glyph_index(char c) {
    if (c >= '0' && c <= '9') return c-'0';
    if (c >= 'A' && c <= 'Z') return 10+c-'A';
    return 36;
}
static void text3(int x, int y, const char *s, int scale, Uint8 r, Uint8 g, Uint8 b) {
    color(r,g,b);
    for (; *s; ++s, x += 4*scale) {
        int gi = glyph_index(*s);
        for (int row=0; row<5; ++row)
            for (int col=0; col<3; ++col)
                if (glyphs[gi][row] & (1 << (2-col))) {
                    SDL_Rect p = {x+col*scale,y+row*scale,scale,scale};
                    SDL_RenderFillRect(renderer,&p);
                }
    }
}
static void draw_background(void) {
    rect(0,0,SCREEN_W,SCREEN_H,20,19,39);
    /* Distant apartment blocks */
    rect(18,48,72,104,34,37,66); rect(105,30,92,122,43,39,72);
    rect(218,55,80,97,32,42,70); rect(316,37,102,115,47,36,66);
    rect(426,62,48,90,35,43,68);
    for (int x=25; x<470; x+=22) {
        for (int y=48; y<137; y+=22) {
            rect(x,y,7,9,225,119,91);
            rect(x+10,y,5,9,83,173,188);
        }
    }
    /* Neon signs */
    rect(125,68,48,18,219,53,115); outline(125,68,48,18,255,166,195);
    text3(130,74,"OPEN",2,255,225,222);
    rect(330,83,55,18,43,185,194); outline(330,83,55,18,147,244,240);
    text3(337,89,"CLUB",2,220,255,245);
    /* Road and sidewalk */
    rect(0,151,480,30,75,69,89);
    rect(0,181,480,91,39,37,54);
    rect(0,178,480,4,238,173,111);
    for (int x=0; x<480; x+=48) rect(x,212,25,2,105,95,103);
    rect(0,246,480,2,74,66,83);
    /* Street lights */
    rect(48,92,3,64,120,124,151); rect(38,88,24,4,250,197,120);
    rect(405,89,3,65,120,124,151); rect(395,85,24,4,250,197,120);
}
static void draw_player(void) {
    int x=(int)player.x, y=(int)player.y;
    rect(x-3,y+27,24,4,20,19,31);
    if (scott_texture) {
        SDL_Rect dst={x-5,y-10,30,40};
        SDL_RendererFlip flip=player.facing<0?SDL_FLIP_HORIZONTAL:SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer,scott_texture,NULL,&dst,0,NULL,flip);
    } else {
        rect(x+2,y+22,6,7,35,39,65); rect(x+10,y+22,6,7,35,39,65);
        rect(x+2,y+12,14,12,49,113,180);
        rect(x+3,y+3,12,10,242,190,151);
        rect(x+2,y+1,14,5,47,34,38);
    }
    if (player.attack_timer>0) {
        int ax = player.facing>0 ? x+18 : x-15;
        rect(ax,y+12,13,7,255,225,91);
        rect(ax+3,y+9,7,13,255,244,164);
    }
}
static void draw_enemy(void) {
    if (enemy.hp<=0) return;
    int x=(int)enemy.x, y=(int)enemy.y;
    rect(x-3,y+26,24,4,20,19,31);
    rect(x+2,y+21,6,7,44,39,52); rect(x+11,y+21,6,7,44,39,52);
    rect(x+1,y+10,16,13,157,56,75);
    rect(x-1,y+12,4,9,221,129,117); rect(x+15,y+12,4,9,221,129,117);
    rect(x+3,y+2,12,10,225,159,133);
    rect(x+2,y,14,5,34,29,43); rect(x+5,y+6,2,2,35,32,42);
}
static void draw_hud(void) {
    rect(0,0,480,29,12,12,25);
    text3(12,7,"NEW CHAOS",2,255,223,170);
    text3(180,7,"SCORE",2,205,215,235);
    char buf[16]; snprintf(buf,sizeof(buf),"%d",score);
    text3(224,7,buf,2,255,255,255);
    text3(310,7,"HP",2,255,184,185);
    rect(335,7,105,9,66,38,55);
    rect(337,9,player.hp,5,232,75,94);
    if (enemy.hp>0) {
        text3(350,37,"RIVAL",2,255,207,194);
        rect(350,49,92,7,65,37,54);
        rect(352,51,enemy.hp*0.88f,3,229,94,110);
    }
}
static void reset_game(void) {
    player=(Fighter){92.0f,176.0f,100,1,0,0};
    enemy=(Fighter){330.0f,178.0f,100,-1,0,0};
    score=0; game_over=false;
}
static void try_attack(bool kick) {
    if (game_over || player.attack_timer>0) return;
    player.attack_timer=kick?18:12;
    float distance = fabsf((enemy.x+ENEMY_W/2)-(player.x+PLAYER_W/2));
    bool facing_enemy = (enemy.x-player.x)*player.facing >= -2.0f;
    if (enemy.hp>0 && distance<43.0f && facing_enemy &&
        fabsf(enemy.y-player.y)<18.0f) {
        int damage=kick?24:14;
        enemy.hp-=damage;
        if (enemy.hp<0) enemy.hp=0;
        score += damage*10;
        enemy.x += player.facing*(kick?13:8);
    }
}
static void update_game(float dt, const Uint8 *keys) {
    (void)dt;
    if (player.attack_timer>0) player.attack_timer--;
    if (player.invuln>0) player.invuln--;
    if (enemy.hp<=0) { game_over=true; return; }
    float speed=2.0f;
    if (keys[SDL_SCANCODE_LEFT]) { player.x-=speed; player.facing=-1; }
    if (keys[SDL_SCANCODE_RIGHT]) { player.x+=speed; player.facing=1; }
    if (keys[SDL_SCANCODE_UP]) player.y-=speed;
    if (keys[SDL_SCANCODE_DOWN]) player.y+=speed;
    if (player.x<8) player.x=8;
    if (player.x>454) player.x=454;
    if (player.y<154) player.y=154;
    if (player.y>226) player.y=226;

    if (enemy.hp>0) {
        float dx=player.x-enemy.x;
        if (fabsf(dx)>24) enemy.x += dx>0?0.55f:-0.55f;
        enemy.facing=dx>0?1:-1;
        if (fabsf(dx)<27 && fabsf(player.y-enemy.y)<13 && player.invuln==0) {
            player.hp-=1; player.invuln=38;
            if (player.hp<0) player.hp=0;
            if (player.hp==0) game_over=true;
        }
    }
}
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_TIMER)!=0) return 1;
    window=SDL_CreateWindow("New Chaos",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                            960,544,SDL_WINDOW_SHOWN);
    if (!window) { SDL_Quit(); return 1; }
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    if (!renderer) renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
    if (!renderer) { SDL_DestroyWindow(window); SDL_Quit(); return 1; }
    SDL_RenderSetLogicalSize(renderer,SCREEN_W,SCREEN_H);
    SDL_RenderSetIntegerScale(renderer,SDL_TRUE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
    scott_surface=SDL_LoadBMP("app0:/assets/scott_idle.bmp");
    if (scott_surface) {
        /* Convert the sprite to RGBA and remove green-screen pixels, including
           slightly off-green shades that caused a visible green box. */
        SDL_Surface *rgba=SDL_ConvertSurfaceFormat(scott_surface,SDL_PIXELFORMAT_RGBA32,0);
        SDL_FreeSurface(scott_surface);
        scott_surface=rgba;
        if (scott_surface && SDL_LockSurface(scott_surface)==0) {
            for (int py=0; py<scott_surface->h; ++py) {
                Uint32 *row=(Uint32 *)((Uint8 *)scott_surface->pixels+py*scott_surface->pitch);
                for (int px=0; px<scott_surface->w; ++px) {
                    Uint8 r,g,b,a;
                    SDL_GetRGBA(row[px],scott_surface->format,&r,&g,&b,&a);
                    if (g>150 && (int)g>(int)r*6/5 && (int)g>(int)b*6/5)
                        row[px]=SDL_MapRGBA(scott_surface->format,r,g,b,0);
                    else
                        row[px]=SDL_MapRGBA(scott_surface->format,r,g,b,255);
                }
            }
            SDL_UnlockSurface(scott_surface);
        }
        if (scott_surface) {
            scott_texture=SDL_CreateTextureFromSurface(renderer,scott_surface);
            if (scott_texture) SDL_SetTextureBlendMode(scott_texture,SDL_BLENDMODE_BLEND);
            SDL_FreeSurface(scott_surface);
            scott_surface=NULL;
        }
    }
    for (int i=0;i<SDL_NumJoysticks();i++) {
        if (SDL_IsGameController(i)) { pad=SDL_GameControllerOpen(i); if (pad) break; }
    }
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    static unsigned int previous_vita_buttons = 0;
    last_tick=SDL_GetTicks();
    while (running) {
        Uint32 now=SDL_GetTicks();
        float dt=(now-last_tick)/16.6667f; last_tick=now;
        if (dt>3.0f) dt=3.0f;
        SDL_Event e;
        bool punch=false,kick=false;
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_QUIT) running=false;
            if (e.type==SDL_KEYDOWN && !e.key.repeat) {
                if (e.key.keysym.sym==SDLK_ESCAPE) running=false;
                if (e.key.keysym.sym==SDLK_z) punch=true;
                if (e.key.keysym.sym==SDLK_x) kick=true;
                if (e.key.keysym.sym==SDLK_RETURN && game_over) reset_game();
            }
            if (e.type==SDL_CONTROLLERBUTTONDOWN) {
                if (e.cbutton.button==SDL_CONTROLLER_BUTTON_A) punch=true;
                if (e.cbutton.button==SDL_CONTROLLER_BUTTON_B) kick=true;
                if (e.cbutton.button==SDL_CONTROLLER_BUTTON_START && game_over) reset_game();
                if (e.cbutton.button==SDL_CONTROLLER_BUTTON_BACK) running=false;
            }
        }
        /* Read the Vita's built-in controls directly. SDL controller mappings
           are not guaranteed to exist on every Vita SDL2 build. */
        SceCtrlData vita_pad;
        memset(&vita_pad, 0, sizeof(vita_pad));
        sceCtrlPeekBufferPositive(0, &vita_pad, 1);
        unsigned int vita_buttons = vita_pad.buttons;
        if (vita_buttons & SCE_CTRL_CROSS) punch=true;
        if (vita_buttons & SCE_CTRL_CIRCLE) kick=true;
        if ((vita_buttons & SCE_CTRL_START) && !(previous_vita_buttons & SCE_CTRL_START) && game_over) reset_game();
        if ((vita_buttons & SCE_CTRL_SELECT) && !(previous_vita_buttons & SCE_CTRL_SELECT)) running=false;
        previous_vita_buttons = vita_buttons;

        const Uint8 *keys=SDL_GetKeyboardState(NULL);
        bool left=keys[SDL_SCANCODE_LEFT] || (vita_buttons & SCE_CTRL_LEFT) || vita_pad.lx < 64;
        bool right=keys[SDL_SCANCODE_RIGHT] || (vita_buttons & SCE_CTRL_RIGHT) || vita_pad.lx > 192;
        bool up=keys[SDL_SCANCODE_UP] || (vita_buttons & SCE_CTRL_UP) || vita_pad.ly < 64;
        bool down=keys[SDL_SCANCODE_DOWN] || (vita_buttons & SCE_CTRL_DOWN) || vita_pad.ly > 192;
        if (pad) {
            if (SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_DPAD_LEFT)) left=true;
            if (SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) right=true;
            if (SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_DPAD_UP)) up=true;
            if (SDL_GameControllerGetButton(pad,SDL_CONTROLLER_BUTTON_DPAD_DOWN)) down=true;
        }
        /* Mirror controller directions into the keyboard state-independent movement. */
        static Uint8 movement[SDL_NUM_SCANCODES];
        movement[SDL_SCANCODE_LEFT]=left; movement[SDL_SCANCODE_RIGHT]=right;
        movement[SDL_SCANCODE_UP]=up; movement[SDL_SCANCODE_DOWN]=down;
        update_game(dt,movement);
        if (punch) try_attack(false);
        if (kick) try_attack(true);

        draw_background();
        draw_enemy();
        if (player.invuln==0 || (player.invuln/4)%2==0) draw_player();
        draw_hud();
        if (game_over) {
            rect(115,104,250,62,13,12,27);
            outline(115,104,250,62,248,184,118);
            text3(165,117,enemy.hp<=0?"STAGE CLEAR":"KNOCKED OUT",2,255,225,177);
            text3(145,143,"PRESS START",2,228,235,255);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    if (scott_texture) SDL_DestroyTexture(scott_texture);
    if (pad) SDL_GameControllerClose(pad);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
