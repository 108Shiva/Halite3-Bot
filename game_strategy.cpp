#include "game_strategy.h"
#//include "halite_map.h"
#include <unordered_set>


GameStrategy::GameStrategy(hlt::Game& game, mt19937& rng)
	:_game(game),
	_rng(rng),
	_radius(5),
	_dropOffs(1),
	_endGame(false),
	_totalHalite(0)

{
	_dropRadius = game.game_settings->GetMaxDropRadius();
	_maxDrops = game.game_settings->GetMaxDrops();
	_maxRadius = game.game_settings->GetMaxExpandRadius();
	_radiusIncreaseMaxHalite = 450;
	_haliteMinDrop = game.game_settings->GetHaliteForDrop();
	_minNoShipsForDrop = 20;
	_initialRadius = game.game_map->height / 4;
	_movesLeft = game.game_settings->GetMovesLeft();
	_percentageCollect = game.game_settings->GetPercentageCollect();
}


GameStrategy::~GameStrategy()
{
}


bool GameStrategy::_FindDrop(Position& pos)
{
	bool foundDrop(false);

	//first see can we create a drop
	if (_dropOffs != static_cast<int>(_drops.size()) )
	{
		// find the best drop
		unique_ptr<GameMap>& game_map = _game.game_map;
		
		int haliteRadius = 5;
		int maxHalite = 0;
		int minHalite = 11000;
		pos = _dropArea.max_halite(*(game_map.get()),haliteRadius, minHalite, maxHalite);
		
		if (maxHalite > 0)
		{			
			log::log("Found drop " + std::to_string(pos.x) + " " + std::to_string(pos.y));
			foundDrop = true;
		}
	}


	return foundDrop;
}

