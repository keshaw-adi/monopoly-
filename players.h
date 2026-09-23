#ifndef PLAYERS_H
#define PLAYERS_H

#include "types.h"

GameState initializePlayers(GameState game);
void printPlayerName(Strategy strategy);
int wantsToBuy(GameState game, int playerId, int squareIndex);
int maximumAuctionBid(GameState game, int playerId, int squareIndex);
GameState runAuction(GameState game, int squareIndex);
GameState developMonopolies(GameState game, int playerId);
int shouldPayBail(GameState game, int playerId);

#endif
