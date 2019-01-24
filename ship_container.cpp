#include "ship_container.hpp"

using namespace hlt;



void ShipContainer::SetDirection(const Direction& direction) 
{   
	_direction = direction; 
	_nextCommand = hlt::command::move(_ship_id, _direction);
}

void ShipContainer::ClearTargetHalite(std::unordered_set<hlt::Position>& maxHalite)
{
	if (!_droppingOff)
	{
		auto iter = maxHalite.find(_target);
		if (iter != maxHalite.end())
		{
			maxHalite.erase(iter);
		}
	}
}

Command ShipContainer::GetInitialCommand(GameMap &game_map, std::shared_ptr<Ship> ship, std::mt19937& rng,std::unordered_set<hlt::Position>& maxCollection, const hlt::PlayerId& playerId, int radius, int remainingTurns, const game_area& area, const std::vector<hlt::Position>& drops)
{
	int halite = 0;
	hlt::Position maxHalite(0,0);
	if (drops.size() > 1)
	{
		auto dropHalite = game_map.halite_in_radius(drops[drops.size() - 1], 5);

		if (dropHalite < 5000)
		{
			maxHalite = game_map.max_halite(ship->position, radius, maxCollection, halite);
		}
		else
		{
			maxHalite = game_map.max_halite(drops[drops.size() - 1], radius, maxCollection, halite);
		}
	}
	else
	{
		maxHalite = area.max_halite(game_map, maxCollection, halite, ship->position);
	}
	_AddExtraPositions(game_map, maxHalite, maxCollection);

	_CheckHaliteMoveStay(halite);

	_target = maxHalite;
	bool checkOpposition = true;
	_direction = game_map.navigate(ship, _target, rng,playerId,_potential,false,false, checkOpposition,_noMove);
	_nextCommand = ship->move(_direction);
	return _nextCommand;
}

void ShipContainer::SetTargetDrop(Position& pos)
{
	_dropPos = pos;
	_target = _dropPos;
	log::log("Setting target drop " + std::to_string(_ship_id) + " " + std::to_string(_dropPos.x) + " " + std::to_string(_dropPos.y));
}

bool   ShipContainer::IsFindingDrop() const
{
	return _dropPos.x != -1;
}


