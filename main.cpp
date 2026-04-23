#include "consts.h"
#include "cpu.h"
#include "display.h"
#include "joypad.h"
#include "memory.h"
#include "ppu.h"

using namespace std::chrono_literals;

uint8_t cgb_mode;

void update_button(SDL_Keycode key, bool pressed) {
    bool request_interrupt = false;

    switch (key) {
        case SDLK_RIGHT:
            request_interrupt = set_button(Button::RIGHT, pressed);
            break;
        case SDLK_LEFT:
            request_interrupt = set_button(Button::LEFT, pressed);
            break;
        case SDLK_UP:
            request_interrupt = set_button(Button::UP, pressed);
            break;
        case SDLK_DOWN:
            request_interrupt = set_button(Button::DOWN, pressed);
            break;
        case SDLK_z:
            request_interrupt = set_button(Button::A, pressed);
            break;
        case SDLK_x:
            request_interrupt = set_button(Button::B, pressed);
            break;
        case SDLK_RSHIFT:
            request_interrupt = set_button(Button::SELECT, pressed);
            break;
        case SDLK_RETURN:
            request_interrupt = set_button(Button::START, pressed);
            break;
        default:
            break;
    }

    if (request_interrupt) {
        write_byte(0xFF0F, read_byte(0xFF0F) | (1 << 4));
    }
}

// taken from: 
// https://www.reddit.com/r/cpp_questions/comments/zl9p9p/is_there_a_better_way_to_read_a_file_into_a/ 
std::vector<uint8_t> read_file(std::string filename) {
    std::ifstream source_file{filename};
    if (source_file) {
        return std::vector<uint8_t>(std::istreambuf_iterator<char>{source_file}, {});
    }

    std::cerr << "Unable to open file " << filename << "\n";

    return {};
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom.gb> [--headless]\n";
        return 1;
    }

    std::string rom_path = argv[1];
    bool headless = argc >= 3 && std::string(argv[2]) == "--headless";

    // rom
    std::vector<uint8_t> bytes = read_file(rom_path);
    if (bytes.empty()) {
        return 1;
    }

    reset_memory();
    for (uint32_t i = 0; i < bytes.size(); i++) {
        write_byte(i, bytes[i]);
    }
    reset_cpu();

    if (headless) {
        while (true) {
            run();
        }
    }

    // Code used from: https://wiki.libsdl.org/SDL3/SDL_CreateWindow 

    // create the window
    SDL_Window *window; 
    bool done = false; // done is true when the user exits the window.

    // inits SDL Video Subsystem
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL failed to initialize: " << SDL_GetError() << "\n";
        return 1;
    }
    std::cout << "creating window \n";

    // window
    window = SDL_CreateWindow(
        "GAMEBOY COLOR EMULATOR",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WIDTH * 4,
        HEIGHT * 4,
        SDL_WINDOW_RESIZABLE
    );

    // check if window exists
    if (!window) {
        std::cerr << "Window not there: " << SDL_GetError() << "\n";
        return 1;
    }

    if (!init_display(window)) {
        std::cout << "couldn't display window\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "made window\n";
    
    SDL_Event event;
    // run window. 
    while (!done) {
        auto t_cur = std::chrono::high_resolution_clock::now();

        // 1. Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                update_button(event.key.keysym.sym, true);
            }
            else if (event.type == SDL_KEYUP) {
                update_button(event.key.keysym.sym, false);
            }
        }

        // 2. Run ONE FRAME worth of cycles
        uint32_t cur_cyc = 0;
        uint32_t dots = 0;
        while (cur_cyc < CYC) {
            uint8_t mcycles = run();
            dots += run_ppu(mcycles);
            cur_cyc += mcycles;
        }


        // 3. Render
        render_display(frame_buffer);
        reset();

        // 4. Frame timing (limit speed)
        while (std::chrono::high_resolution_clock::now() - t_cur < FRAME_LEN * 1ms) {}
    }

    destroy_display();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
