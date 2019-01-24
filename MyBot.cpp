#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"
#include "ship_container.hpp"
#include <string>
#include <random>
#include <ctime>
#include <unordered_set>
#include "game_strategy.h"


// version 55 reaches 152 rank
using namespace std;
using namespace hlt;

int main(int argc, char* argv[]) {
    unsigned int rng_seed;
    if (argc > 1) 
	{
        rng_seed = static_cast<unsigned int>(stoul(argv[1]));
    } 
	else 
	{
        rng_seed = static_cast<unsigned int>(time(nullptr));
    }
    
	mt19937 rng(rng_seed);

    Game game;
    // At this point "game" variable is populated with initial map data.
    // This is a good place to do computationally expensive start-up pre-processing.
    // As soon as you call "ready" function below, the 2 second per turn timer will start.
    game.ready("MyCppBot");

    log::log("Successfully created bot! My Player ID is " + to_string(game.my_id) + ". Bot rng seed is " + to_string(rng_seed) + ".");

	std::unique_ptr<GameStrategy> strategy;
	
	strategy = make_unique<GameStrategy>(game, rng);

	for (;;) 
	{
        game.update_frame();

		if (!game.end_turn(strategy->GetCommandQueue()))
		{
			break;
		}       
    }

    return 0;
}
