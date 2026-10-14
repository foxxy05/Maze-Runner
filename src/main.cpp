#include "Game.h"
#include <string>

int main(int argc, char* argv[]) {
    unsigned int initialSeed = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--seed" && i + 1 < argc) {
            try {
                initialSeed = static_cast<unsigned int>(std::stoul(argv[++i]));
            } catch (...) {
                initialSeed = 0;
            }
        }
    }

    Game game(initialSeed);
    game.run();

    return 0;
}
