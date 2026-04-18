#include "consts.h"
#include "memory.h"

// taken from: 
// https://www.reddit.com/r/cpp_questions/comments/zl9p9p/is_there_a_better_way_to_read_a_file_into_a/ 
std::vector<uint8_t> read_file(std::string filename) {
    if (std::ifstream source_file {
            filename, 
            std::ios::binary
        }; source_file) {

        return std::vector<uint8_t>(std::istreambuf_iterator<char>{source_file}, {});
    }

    std::cerr << "Unable to open file " << filename << "\n";

    return {};
}

int main() {
    std::cout << "Hello World!\n";

    // rom
    std::vector<uint8_t> bytes = read_file("drmario.gb");
    for (uint32_t i = 0; i < bytes.size(); i++) {
        write_byte(i, bytes[i]);
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

    // window
    window = SDL_CreateWindow(
        "GAMEBOY COLOR EMULATOR",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, 
        WIDTH, 
        HEIGHT,
        0
    );

    // check if window exists
    if (!window) {
        std::cerr << "Window not there: " << SDL_GetError() << "\n";
        return 1;
    }
    
    SDL_Event event;

    // run window. 
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}