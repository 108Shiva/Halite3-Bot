#pragma once

#include "types.hpp"
#include "map_cell.hpp"
#include <random>
#include <vector>
#include <unordered_set>

namespace hlt {
    struct GameMap {
        int width;
        int height;


        std::vector<std::vector<MapCell>> cells;

        MapCell* at(const Position& position) {
            Position normalized = normalize(position);
            return &cells[normalized.y][normalized.x];
        }

        MapCell* at(const Entity& entity) {
            return at(entity.position);
        }

        MapCell* at(const Entity* entity) {
            return at(entity->position);
        }

        MapCell* at(const std::shared_ptr<Entity>& entity) {
            return at(entity->position);
        }

        int calculate_distance(const Position& source, const Position& target) {
            const auto& normalized_source = normalize(source);
            const auto& normalized_target = normalize(target);

            const int dx = std::abs(normalized_source.x - normalized_target.x);
            const int dy = std::abs(normalized_source.y - normalized_target.y);

            const int toroidal_dx = std::min(dx, width - dx);
            const int toroidal_dy = std::min(dy, height - dy);

            return toroidal_dx + toroidal_dy;
        }

        Position normalize(const Position& position) {
            const int x = ((position.x % width) + width) % width;
            const int y = ((position.y % height) + height) % height;
            return { x, y };
        }

        std::vector<Direction> get_unsafe_moves(const Position& source, const Position& destination) {
            const auto& normalized_source = normalize(source);
            const auto& normalized_destination = normalize(destination);

            const int dx = std::abs(normalized_source.x - normalized_destination.x);
            const int dy = std::abs(normalized_source.y - normalized_destination.y);
            const int wrapped_dx = width - dx;
            const int wrapped_dy = height - dy;

            std::vector<Direction> possible_moves;

            if (normalized_source.x < normalized_destination.x) {
                possible_moves.push_back(dx > wrapped_dx ? Direction::WEST : Direction::EAST);
            } else if (normalized_source.x > normalized_destination.x) {
                possible_moves.push_back(dx < wrapped_dx ? Direction::WEST : Direction::EAST);
            }

            if (normalized_source.y < normalized_destination.y) {
                possible_moves.push_back(dy > wrapped_dy ? Direction::NORTH : Direction::SOUTH);
            } else if (normalized_source.y > normalized_destination.y) {
                possible_moves.push_back(dy < wrapped_dy ? Direction::NORTH : Direction::SOUTH);
            }

            return possible_moves;
        }

		std::vector<Direction> get_alt_unsafe_moves(const Position& source, const Position& destination) {
			const auto& normalized_source = normalize(source);
			const auto& normalized_destination = normalize(destination);

			std::vector<Direction> possible_moves;

			if (normalized_source.x == normalized_destination.x) 
			{
				possible_moves.push_back(Direction::WEST);
				possible_moves.push_back(Direction::EAST);
			}
			else if (normalized_source.y == normalized_destination.y) 
			{
				possible_moves.push_back(Direction::NORTH);
				possible_moves.push_back(Direction::SOUTH);
			}

			return possible_moves;
		}
        Direction naive_navigate(std::shared_ptr<Ship> ship, const Position& destination) {
            // get_unsafe_moves normalizes for us
            for (auto direction : get_unsafe_moves(ship->position, destination)) {
                Position target_pos = ship->position.directional_offset(direction);
                if (!at(target_pos)->is_occupied()) {
                    at(target_pos)->mark_unsafe(ship);
                    return direction;
                }
            }

            return Direction::STILL;
        }

		bool navigate_possible(const hlt::Position& start, const Position& destination, const std::unordered_set<Direction>& possibleDirs)
		{
			// get_unsafe_moves normalizes for us
			for (auto direction : get_unsafe_moves(start, destination)) 
			{
				if (possibleDirs.find(direction) != possibleDirs.end())
				{
					return true;
				}
			}

			return false;
		}
		
		bool    has_opponent(const Position& destination, const hlt::PlayerId& playerId);

		Direction navigate(std::shared_ptr<Ship> ship, const Position& destination, std::mt19937& rng, const hlt::PlayerId& playerId, std::vector<Direction>& potential, bool endGame, bool dropOff, bool checkOpposition, bool& noMove);

		int halite_in_radius(const Position& c, int radius)
		{
			Position center(c.x, c.y);
			int total_halite = 0;
			center = normalize(center);

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
						check = normalize(check);
						auto cell = at(check);
						if (cell != nullptr)
						{
							if (!cell->has_structure())
							{
								total_halite += cell->halite;
							}
						}
					}
				}
			}

			return total_halite;
		}

		int ships_in_radius(const Position& c, int radius, const PlayerId& player)
		{
			Position center(c.x, c.y);
			int ships = 0;
			center = normalize(center);

			log::log("ships in radius " + std::to_string(radius));
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
						check = normalize(check);
						auto cell = at(check);
						if (cell != nullptr && cell->is_occupied_by_myself(player))
						{
							ships++;
						}
					}
				}
			}
			return ships;
		}

		static int area_of_radius(int radius)
		{
			return (radius)*(radius + 1) * 2;
		}

		Position max_halite(const Position& x, int searchRadius,int minDistance, int haliteRadius, std::vector<Position>& drops, int minHalite, int& maxHalite)
		{
			Position center(x.x, x.y);
			Position ret = center;
			maxHalite = 0;
			center = normalize(center);

			log::log("checking for max_halite");
			int current = minDistance - 1;
			while (current < searchRadius)
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
						check = normalize(check);
						auto cell = at(check);
						if (cell != nullptr)
						{
							if (!cell->has_structure())
							{
								int d = -1;
								for (auto drop : drops)
								{
									int di = calculate_distance(check, drop);
									if (d == -1)
									{
										d = di;
									}
									else if (di < d)
									{
										d = di;
									}
								}
								if (d >= minDistance)
								{
									int halite = halite_in_radius(check, haliteRadius);
									if (halite > minHalite)
									{
										if (halite > maxHalite)
										{
											maxHalite = halite;
											ret = check;
										}
									}
								}
							}
						}
					}
				}
			}

			log::log("done max_halite " + std::to_string( maxHalite));
			return ret;
		}

		Position max_halite(const Position& x, int radius, std::unordered_set<Position>& maxCollection,int& maxHalite)
		{
			Position center(x.x, x.y);
			Position ret = center;
			maxHalite = 0;
			center = normalize(center);

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
						check = normalize(check);;
						auto cell = at(check);
						if (cell != nullptr)
						{
							if (!cell->has_structure())
							{
								if (maxCollection.find(check) == maxCollection.end())
								{
									if (cell->halite > maxHalite)
									{
										ret = check;
										maxHalite = cell->halite;
									}
								}
							}
						}
					}
				}
			}
			maxCollection.insert(ret);
			return ret;
		}

        void _update();
        static std::unique_ptr<GameMap> _generate();
    };
}
