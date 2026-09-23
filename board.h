#ifndef BOARD_H
#define BOARD_H

#include "types.h"

GameState BoardStructure(GameState game);
GameState initializeBoard(GameState game);
DiceRoll rollDice(void);
GameState movePlayer(GameState game, int playerId, int spaces);
int isPurchasable(SpaceType type);
int groupPropertyCount(int group);
int countOwnedInGroup(GameState game, int playerId, int group);
int ownsMonopoly(GameState game, int playerId, int group);
int countOwnedRailways(GameState game, int playerId);
int countOwnedUtilities(GameState game, int playerId);
int countOwnedAssets(GameState game, int playerId);
int countOwnedProperties(GameState game, int playerId);
int countHotels(GameState game, int playerId);
void printSquareName(GameState game, int squareIndex);

#endif
