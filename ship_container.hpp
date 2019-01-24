#pragma once

#include "hlt/entity.hpp"
#include "hlt/constants.hpp"
#include "hlt/game_map.hpp"
#include "hlt/ship.hpp"
#include "game_area.h"
#include <string>

using namespace hlt;

class ShipContainer
{
public:
	ShipContainer(const EntityId& ship_id)
		: _ship_id(ship_id),
		  _target(0,0),
		  _dropOff(0,0),
		  _newDrop(-1,-1),
		  _dropPos(-1,-1),
		  _direction(Direction::STILL),
		  _droppingOff(false),
		  _endGame(false),
   		  _madeDrop(false),
		  _haliteRequiredToMove(40), // if less than this then it would move on
		  _haliteRequiredToStay(100), // if more than this that it will stay and pick it up if halite < 700 and dropping off
		  _noMove(false),
		  _noDrops(0),
		  _collectMoreHalite(700),
		  _collectHalite(900)
	{

	}

	Command GetInitialCommand(GameMap &game_map, std::shared_ptr<Ship> ship, std::mt19937& rng, 
		   std::unordered_set<hlt::Position>& maxHalite, const hlt::PlayerId& playerId, int radius, 
		int remainingTurns, const game_area& area, const std::vector<hlt::Position>& drops);

	Command GetNextCommand(GameMap &game_map, std::shared_ptr<Ship> ship, std::mt19937& rng, 
		std::unordered_set<hlt::Position>& maxHalite, const hlt::PlayerId& playerId, int radius,
		int remainingTurns, const std::vector<hlt::Position>& drops, const game_area& area, int myHalite);

	bool	IsDroppingOff() const { return _droppingOff;};
	bool    IsFindingDrop() const;
	bool	GetMadeDrop() const { return _madeDrop; };

	void ClearTargetHalite(std::unordered_set<hlt::Position>& maxHalite);
	void	SetTargetDrop(Position& pos);

	const Position&		GetTarget() const { return _target; }
	const EntityId&		GetShipID() const { return _ship_id; }
	const Direction&	GetDirection() const { return _direction; }

	bool GetEndGame() const { return _endGame; }
	bool CanMove() const { return !_noMove; }

	void SetTarget(const Position& target)			{ _target = target; }
	void SetDropOff(const Position& dropOff)		{ _dropOff = dropOff; }
	void SetDirection(const Direction& direction);
	void MakeDrop(const Position& drop)				 { _newDrop = drop; _droppingOff = false; _noMove = true; }

	void SetHaliteRequiredToMove(int value)			{ _haliteRequiredToMove = value; }
	void SetHaliteRequiredToStay(int value)			{ _haliteRequiredToStay = value; }

	void ClearNextCommand(); 
	const std::string& GetNextCommand() const { return _nextCommand; };

	bool HavePotentialMoves() const { return !_potential.empty(); };
	const std::vector<Direction> GetPotentialMoves() const { return _potential; };

private:
	void _CheckHaliteMoveStay(int halite);

	void _RemovePosition(GameMap &game_map,const Position& position, std::unordered_set<Position>& v);
	void _AddExtraPositions(GameMap &game_map, const Position& position, std::unordered_set<Position>& v);

	std::vector<Position>  _GetExtraPosition(GameMap &game_map, const Position& position);

	std::vector<Direction> _potential;
	EntityId	_ship_id;
	Position	_target;
	Position	_dropOff;
	Position	_newDrop;
	Position    _dropPos;
	Direction	_direction;
	bool		_droppingOff;
	bool		_endGame;
	bool        _madeDrop;
	int			_haliteRequiredToMove;
	int			_haliteRequiredToStay;
	std::string		_nextCommand;
	bool	_noMove;
	unsigned int  _noDrops;
	int			_collectMoreHalite;
	int			_collectHalite;
};

//class ShipCollectorContainer collects everything in radius