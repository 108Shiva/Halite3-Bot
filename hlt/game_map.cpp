#include "game_map.hpp"
#include "input.hpp"
#include <string>

void hlt::GameMap::_update() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            cells[y][x].ship.reset();
        }
    }

    int update_count;
    hlt::get_sstream() >> update_count;

    for (int i = 0; i < update_count; ++i) {
        int x;
        int y;
        int halite;
        hlt::get_sstream() >> x >> y >> halite;
        cells[y][x].halite = halite;
    }
}

std::unique_ptr<hlt::GameMap> hlt::GameMap::_generate() {
    std::unique_ptr<hlt::GameMap> map = std::make_unique<GameMap>();

    hlt::get_sstream() >> map->width >> map->height;

    map->cells.resize((size_t)map->height);
    for (int y = 0; y < map->height; ++y) {
        auto in = hlt::get_sstream();

        map->cells[y].reserve((size_t)map->width);
        for (int x = 0; x < map->width; ++x) {
            hlt::Halite halite;
            in >> halite;

            map->cells[y].push_back(MapCell(x, y, halite));
        }
    }

    return map;
}

bool hlt::GameMap::has_opponent(const Position& destination, const hlt::PlayerId& playerId)
{
	auto surronding = destination.get_surrounding_cardinals();

	for (auto pos : surronding)
	{
		if (at(pos)->is_occupied() && !at(pos)->is_occupied_by_myself(playerId))
		{
			return true;
		}
	}
	return false;
}

hlt::Direction hlt::GameMap::navigate(std::shared_ptr<hlt::Ship> ship, const Position& destination, std::mt19937& rng, const hlt::PlayerId& playerId, std::vector<Direction>& potential, bool endGame, bool dropOff, bool checkOpposition, bool &noMove)
{
	auto haliteForMove = at(ship->position)->halite / 10;
	if (!at(ship->position)->has_structure() && ship->halite < haliteForMove)
	{
		noMove = true;
		return hlt::Direction::STILL;
	}

	noMove = false;

	/*
	Direction dir = naive_navigate(ship, destination);
	*/
	// get_unsafe_moves normalizes for us
	Direction dir = Direction::STILL;
	std::vector<Direction> possibleDirs;
	std::vector<Direction> extraDirs;

	//check if two ships need to swap
	for (auto direction : get_unsafe_moves(ship->position, destination) ) 
	{
		Position target_pos = ship->position.directional_offset(direction);
		if (target_pos == destination && endGame)
		{
			possibleDirs.emplace_back(direction);
			break;
		}
		else if (dropOff && target_pos == destination)
		{
			if (!at(target_pos)->is_occupied_by_myself(playerId))
			{
				possibleDirs.emplace_back(direction);
			}
			else
			{
				potential.emplace_back(direction); // we were here
			}
		}
		else if (!at(target_pos)->is_occupied())
		{
			possibleDirs.emplace_back(direction);
		}
		else 
		{
			if (at(target_pos)->is_occupied_by_myself(playerId))
			{
				potential.emplace_back(direction); // we were here
			}
			else if ((dropOff || endGame) && calculate_distance(target_pos, destination) == 1)
			{
				extraDirs.emplace_back(direction);
			}
		}
	}
	
	bool checkOpponent = true;
	if ((dropOff || endGame) && calculate_distance(ship->position, destination) <= 3)
	{
		checkOpponent = false;
	}

	if (ship->halite < 100)
	{
		checkOpponent = false;
	}

	if (checkOpposition == false)
	{
		checkOpponent = false;
	}

	if (checkOpponent)
	{
		auto possibleCopy = possibleDirs;
		possibleDirs.clear();
		for (auto x : possibleCopy)
		{
			auto newPos = ship->position.directional_offset(x);
			if (!has_opponent(newPos, playerId))
			{
				possibleDirs.push_back(x);
			}
		}
	}
	if (possibleDirs.empty())
	{
		possibleDirs = extraDirs;
	}
	
	if (possibleDirs.empty())
	{
		for (auto direction : get_alt_unsafe_moves(ship->position, destination))
		{
			Position target_pos = ship->position.directional_offset(direction);
			if (!at(target_pos)->is_occupied())
			{
				if (!checkOpponent || !has_opponent(target_pos, playerId))
				{
					if (!dropOff || at(target_pos)->halite < 10) // only move a drop off ship if we need to other wise don't
					{
						potential.emplace_back(direction);
					}
				}
			}
		}
	}
	
	if (!possibleDirs.empty())
	{
		Direction nextDir = possibleDirs[0];
		int halite = at(ship->position.directional_offset(nextDir))->halite;

		for( auto d : possibleDirs)
		{ 
			int h = at(ship->position.directional_offset(d))->halite;

			if (dropOff)
			{
				if (h < halite)
				{
					halite = h;
					nextDir = d;
				}
			}
			else
			{
				if (h > halite)
				{
					halite = h;
					nextDir = d;
				}
			}
		}

		dir = nextDir;
		Position target_pos = ship->position.directional_offset(dir);
		at(ship->position)->mark_safe();
		at(target_pos)->mark_unsafe(ship);
	}
	/*
	
	if (dir == Direction::STILL)
	{
		// pick a random direction
		// 50% we stay were we are...
		int move = rng() % 2;
		if (move == 1)
		{
			std::vector<Direction> directions{ Direction::NORTH, Direction::SOUTH, Direction::EAST, Direction::WEST };

			while (dir == Direction::STILL && !directions.empty())
			{
				int index = rng() % directions.size();
				Direction random_direction = directions[index];

				log::log("Ship " + std::to_string(ship->id) + "  trying direction " + std::string(1, (char)random_direction));

				if (at(ship->position.directional_offset(random_direction))->is_empty())
					dir = random_direction;
				else
				{
					directions.erase(directions.begin() + index);
				}
			}
			log::log("Ship " + std::to_string(ship->id) + "  Direction still now " + std::string(1, (char)dir));

			if (dir != Direction::STILL)
			{
				at(ship->position)->mark_safe();
				at(ship->position.directional_offset(dir))->mark_unsafe(ship);
			}
		}
		else
		{
			log::log("Ship staying here random");
		}
	}*/
	return dir;
}