bool GameStrategy::_CreateDrops()
{
	bool madeDrop(false);

	//first see can we create a drop
	if (_dropOffs != static_cast<int>(_drops.size()) && _game.me->halite >= constants::DROPOFF_COST)
	{
		Position newDrop(0, 0);

		EntityId dropOffShip = -1;
		int halite = 0;
		int distance = 0;

		log::log("checking for drops");

		for (const auto& ship_iterator : _game.me->ships)
		{
			shared_ptr<Ship> ship = ship_iterator.second;

			auto iter = _shipContainers.find(ship->id);

			if (iter != _shipContainers.end())
			{
				int d = -1;

				if (!_game.game_map->at(ship->position)->has_structure())
				{
					for (auto drop : _drops)
					{
						int di = _game.game_map->calculate_distance(ship->position, drop);
						if (d == -1)
						{
							d = di;
						}
						else if (di < d)
						{
							d = di;
						}
					}
				}
				log::log("distance " + to_string(d));

				if (d >= _dropRadius)
				{
					distance = d;
					int dropHalite = _game.game_map->halite_in_radius(ship->position, 5);
					log::log("max halite " + to_string(dropHalite));
					if (dropHalite > halite && dropHalite > _haliteMinDrop )
					{
						halite = dropHalite;
						dropOffShip = ship->id;
						newDrop = ship->position;
					}
				}
			}
		}

		if (distance >= _dropRadius)
		{
			if (dropOffShip != -1)
			{
				auto iter = _shipContainers.find(dropOffShip);
				iter->second->MakeDrop(newDrop);
				_drops.push_back(newDrop);
				_area.add_radius(*(_game.game_map.get()), newDrop, _radius);
				_dropArea.remove_radius(*(_game.game_map.get()), newDrop, _dropRadius);
				madeDrop = true;
			}
		}
	}

	return madeDrop;
}
vector<Command> GameStrategy::GetCommandQueue()
{
	vector<Command> command_queue;

	//halite_map haliteMap;
	shared_ptr<Player> me = _game.me;
	unique_ptr<GameMap>& game_map = _game.game_map;
	auto shipYardPos = me->shipyard->position;
	//haliteMap.setGameMap(*(game_map.get()));


	if (_area.get_area() == 0)
	{
		// add the ship yard
		_area.add_radius(*(game_map.get()),shipYardPos, _radius);
	}
	if (_dropArea.get_area() == 0)
	{
		_dropArea.add_initial(*(game_map.get()), shipYardPos, _game.players.size());
		_dropArea.remove_radius(*(game_map.get()), shipYardPos, _dropRadius);
	}
	if (_totalHalite == 0)
	{
		for (auto& x : game_map->cells)
		{
			for (auto& cell : x)
			{
				_totalHalite += cell.halite;
			}
		}

	}

	if (_drops.empty())
	{
		_drops.emplace_back(shipYardPos);
	}

	auto remainingTurns = constants::MAX_TURNS - _game.turn_number;

	unordered_set<EntityId> dockingShips;
	
	unordered_set<Position> mapPos;

	Position dropPos(0, 0);
	bool foundDrop(false);
	bool madeDrop(false);


	// all ship containers clear next command;
	bool findingDrop(false);

	for (auto& iter : _shipContainers)
	{
		iter.second->ClearNextCommand();
		if (iter.second->IsFindingDrop())
		{
			findingDrop = true;
		}
	}

	if (_dropOffs > 2)
	{
		madeDrop = _CreateDrops();
	}
	else
	{
		if (findingDrop == false)
		{
			if (me->ships.size() >= 10)
			{
				foundDrop = _FindDrop(dropPos);
			}
		}
	}

	// find ship closet to dropPos
	if (foundDrop)
	{
		int minDistance = -1;
		int dropShipID = -1;
		for (const auto& ship_iterator : me->ships)
		{
			shared_ptr<Ship> ship = ship_iterator.second;
			auto d = game_map->calculate_distance(ship->position, dropPos);
			if (minDistance == -1 || d < minDistance)
			{
				dropShipID = ship_iterator.first;
				minDistance = d;
			}
		}

		if (dropShipID != -1)
		{
			auto iter = _shipContainers.find(dropShipID);
			if (iter != _shipContainers.end())
			{
				iter->second->SetTargetDrop(dropPos);
				_dropArea.remove_radius(*(_game.game_map.get()), dropPos, _dropRadius);
				_area.add_radius(*(_game.game_map.get()), dropPos, _dropRadius);

			}
		}
	}

	/*
		If a ship has had a collision then we need to remove the target i.e halite from the halite collection

		so first pass check all ships in _shipContainer and then check if the ship exists in me->ships....
		
	
	*/
	for (auto& iter : _shipContainers)
	{
		auto id = iter.first;

		// ship doesn't exist anymore so we need to clear halite target
		if (me->ships.count(id) == 0)
		{
			iter.second->ClearTargetHalite(_maxCollection);
		}
	}



	for (const auto& ship_iterator : me->ships)
	{
		shared_ptr<Ship> ship = ship_iterator.second;

		auto iter = _shipContainers.find(ship->id);

		if (iter != _shipContainers.end())
		{
			auto shipContainer = iter->second.get();
			if (shipContainer->IsDroppingOff())
			{
				shipContainer->GetNextCommand(*game_map.get(), ship, _rng, _maxCollection, me->id, _radius, remainingTurns,_drops,_area,me->halite);
				dockingShips.insert(ship->id);
			}
		}
	}


	for (const auto& ship_iterator : me->ships)
	{
		shared_ptr<Ship> ship = ship_iterator.second;

		if (dockingShips.find(ship->id) != dockingShips.end())
			continue;

		auto iter = _shipContainers.find(ship->id);

		if (iter == _shipContainers.end())
		{
			ShipContainer *shipContainer = new ShipContainer(ship->id);

			shipContainer->SetDropOff(shipYardPos);
			_shipContainers[ship->id] = unique_ptr<ShipContainer>(shipContainer);

			shipContainer->GetInitialCommand(*game_map.get(), ship, _rng, _maxCollection, me->id, _radius, remainingTurns,_area,_drops);
		}
		else
		{
			auto shipContainer = iter->second.get();
			 
			shipContainer->GetNextCommand(*game_map.get(), ship, _rng, _maxCollection, me->id, _radius, remainingTurns,_drops,_area, me->halite);

			if (shipContainer->GetEndGame())
			{
				_endGame = true;
			}
		}

	}

	std::unordered_set<EntityId> secondPhase;

	int dropID = -1;
	for (auto& iter : _shipContainers)
	{
		if (iter.second->IsFindingDrop())
		{
			command_queue.push_back(iter.second->GetNextCommand());
			continue;
		}
		if (iter.second->GetMadeDrop())
		{
			dropID = iter.first;
			command_queue.push_back(iter.second->GetNextCommand());
			madeDrop = true;
			if (_drops.size() < 2)
			{
				_drops.push_back(iter.second->GetTarget());
			}
			continue;
		}
		if (iter.second->CanMove() && iter.second->GetDirection() == Direction::STILL)
		{
			secondPhase.insert(iter.second->GetShipID());
		}
		else
		{
			command_queue.push_back(iter.second->GetNextCommand());
		}
	}

	if (dropID != -1)
	{
		auto iter = _shipContainers.find(dropID);
		if (iter != _shipContainers.end())
		{
			_shipContainers.erase(iter);
		}
	}
	for (auto iter : secondPhase)
	{
		auto iterS = _shipContainers.find(iter);
		if (iterS == _shipContainers.end())
			continue;

		if (iterS->second->GetDirection() != Direction::STILL)
			continue;

		if (!iterS->second->IsDroppingOff())
			continue;

		log::log("Checking Second phase for " + to_string(iter));

		auto potential = iterS->second->GetPotentialMoves();

		auto ship = me->ships.find(iterS->first);

		bool foundMatch(false);
		for (auto direction : potential)
		{
			if (foundMatch)
				break;
			log::log("Potential direction " + std::string(1,(char)direction));
			
			Position target = ship->second->position.directional_offset(direction);
			if (game_map.get()->at(target)->ship == nullptr)
			{
				// that means we can move now
				foundMatch = true;
				iterS->second->SetDirection(direction);
				game_map.get()->at(ship->second->position)->mark_safe();
				game_map.get()->at(target)->mark_unsafe(ship->second);
				log::log("Found possible move second time around");
			}
			else
			{
				auto iterN = _shipContainers.find(game_map.get()->at(target)->ship->id);
				if (iterN != _shipContainers.end() && iterN->second->GetDirection() == Direction::STILL)
				{		
					log::log("Found other ship " + to_string( iterN->first));

					if (iterN->second->IsDroppingOff())
					{
						auto potentalNext = iterN->second->GetPotentialMoves();
						if (!potentalNext.empty())
						{
							for (auto x : potentalNext)
							{
								log::log("Potential other direction " + std::string(1, (char)x));
							}

							Direction newDirection;
							for (auto x : potentalNext)
							{
								if (x == invert_direction(direction))
								{
									newDirection = x;
									foundMatch = true;
									break;
								}
							}

							if (foundMatch)
							{
								log::log("Found Match to swap");
								iterN->second->SetDirection(newDirection);
								iterS->second->SetDirection(direction);
							}
						}
					}
					else if ( iterN->second->CanMove()) // we swap regardless
					{
						iterN->second->SetDirection(invert_direction(direction));
						iterS->second->SetDirection(direction);
						foundMatch = true;
					}
				}
			}
		}
	}

	// other ships now
	for (auto iter : secondPhase)
	{
		auto iterS = _shipContainers.find(iter);
		if (iterS == _shipContainers.end())
			continue;

		if (iterS->second->GetDirection() != Direction::STILL)
			continue;

		log::log("Checking Second phase for " + to_string(iter));

		auto potential = iterS->second->GetPotentialMoves();

		auto ship = me->ships.find(iterS->first);

		bool foundMatch(false);
		for (auto direction : potential)
		{
			if (foundMatch)
				break;
			log::log("Potential direction " + std::string(1, (char)direction));

			Position target = ship->second->position.directional_offset(direction);
			if (game_map.get()->at(target)->ship == nullptr)
			{
				// that means we can move now
				foundMatch = true;
				iterS->second->SetDirection(direction);
				game_map.get()->at(ship->second->position)->mark_safe();
				game_map.get()->at(target)->mark_unsafe(ship->second);
				log::log("Found possible move second time around");
			}
			else
			{
				auto iterN = _shipContainers.find(game_map.get()->at(target)->ship->id);
				if (iterN != _shipContainers.end() && iterN->second->GetDirection() == Direction::STILL)
				{
					log::log("Found other ship " + to_string(iterN->first));

					auto potentalNext = iterN->second->GetPotentialMoves();
					if (!potentalNext.empty())
					{
						for (auto x : potentalNext)
						{
							log::log("Potential other direction " + std::string(1, (char)x));
						}

						Direction newDirection;
						for (auto x : potentalNext)
						{
							if (x == invert_direction(direction))
							{
								newDirection = x;
								foundMatch = true;
								break;
							}
						}

						if (foundMatch)
						{
							log::log("Found Match to swap");
							iterN->second->SetDirection(newDirection);
							iterS->second->SetDirection(direction);
						}
					}
				}
			}
		}
	}

	// third phase now try alt moves
	// can we do swap on those.
	// if not then do the alt moves
	// these are the EW when we want to go North or South

	// fourth try an other moves that left over

	for (auto iter : secondPhase)
	{
		auto iterS = _shipContainers.find(iter);
		if (iterS == _shipContainers.end())
			continue;

		command_queue.push_back(iterS->second->GetNextCommand());
	}
	// check all ships that could not move and then see if they are two ships that could be swapped so we do
	// have movement
	// or first pass get all possible moves and then second pass allow those moves that no one else needs
	// can be done via a different map that store ship id at pos that needs them
	//
	_CheckRadius();
	
	if (_maxDrops >= 1 && _dropOffs <= 1)
	{
		_dropOffs = 2;
	}

	if (!madeDrop && !_endGame)
	{
		// only make ships if we are below half time and  not after it:-
		if (_CreateShip())
		{
			if (_game.turn_number <= constants::MAX_TURNS - _movesLeft)
			{
				int requiredHalite = constants::SHIP_COST;
				if (_dropOffs != static_cast<int>(_drops.size()) && ( findingDrop || me->ships.size() >= _minNoShipsForDrop))
				{
					requiredHalite += constants::DROPOFF_COST;
				}
				if (me->halite >= requiredHalite &&
					!game_map->at(me->shipyard)->is_occupied_by_myself(me->id))
				{
					command_queue.push_back(me->shipyard->spawn());
				}
			}
		}
	}

	return command_queue;
}

