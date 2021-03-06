#pragma once

#include "game_map.hpp"
#include "player.hpp"
#include "types.hpp"
#include "game_settings.h"
#include <vector>
#include <iostream>

namespace hlt {
    struct Game {
        int turn_number;
        PlayerId my_id;
        std::vector<std::shared_ptr<Player>> players;
        std::shared_ptr<Player> me;
        std::unique_ptr<GameMap> game_map;
		std::shared_ptr<GameSettings> game_settings;

        Game();
        void ready(const std::string& name);
        void update_frame();
        bool end_turn(const std::vector<Command>& commands);
    };
}
