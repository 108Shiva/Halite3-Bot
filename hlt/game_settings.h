#pragma once
class GameSettings
{
public:
	GameSettings();
	virtual ~GameSettings();

	virtual int GetMaxDrops()		const = 0;
	virtual int GetMaxDropRadius()  const = 0;
	virtual int GetMaxExpandRadius()  const = 0;
	virtual bool GetAvoidCollision() const { return true; }
	virtual int  GetPercentageCollect() const { return 50; };
	virtual int  GetMovesLeft() const { return 100; };
	virtual int  GetHaliteForDrop() const { return 11000; }
};


class GameSettings_32_2_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 1; }
	virtual int GetMaxDropRadius()		const { return 10; }
	virtual int GetMaxExpandRadius()	const { return 16; }
	virtual int  GetPercentageCollect() const { return 60; };
	virtual int  GetHaliteForDrop() const { return 10000; }

};

class GameSettings_32_4_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return -1; }
	virtual int GetMaxDropRadius()		const { return 10; }
	virtual int GetMaxExpandRadius()	const { return 12; }
	virtual int  GetPercentageCollect() const { return 60; };
	virtual int  GetHaliteForDrop() const { return 12000; }

};

class GameSettings_40_2_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 2; }
	virtual int GetMaxDropRadius()		const { return 10; }
	virtual int GetMaxExpandRadius()	const { return 20; }
	virtual int  GetPercentageCollect() const { return 50; };
	virtual int  GetHaliteForDrop() const { return 12000; }
};

class GameSettings_40_4_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 1; }
	virtual int GetMaxDropRadius()		const { return 10; }
	virtual int GetMaxExpandRadius()	const { return 10; }
	virtual int  GetPercentageCollect() const { return 55; };
	virtual int  GetHaliteForDrop() const { return 12000; }
};

class GameSettings_48_2_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 2; }
	virtual int GetMaxDropRadius()		const { return 12; }
	virtual int GetMaxExpandRadius()	const { return 24; }
	virtual int  GetPercentageCollect() const { return 50; };

};

class GameSettings_48_4_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 2; }
	virtual int GetMaxDropRadius()		const { return 12; }
	virtual int GetMaxExpandRadius()	const { return 20; }

};

class GameSettings_56_2_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 3; }
	virtual int GetMaxDropRadius()		const { return 12; }
	virtual int GetMaxExpandRadius()	const { return 28; }
	virtual int GetPercentageCollect() const { return 50; };
	virtual int GetMovesLeft()			const { return 125; };
};

class GameSettings_56_4_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 2; }
	virtual int GetMaxDropRadius()		const { return 12; }
	virtual int GetMaxExpandRadius()	const { return 21; }
};

class GameSettings_64_2_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 4; }
	virtual int GetMaxDropRadius()		const { return 14; }
	virtual int GetMaxExpandRadius()	const { return 32; }
	virtual int  GetPercentageCollect() const { return 50; };
	virtual int  GetMovesLeft()			const { return 150; };
};

class GameSettings_64_4_Player : public GameSettings
{
public:
	virtual int GetMaxDrops()			const { return 4; }
	virtual int GetMaxDropRadius()		const { return 14; }
	virtual int GetMaxExpandRadius()	const { return 24; }
};