Command ShipContainer::GetNextCommand(GameMap &game_map, std::shared_ptr<Ship> ship, std::mt19937& rng, std::unordered_set<hlt::Position>& maxCollection, const hlt::PlayerId& playerId, int radius, int remainingTurns, const std::vector<hlt::Position>& drops, const game_area& area, int myHalite)
{
	std::string str;
	str = "Ship " + std::to_string(ship->id) + ", Target: (" + std::to_string(_target.x) + "," + std::to_string(_target.y) + ")";
	str += " , Position: (" + std::to_string(ship->position.x) + "," + std::to_string(ship->position.y) + ")";
	str += " , DropOff: (" + std::to_string(_dropOff.x) + "," + std::to_string(_dropOff.y) + ")";
	str += " , Halite: " + std::to_string(ship->halite);
	str += " , Halite At Position: " + std::to_string(game_map.at(ship->position)->halite);

	Command nextCommand;

	bool recheckDrop(false);
	if (_noDrops != drops.size())
	{
		_noDrops = drops.size();
		recheckDrop = true;
	}

	if (IsFindingDrop())
	{
		log::log("DropShip " + str);
		if (_dropPos == ship->position)
		{
			_noMove = true;
			log::log("Ship at drop off " + std::to_string(myHalite));
			if (myHalite >= constants::DROPOFF_COST)
			{
				log::log("created drop ");
				_newDrop = _dropPos;
			}
			else
			{
				_direction = Direction::STILL;
				nextCommand = ship->move(_direction);
				_nextCommand = nextCommand;
				return nextCommand;
			}
		}
		else
		{
			bool checkOpposition = true;
			_direction = game_map.navigate(ship, _dropPos, rng, playerId, _potential, false, false, checkOpposition, _noMove);
			_noMove = true;
			nextCommand = ship->move(_direction);
			_nextCommand = nextCommand;
			log::log("drop ship moving " + _nextCommand);
			return nextCommand;
		}
	}

	if (_newDrop.x != -1)
	{
		_dropPos.x = -1;
		_dropPos.y = -1;
		_newDrop.x = -1;
		_newDrop.y = -1;
		nextCommand =  ship->make_dropoff();
		_nextCommand = nextCommand;
		_madeDrop = true;
		return nextCommand;
	}
	if (_endGame == false ) // need to work out max turns required to get to drop off
	{
		int turnsToDropOff = game_map.calculate_distance(ship->position, _dropOff);
		if ((1.75*turnsToDropOff) >= ( remainingTurns -4  ) && (1.75*turnsToDropOff) <= remainingTurns  )
		{
			_endGame = true;
		}
	}

	if (_droppingOff)
	{
		str += " DROPPING OFF";
	}
	if (_endGame)
	{
		str += " END GAME";
	}

	log::log(str);
	if (_endGame)
	{
		if (ship->position == _dropOff)
		{
			_direction = Direction::STILL;
			nextCommand = ship->move(_direction);
		}
		else
		{
			_direction = game_map.navigate(ship, _dropOff, rng, playerId, _potential, _endGame,true,false,_noMove);
			nextCommand = ship->move(_direction);
		}
	}
	else if (_droppingOff)
	{
		if (ship->position == _dropOff)
		{
			log::log("Dropping off. Position == DropOff");
			_droppingOff = false;
		//	int areaHalite = 0;
		//	auto areaMaxHalite = area.max_halite(game_map, maxCollection,areaHalite);
		//	_RemovePosition(game_map, areaMaxHalite, maxCollection);
			int halite = 0;
			auto maxHalite = game_map.max_halite(_dropOff, radius, maxCollection, halite);
		/*	if (areaHalite > halite)
			{
				_RemovePosition(game_map, maxHalite, maxCollection);
				maxHalite = areaMaxHalite;
				halite = areaHalite;
				maxCollection.insert(maxHalite);
			}*/
			_AddExtraPositions(game_map, maxHalite, maxCollection);

			_CheckHaliteMoveStay(halite);

		// check were is the best halite anywhere or around this???
			bool checkOpposition = true;
			_target = maxHalite;
			_direction = game_map.navigate(ship, _target, rng,playerId, _potential,false, _droppingOff,checkOpposition, _noMove);
			nextCommand = ship->move(_direction);
		}
		else
		{
			if (recheckDrop)
			{
				if (drops.size() > 1)
				{
					int dist = game_map.calculate_distance(_dropOff, ship->position);
					for (auto drop : drops)
					{
						int d = game_map.calculate_distance(drop, ship->position);
						if (d < dist)
						{
							dist = d;
							_dropOff = drop;
						}
					}
				}
				_target = _dropOff;
			}


			if (ship->halite < _collectMoreHalite && game_map.at(ship)->halite > _haliteRequiredToStay)
			{
				log::log("Dropping off. Halite < 700");
				nextCommand = ship->stay_still();
			}
			else
			{
				log::log("Dropping off. Moving to target");
				bool checkOpposition = true;
				_direction = game_map.navigate(ship, _target, rng,playerId,_potential,false, _droppingOff, checkOpposition , _noMove);
				nextCommand = ship->move(_direction);
			}
		}
	}
	else if ( /*ship->is_full()*/ ship->halite > _collectHalite )
	{
		log::log("Full Ship. Moving to dropoff");
		_droppingOff = true;
		// we need to find the best drop off from the vectors. i.e the closet one and use that.....

		// remove our current target
		_RemovePosition(game_map, _target, maxCollection);

		// find closet drop off point
		if (drops.size() > 1)
		{
			int dist = game_map.calculate_distance(_dropOff, ship->position);
			for (auto drop : drops)
			{
				int d = game_map.calculate_distance(drop, ship->position);
				if (d < dist)
				{
					dist = d;
					_dropOff = drop;
				}
			}
		}
		_target = _dropOff;
		bool checkOpposition = true;
		_direction = game_map.navigate(ship, _dropOff, rng,playerId, _potential,false,_droppingOff, checkOpposition , _noMove);
		nextCommand = ship->move(_direction);
	}
	else if (game_map.at(ship)->halite < _haliteRequiredToMove)
	{
		if (ship->position == _target)
		{
			log::log("At Target finding new target");
			_RemovePosition(game_map,_target, maxCollection);
			int maxHalite = 0;
			//_target = area.max_halite(game_map, maxCollection, maxHalite, ship->position, 4);
			_target = game_map.max_halite(ship->position, 5, maxCollection,maxHalite);
			_AddExtraPositions(game_map, _target, maxCollection);

			_CheckHaliteMoveStay(maxHalite);

		}
		else if (game_map.at(_target)->is_occupied() )
		{
			auto prevTarget = _target;
			int maxHalite = 0;
			_target = game_map.max_halite(_target, 5, maxCollection, maxHalite);
			_AddExtraPositions(game_map, _target, maxCollection);

			_RemovePosition(game_map, prevTarget, maxCollection);
			_CheckHaliteMoveStay(maxHalite);
		}
		bool checkOpposition = true;
		_direction = game_map.navigate(ship, _target, rng,playerId, _potential,false, _droppingOff, checkOpposition,_noMove);
		nextCommand = ship->move(_direction);
	}
	else if (ship->position == _target)
	{
		log::log("Reached Target. Stay");
		nextCommand = ship->stay_still();
	}
	else
	{
		log::log("Other. Stay");
		nextCommand = ship->stay_still();
	}

	str = "Ship " + std::to_string(ship->id) + ", Target: (" + std::to_string(_target.x) + "," + std::to_string(_target.y) + ")";
	str += " , DropOff: (" + std::to_string(_dropOff.x) + "," + std::to_string(_dropOff.y) + ")";
	str += " , Direction: " + std::string(1,(char)_direction);
	str += " , Command: " + nextCommand;
	if (_droppingOff)
	{
		str += " DROPPING OFF";
	}
	log::log(str);
	_nextCommand = nextCommand;
	return nextCommand;
}

