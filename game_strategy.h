#pragma once

#include "hlt/game.hpp"
#include "hlt/command.hpp"
#include "ship_container.hpp"
#include "game_area.h"
#include <vector>
#include <random>

using namespace hlt;
using namespace std;

class GameStrategy
{
public:
	GameStrategy(hlt::Game& game, mt19937& rng);
	virtual ~GameStrategy();

	virtual vector<Command> GetCommandQueue();
protected:
	virtual void _CheckRadius();
	virtual bool _CreateShip();

	bool  _FindDrop(Position& pos);
	bool  _CreateDrops();

//	virtual ShipContainer*	_CreateShipContainer
	Game& _game;
	unordered_map<EntityId, unique_ptr<ShipContainer>> _shipContainers;

	unordered_set<Position> _maxCollection;
	mt19937&		 _rng;
	int				 _radius;
	int				 _dropOffs;
	vector<Position> _drops;
	bool			 _endGame;
	game_area		 _area;
	game_area		 _dropArea;
	int				 _dropRadius;
	int				 _maxDrops;
	int				 _maxRadius;
	int				 _radiusIncreaseMaxHalite;
	int				 _haliteMinDrop;
	unsigned int	 _minNoShipsForDrop;
	int				 _totalHalite;
	int				 _initialRadius;
	int				 _movesLeft;
	int				 _percentageCollect;
};