bool GameStrategy::_CreateShip()
{
	int currentHalite = 0;
	for (auto& x : _game.game_map->cells)
	{
		for (auto& cell : x)
		{
			currentHalite += cell.halite;
		}
	}

	double perc = (double)(_percentageCollect) / 100.0;
	int collected = _totalHalite - currentHalite;

	return (((double)(collected) / ((double)_totalHalite) <= perc));
	//return (currentHalite >= _totalHalite / 2);
}

void GameStrategy::_CheckRadius()
{
//	auto halite = _area.get_halite(*(_game.game_map.get()));
//	auto area = _area.get_area();

	auto max_halite = _area.get_max_halite(*_game.game_map.get());

	if ( max_halite < _radiusIncreaseMaxHalite )
//	if ((halite / area) < 200 )
	{
		auto prevRadius = _radius;
		_radius += 2;
		_radius = std::min(_radius, _initialRadius + _maxRadius);
		if (_radius != prevRadius)
		{
			//_maxCollection.clear();// when we get new radius
			for (auto drop : _drops)
			{
				_area.add_radius(*(_game.game_map.get()), drop, _radius);
			}
		}
	}

	log::log("Radius " + to_string(_radius) + " drop radius " + to_string(_dropRadius));

	
	if (_dropRadius != -1 && _radius >= _dropRadius)
	{
		log::log("Radius " + to_string(_radius));
		/*
		if ( _dropOffs >=2 && _dropOffs < (_maxDrops + 1))_
		{

			log::log("drops " + to_string(_dropOffs));
			_dropOffs++;
		}
		*/
		if ( _drops.size() >=2 && _dropOffs == static_cast<int>(_drops.size()) && _dropOffs < (_maxDrops + 1))
		{ 
			auto dropHalite = _game.game_map->halite_in_radius(_drops[_drops.size() - 1], 5);
			if (dropHalite < 2500 )
			{
				log::log("drops " + to_string(_dropOffs));
				_dropOffs++;
			}
		}
	}
}
