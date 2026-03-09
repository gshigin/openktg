#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include <SDL3/SDL.h>

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <bit>
#include <imgui.h>

#include <openktg/gentexture.h>

#include <cstdint>

constexpr uint64_t kScreenWidth{800};
constexpr uint64_t kScreenHeight{600};

constexpr uint64_t kScreenFps{60};
constexpr uint64_t kNsPerFrame = 1'000'000'000 / kScreenFps;

static bool SaveImage(GenTexture &img, const char *filename) {
  FILE *f = fopen(filename, "wb");
  if (!f)
    return false;

  // prepare header
  sU8 buffer[18];
  sSetMem(buffer, 0, sizeof(buffer));

  buffer[2] = 2;                // image type code 2 (RGB, uncompressed)
  buffer[12] = img.XRes & 0xff; // width (low byte)
  buffer[13] = img.XRes >> 8;   // width (high byte)
  buffer[14] = img.YRes & 0xff; // height (low byte)
  buffer[15] = img.YRes >> 8;   // height (high byte)
  buffer[16] = 32;              // pixel size (bits)

  // write header
  if (!fwrite(buffer, sizeof(buffer), 1, f)) {
    fclose(f);
    return false;
  }

  // write image data
  sU8 *lineBuf = new sU8[img.XRes * 4];
  for (sInt y = 0; y < img.YRes; y++) {
    const Pixel *in = &img.Data[y * img.XRes];

    // convert a line of pixels (as simple as possible - no gamma correction
    // etc.)
    for (sInt x = 0; x < img.XRes; x++) {
      lineBuf[x * 4 + 0] = in->b >> 8;
      lineBuf[x * 4 + 1] = in->g >> 8;
      lineBuf[x * 4 + 2] = in->r >> 8;
      lineBuf[x * 4 + 3] = in->a >> 8;

      in++;
    }

    // write to file
    if (!fwrite(lineBuf, img.XRes * 4, 1, f)) {
      fclose(f);
      return false;
    }
  }

  fclose(f);
  return true;
}

static GenTexture LinearGradient(sU32 startCol, sU32 endCol) {
  GenTexture tex;

  tex.Init(2, 1);
  tex.Data[0].Init(startCol);
  tex.Data[1].Init(endCol);

  return tex;
}

class timer {
public:
  timer() = default;

  void start() noexcept {
    started_ = true;
    paused_ = false;
    start_ticks_ = SDL_GetTicksNS();
    paused_ticks_ = 0;
  }

  void stop() noexcept {
    started_ = false;
    paused_ = false;
    start_ticks_ = 0;
    paused_ticks_ = 0;
  }

  void pause() noexcept {
    if (started_ && !paused_) {
      paused_ = true;
      paused_ticks_ = SDL_GetTicksNS() - start_ticks_;
      start_ticks_ = 0;
    }
  }

  void unpause() noexcept {
    if (started_ && paused_) {
      paused_ = false;
      start_ticks_ = SDL_GetTicksNS() - paused_ticks_;
      paused_ticks_ = 0;
    }
  }

  [[nodiscard]] uint64_t get_ticks_ns() const noexcept {
    if (!started_)
      return 0;
    return paused_ ? paused_ticks_ : (SDL_GetTicksNS() - start_ticks_);
  }

  [[nodiscard]] bool is_started() const noexcept { return started_; }
  [[nodiscard]] bool is_paused() const noexcept { return started_ && paused_; }

private:
  uint64_t start_ticks_{0};
  uint64_t paused_ticks_{0};

  bool paused_{false};
  bool started_{false};
};

void game_loop(SDL_Window *window, SDL_Renderer *renderer,
               ImGuiIO &io) noexcept {
  // imgui state
  bool show_demo_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  bool quit = false;
  SDL_Event e;
  timer cap_timer;

  // simple noise test texture
  InitTexgen();
  GenTexture noise;
  noise.Init(256, 256);
  noise.Noise(LinearGradient(0xff000000, 0xffffffff), 2, 2, 6, 0.5f, 123,
              GenTexture::NoiseDirect | GenTexture::NoiseBandlimit |
                  GenTexture::NoiseNormalize);

  SaveImage(noise, "noise.tga");

  SDL_Texture *texture = nullptr;

  if (SDL_Surface *surface =
          SDL_CreateSurface(256, 256, SDL_PIXELFORMAT_RGBA64);
      surface != nullptr) {
    std::memcpy(surface->pixels, noise.Data, 256 * 256 * 8);

    texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_DestroySurface(surface);
  } else {
    SDL_Log("Whoops: %s", SDL_GetError());
  }

  if (texture == nullptr) {
    SDL_Log("Whoops: %s", SDL_GetError());
  }

  while (!quit) {
    cap_timer.start();

    while (SDL_PollEvent(&e)) {
      ImGui_ImplSDL3_ProcessEvent(&e);
      if (e.type == SDL_EVENT_QUIT)
        quit = true;
      if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          e.window.windowID == SDL_GetWindowID(window))
        quit = true;
    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Windows
    {
      ImGui::Begin("Texture");

      // ImGui::PushItemWidth(-ImGui::GetFontSize() * 15);
      ImDrawList *draw_list = ImGui::GetWindowDrawList();

      const ImVec2 p = ImGui::GetCursorScreenPos();
      float x = p.x + 4.0f;
      float y = p.y + 4.0f;

      // TEXTURE ROTATION PROBLEM!
      draw_list->AddImage(ImTextureRef(std::bit_cast<ImTextureID>(texture)),
                          ImVec2(x + 256, y + 256), ImVec2(x, y));

      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y,
                                clear_color.z, clear_color.w);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);

    const uint64_t frameNs = cap_timer.get_ticks_ns();
    if (frameNs < kNsPerFrame) [[likely]] {
      SDL_DelayNS(kNsPerFrame - frameNs);
    }
  }
}

int main(int argc, char *args[]) {
  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  if (SDL_CreateWindowAndRenderer("OpenKTG Demo", kScreenWidth, kScreenHeight,
                                  0, &window, &renderer) == false) {
    SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
    SDL_Quit();
    return 2;
  }

  // imgui setup
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup scaling
  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(
      main_scale); // Bake a fixed style scale. (until we have a solution for
                   // dynamic style scaling, changing this requires resetting
                   // Style + calling this again)
  style.FontScaleDpi =
      main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true
                  // makes this unnecessary. We leave both here for
                  // documentation purpose)

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);

  // loop
  game_loop(window, renderer, io);

  // deinit
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}