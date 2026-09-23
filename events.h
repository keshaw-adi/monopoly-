#ifndef EVENTS_H
#define EVENTS_H

#include "types.h"

GameState initializeEvents(GameState game);
GameState drawNationalCard(GameState game, int playerId);
GameState processScheduledEvents(GameState game);
GameState processEventTimers(GameState game);
GameState causeDisaster(GameState game);
void printMarketConditions(GameState game);
int adjustedPurchasePrice(GameState game, int squareIndex);
int adjustedMarketValue(GameState game, int squareIndex);
int adjustedRentBase(GameState game, int squareIndex);
int adjustedHouseCost(GameState game, int playerId, int squareIndex);
int adjustedHotelCost(GameState game, int playerId, int squareIndex);
int adjustedMortgageValue(GameState game, int squareIndex);
int adjustedInsurancePremium(GameState game, int playerId, int amount);
int effectiveLoanRate(GameState game, int playerId);
int constructionIsSuspended(GameState game, int playerId);

#endif
