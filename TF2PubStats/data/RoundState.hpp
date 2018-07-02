#pragma once

#if 1
enum class RoundState
{
	Init = 0,
	Pregame = 1,
	StartGame = 2,
	Preround = 3,
	Running = 4,
	TeamWin = 5,
	Restart = 6,
	Stalemate = 7,
	GameOver = 8,
	Bonus = 9,
	BetweenRounds = 10,
};
#else
enum gamerules_roundstate_t
{
	// initialize the game, create teams
	GR_STATE_INIT = 0,

	//Before players have joined the game. Periodically checks to see if enough players are ready
	//to start a game. Also reverts to this when there are no active players
	GR_STATE_PREGAME,

	//The game is about to start, wait a bit and spawn everyone
	GR_STATE_STARTGAME,

	//All players are respawned, frozen in place
	// Pazer: this also means setup time
	GR_STATE_PREROUND,

	//Round is on, playing normally
	GR_STATE_RND_RUNNING,

	//Someone has won the round
	GR_STATE_TEAM_WIN,

	//Noone has won, manually restart the game, reset scores
	GR_STATE_RESTART,

	//Noone has won, restart the game
	GR_STATE_STALEMATE,

	//Game is over, showing the scoreboard etc
	GR_STATE_GAME_OVER,

	GR_NUM_ROUND_STATES
};
#endif