void ShipContainer::ClearNextCommand()
{
	_nextCommand = ""; 
	_direction = Direction::STILL; 
	_noMove = true;
	_madeDrop = false;
	_potential.clear();
}

void ShipContainer::_RemovePosition(GameMap &game_map, const Position& position, std::unordered_set<Position>& v)
{
	auto pos = _GetExtraPosition(game_map, position);

	for (auto x : pos)
	{
		auto iter = v.find(x);
		if (iter != v.end())
		{
			v.erase(iter);
		}
	}
}

void ShipContainer::_AddExtraPositions(GameMap &game_map, const Position& position, std::unordered_set<Position>& v)
{
	auto pos = _GetExtraPosition(game_map, position);
	for (auto x : pos)
	{
		v.insert(x);
	}
}

std::vector<Position>  ShipContainer::_GetExtraPosition(GameMap &game_map, const Position& position)
{
	std::vector<Position> pos;

	pos.push_back(position);
	/*
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; i <= 1; i++)
		{
			if (i != 0 && j != 0)
			{
				Position n(position.x + i, position.y + j);
				n = game_map.normalize(n);
				pos.push_back(n);
			}
		}
	}*/
	return pos;
}


void ShipContainer::_CheckHaliteMoveStay(int halite)
{
	if (halite == 0)
		return;

	_collectHalite = 900;
	_collectMoreHalite = 700;
	if (halite < 20)
	{
		_haliteRequiredToMove = 1;
		_haliteRequiredToMove = 1;
		_collectHalite = 999;
		_collectMoreHalite = 900;
	}
	else if (halite < 40)
	{
		_haliteRequiredToMove = 10;
		_haliteRequiredToStay = 12;
		_collectHalite = 990;
		_collectMoreHalite = 900;
	}
	else if (halite < 100)
	{
		_haliteRequiredToMove = halite/5;
		_haliteRequiredToStay = 20;
		_collectHalite = 950;
		_collectMoreHalite = 800;
	}
	else if (halite < 200)
	{
		_haliteRequiredToMove = halite / 5;
		_haliteRequiredToStay = 40;
	}
	else
	{
		_haliteRequiredToMove = halite / 5;
		_haliteRequiredToStay = 100;
	}

	log::log("Halite: " + to_string(halite) + " stay " + to_string(_haliteRequiredToStay) + " move " + to_string(_haliteRequiredToMove));
}