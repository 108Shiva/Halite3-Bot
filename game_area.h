#pragma once

#include "hlt/game_map.hpp"
#include <unordered_set>

using namespace std;

class game_area
{
public:
	game_area();
	~game_area();

	void add_radius(hlt::GameMap& gameMap,const hlt::Position& pos, int radius);
	void add_initial(hlt::GameMap& gameMap, const hlt::Position& pos, int noPlayers);
	void remove_radius(hlt::GameMap& gameMap, const hlt::Position& pos, int radius);

	hlt::Position max_halite(hlt::GameMap& gameMap, int haliteRadius, int minHalite, int& maxHalite);

	int  get_halite(hlt::GameMap& gameMap) const;
	int  get_area() const;
	int  get_max_halite(hlt::GameMap& gameMap) const;

	hlt::Position max_halite(hlt::GameMap& gameMap, std::unordered_set<hlt::Position>& maxCollection, int& halite) const;
	hlt::Position max_halite(hlt::GameMap& gameMap, std::unordered_set<hlt::Position>& maxCollection, int& halite, const hlt::Position& start, int radius = 0) const;

private:

	unordered_set<hlt::Position>  _area;
};

