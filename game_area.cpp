#include "game_area.h"

using namespace hlt;

game_area::game_area()
{
}


game_area::~game_area()
{
}

void game_area::add_initial(hlt::GameMap& gameMap, const hlt::Position& pos, int noPlayers)
{
	int height = gameMap.height;
	int width = gameMap.width;

	if (noPlayers == 4)
	{
		height /= 2;
		width /= 2;
	}
	else
	{
		width /= 2;
	}

	//rectange x1,y1 to x2,y2
	Position p(0, 0);
	Position q(width - 1, height - 1);

	if (pos.x > width - 1)
	{
		p.x += width;
		q.x += width;
	}
	
	if ( noPlayers == 4)
	{
		if (pos.y > height - 1)
		{
			p.y += height;
			q.y += height;
		}
	}

	// now we need to add rectangle between p and q.
	for (int x = p.x; x <= q.x; x++)
	{
		for (int y = p.y; y <= q.y; y++)
		{
			Position pos(x, y);
			pos = gameMap.normalize(pos);
			auto cell = gameMap.at(pos);
			if (cell != nullptr)
			{
				if (!cell->has_structure())
				{
					_area.insert(pos);
				}
			}
		}
	}
}
void game_area::remove_radius(hlt::GameMap& gameMap, const hlt::Position& pos, int radius)
{
	Position center(pos);

	center = gameMap.normalize(center);

	int current = 0;
	while (current < radius)
	{
		current++;
		//four sides N,E,S,W
		for (int i = 0; i < 4; i++)
		{
			int xDiff = 0;
			int yDiff = 0;
			int xStart = center.x;
			int yStart = center.y;
			if (i == 0)
			{
				xDiff = 1;
				yDiff = 1;
				yStart -= current;
			}
			else  if (i == 1)
			{
				xDiff = -1;
				yDiff = 1;
				xStart += current;
			}
			else  if (i == 2)
			{
				xDiff = -1;
				yDiff = -1;
				yStart += current;
			}
			else  if (i == 3)
			{
				xDiff = 1;
				yDiff = -1;
				xStart -= current;
			}

			for (int j = 0; j < current; j++)
			{
				Position check(xStart + (j*xDiff), yStart + (j*yDiff));
				check = gameMap.normalize(check);;
				auto iter = _area.find(check);
				if (iter != _area.end())
				{
					_area.erase(iter);
				}
			}
		}
	}
}

void game_area::add_radius(hlt::GameMap& gameMap, const hlt::Position& pos, int radius)
{
	Position center(pos);

	center = gameMap.normalize(center);

	int current = 0;
	while (current < radius)
	{
		current++;
		//four sides N,E,S,W
		for (int i = 0; i < 4; i++)
		{
			int xDiff = 0;
			int yDiff = 0;
			int xStart = center.x;
			int yStart = center.y;
			if (i == 0)
			{
				xDiff = 1;
				yDiff = 1;
				yStart -= current;
			}
			else  if (i == 1)
			{
				xDiff = -1;
				yDiff = 1;
				xStart += current;
			}
			else  if (i == 2)
			{
				xDiff = -1;
				yDiff = -1;
				yStart += current;
			}
			else  if (i == 3)
			{
				xDiff = 1;
				yDiff = -1;
				xStart -= current;
			}

			for (int j = 0; j < current; j++)
			{
				Position check(xStart + (j*xDiff), yStart + (j*yDiff));
				check = gameMap.normalize(check);;
				auto cell = gameMap.at(check);
				if (cell != nullptr)
				{
					if (!cell->has_structure())
					{
						_area.insert(check);
					}
				}
			}
		}
	}
}

Position game_area::max_halite(hlt::GameMap& gameMap, int haliteRadius, int minHalite, int& maxHalite)
{
	Position ret(0, 0);

	maxHalite = 0;
	for (auto& pos : _area)
	{
		auto cell = gameMap.at(pos);
		if (cell != nullptr)
		{
			if (!cell->has_structure())
			{
				int halite = gameMap.halite_in_radius(pos, haliteRadius);
				if (halite > minHalite)
				{
					if (halite > maxHalite)
					{
						maxHalite = halite;
						ret = pos;
					}
				}
			}
		}
	}
	return ret;
}

int  game_area::get_max_halite(hlt::GameMap& gameMap) const
{
	int halite = 0;
	for (auto pos : _area)
	{
		auto cell = gameMap.at(pos);
		if (cell)
		{
			if (!cell->has_structure())
			{
				if (cell->halite > halite)
				{
					halite = cell->halite;
				}
			}
		}
	}
	return halite;
}
int  game_area::get_halite(hlt::GameMap& gameMap) const
{
	int halite = 0;
	for (auto pos : _area)
	{
		auto cell = gameMap.at(pos);
		if ( cell)
		{
			if (!cell->has_structure())
			{
				halite += cell->halite;
			}
		}
	}
	return halite;
}

int  game_area::get_area() const
{
	return _area.size();
}


Position game_area::max_halite(GameMap& gameMap, std::unordered_set<Position>& maxCollection, int& halite, const hlt::Position& start, int radius) const
{
	std::unordered_set<Direction> possibleDirs;
	// need to get valid directions from this as we are just leaving
	if (gameMap.at(start.directional_offset(Direction::EAST))->is_empty())
	{
		possibleDirs.insert(Direction::EAST);
	}
	if (gameMap.at(start.directional_offset(Direction::WEST))->is_empty())
	{
		possibleDirs.insert(Direction::WEST);
	}
	if (gameMap.at(start.directional_offset(Direction::NORTH))->is_empty())
	{
		possibleDirs.insert(Direction::NORTH);
	}
	if (gameMap.at(start.directional_offset(Direction::SOUTH))->is_empty())
	{
		possibleDirs.insert(Direction::SOUTH);
	}

	Position ret(0, 0);
	halite = 0;
	for (auto pos : _area)
	{
		auto cell = gameMap.at(pos);
		if (cell)
		{
			if (!cell->has_structure())
			{
				if (maxCollection.find(pos) == maxCollection.end())
				{
					if (gameMap.navigate_possible(start, pos, possibleDirs))
					{
						if (radius == 0 || gameMap.calculate_distance(start, pos) <= radius)
						{
							if (cell->halite > halite)
							{
								ret = pos;
								halite = cell->halite;
							}
						}
					}
				}
			}
		}
	}
	if (halite > 0)
	{
		// add 8 squares around it....
		maxCollection.insert(ret);
	}
	return ret;
}
Position game_area::max_halite(GameMap& gameMap, std::unordered_set<Position>& maxCollection, int& halite) const
{
	Position ret(0, 0);
	halite = 0;
	for (auto pos : _area)
	{
		auto cell = gameMap.at(pos);
		if (cell)
		{
			if (!cell->has_structure())
			{
				if ( maxCollection.find(pos) == maxCollection.end())
				{
					if (cell->halite > halite)
					{
						ret = pos;
						halite = cell->halite;
					}
				}
			}
		}
	}
	// add 8 squares around it....
	maxCollection.insert(ret);
	return ret;
}