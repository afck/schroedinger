#include <cmath>
#include <complex>
#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Bencher.h"
#include "Wave.h"

using namespace std;

inline Uint32 rgbToColor(double r, double g, double b) {
  return ((static_cast<Uint32>(min(max(r * 255, 0.0), 255.0))) << 16) +
         ((static_cast<Uint32>(min(max(g * 255, 0.0), 255.0))) << 8) +
         (static_cast<Uint32>(min(max(b * 255, 0.0), 255.0)));
}

inline Uint32 hsvToColor(double h, double s, double v) {
  if (s == 0) {
    return rgbToColor(v, v, v);
  }
  h = h * 3.0 / M_PI + 3.0;
  int i = floor(h);
  double f = h - i;
  double p = v * (1 - s);
  double q = v * (1 - s * f);
  double t = v * (1 - s * (1 - f));
  switch (i) {
  case 0:
    return rgbToColor(v, t, p);
  case 1:
    return rgbToColor(q, v, p);
  case 2:
    return rgbToColor(p, v, t);
  case 3:
    return rgbToColor(p, q, v);
  case 4:
    return rgbToColor(t, p, v);
  default:
    return rgbToColor(v, p, q);
  }
}

Uint32 toColor0(complex<double> c, double p) {
  return hsvToColor(arg(c), max(0.0, 1.0 - p), min(abs(c) * 0.5 + p, 1.0));
}

Uint32 toColor1(complex<double> c, double p) {
  c *= 0.5;
  return rgbToColor(c.real() + 0.5, c.imag() + 0.5, min(p, 1.0));
}

void addBump(Wave *wave, int x, int y, double scale, bool pot, bool psi) {
  const Uint8 *keys = SDL_GetKeyboardState(0);
  const int size = keys[SDL_SCANCODE_SPACE] ? 20 : 6;
  const double weight =
      keys[SDL_SCANCODE_L] ? 0.1 : keys[SDL_SCANCODE_S] ? 1.0 : 0.3;
  const double theta = keys[SDL_SCANCODE_P] ? clock() * 0.00002 : 0;
  const complex<double> c = polar(2.0, theta);
  if (pot) {
    wave->addPotentialBump(x / scale, y / scale, weight, size);
  }
  if (psi) {
    wave->addBump(x / scale, y / scale, c * weight, size);
  }
}

int main(int argc, char *argv[]) {
  const int width = (argc > 1) ? stoi(argv[1]) : 256;
  const int height = (argc > 2) ? stoi(argv[2]) : 128;
  const double scale = (argc > 3) ? stof(argv[3]) : 2.0;
  const bool bench = argc > 4 && strcmp(argv[4], "bench") == 0;
  const int skipFrames = 5;

  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Init(SDL_INIT_VIDEO); // TODO: Handle error.
  SDL_CreateWindowAndRenderer(width * scale, height * scale, 0, &window,
                              &renderer);
  SDL_SetWindowTitle(window, "Schrödinger-Poisson equation");
  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, width, height);
  SDL_RenderSetScale(renderer, scale, scale);
  static Uint32 *pixels = new Uint32[height * width];
  Wave wave(width, height);
  Bencher bencher(bench);
  int colorf = 0;

  SDL_Event event;
  bool running = true;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_MOUSEMOTION:
        addBump(&wave, event.motion.x, event.motion.y, scale,
                event.motion.state & SDL_BUTTON_LMASK,
                event.motion.state & SDL_BUTTON_RMASK);
        break;
      case SDL_MOUSEBUTTONDOWN:
        addBump(&wave, event.motion.x, event.motion.y, scale,
                event.button.button == SDL_BUTTON_LMASK,
                event.button.button == SDL_BUTTON_RMASK);
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          running = false;
          break;
        case SDLK_c:
          colorf ^= 1;
          break;
        }
        break;
      case SDL_QUIT:
        running = false;
        break;
      }
    }
    wave.normalize();
    for (int i = 0; i < skipFrames; i++) {
      wave.evolve();
    }
    bencher.bench("Calculation");
    wave.draw(pixels, colorf == 0 ? toColor0 : toColor1);
    bencher.bench("Color coding");
    SDL_UpdateTexture(texture, nullptr, pixels, width * sizeof(Uint32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    bencher.bench("Rendering");
  }

  bencher.print();

  SDL_DestroyWindow(window);
  SDL_Quit();
  delete[] pixels;

  return 0;
}
