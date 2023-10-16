#include "main.hpp"

int main(int argv, char **args) {

  try {
    std::cout << "Starting App Tho\n";
    GameEngine::Game game;
    game.run();
  } catch (const std::exception &e) {

    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}