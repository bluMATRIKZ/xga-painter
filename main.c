/*
xga-painter - My 4-bit color image paint program
Copyright (C) 2025 Connor Thomson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or 
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_WIDTH 256
#define MAX_HEIGHT 256

SDL_Color palette[16] = {
    {0x00, 0x00, 0x00, 255}, // 0 Black
    {0x80, 0x00, 0x00, 255}, // 1 Maroon
    {0x00, 0x80, 0x00, 255}, // 2 Green
    {0x80, 0x80, 0x00, 255}, // 3 Olive
    {0x00, 0x00, 0x80, 255}, // 4 Navy
    {0x80, 0x00, 0x80, 255}, // 5 Purple
    {0x00, 0x80, 0x80, 255}, // 6 Teal
    {0xC0, 0xC0, 0xC0, 255}, // 7 Silver
    {0x80, 0x80, 0x80, 255}, // 8 Gray
    {0xFF, 0x00, 0x00, 255}, // 9 Red
    {0x00, 0xFF, 0x00, 255}, // 10 Lime
    {0xFF, 0xFF, 0x00, 255}, // 11 Yellow
    {0x00, 0x00, 0xFF, 255}, // 12 Blue
    {0xFF, 0x00, 0xFF, 255}, // 13 Fuchsia
    {0x00, 0xFF, 0xFF, 255}, // 14 Aqua
    {0xFF, 0xFF, 0xFF, 255}  // 15 White
};

int map_key(SDL_Keycode code) {
    const char* keys = "qwertyuiopasdfgh";
    for (int i = 0; i < 16; i++) {
        if (code == keys[i]) return i;
    }
    return -1;
}

void save_xga_file(uint8_t pixels[MAX_HEIGHT][MAX_WIDTH], int width, int height, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "xga-1;\n%dx%d;\n", width, height);

    int total = width * height;
    int count = 0, last = pixels[0][0];

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int px = pixels[y][x];
            if (px == last) {
                count++;
            } else {
                fprintf(f, "%d:%d;\n", count, last);
                count = 1;
                last = px;
            }
        }
    }

    if (count > 0)
        fprintf(f, "%d:%d;\n", count, last);

    fclose(f);
}

int main() {
    int width, height;
    char filename[256];

    printf("Canvas width (max %d): ", MAX_WIDTH); scanf("%d", &width);
    printf("Canvas height (max %d): ", MAX_HEIGHT); scanf("%d", &height);
    printf("File name: "); scanf("%s", filename);

    if (strlen(filename) < 4 || strcmp(filename + strlen(filename) - 4, ".xga") != 0)
        strcat(filename, ".xga");

    if (width <= 0 || height <= 0 || width > MAX_WIDTH || height > MAX_HEIGHT) {
        printf("Invalid size.\n"); return 1;
    }

    uint8_t pixels[MAX_HEIGHT][MAX_WIDTH];
    memset(pixels, 15, sizeof(pixels));

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow("XGA Painter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    int zoom = WINDOW_WIDTH / width;
    if (WINDOW_HEIGHT / height < zoom) zoom = WINDOW_HEIGHT / height;
    int offsetX = (WINDOW_WIDTH - width * zoom) / 2;
    int offsetY = (WINDOW_HEIGHT - height * zoom) / 2;

    int brush = 0;
    int mdown_l = 0, mdown_r = 0;
    int running = 1;

    save_xga_file(pixels, width, height, filename);

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;

            if (e.type == SDL_KEYDOWN) {
                int idx = map_key(e.key.keysym.sym);
                if (idx >= 0) brush = idx;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) mdown_l = 1;
                if (e.button.button == SDL_BUTTON_RIGHT) mdown_r = 1;
            }

            if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) mdown_l = 0;
                if (e.button.button == SDL_BUTTON_RIGHT) mdown_r = 0;
            }

            if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = (e.type == SDL_MOUSEMOTION) ? e.motion.x : e.button.x;
                int my = (e.type == SDL_MOUSEMOTION) ? e.motion.y : e.button.y;
                int cx = (mx - offsetX) / zoom;
                int cy = (my - offsetY) / zoom;
                if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                    if (mdown_l) pixels[cy][cx] = brush;
                    if (mdown_r) pixels[cy][cx] = 15;
                    save_xga_file(pixels, width, height, filename);
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren, palette[brush].r, palette[brush].g, palette[brush].b, 255);
        SDL_Rect b1 = {offsetX - 1, offsetY - 1, width * zoom + 2, height * zoom + 2};
        SDL_Rect b2 = {offsetX - 2, offsetY - 2, width * zoom + 4, height * zoom + 4};
        SDL_Rect b3 = {offsetX - 3, offsetY - 3, width * zoom + 6, height * zoom + 6};
        SDL_RenderDrawRect(ren, &b1);
        SDL_RenderDrawRect(ren, &b2);
        SDL_RenderDrawRect(ren, &b3);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                SDL_Color c = palette[pixels[y][x]];
                SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, 255);
                SDL_Rect rect = {offsetX + x * zoom, offsetY + y * zoom, zoom, zoom};
                SDL_RenderFillRect(ren, &rect);
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
