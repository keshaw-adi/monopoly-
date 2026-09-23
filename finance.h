#ifndef FINANCE_H
#define FINANCE_H

#include "types.h"

int calculateRent(GameState game, int squareIndex, int diceTotal);
PaymentResult transferPayment(GameState game, int payerId, int receiverId, int amount);
GameState payRent(GameState game, int playerId, int squareIndex, int diceTotal);
GameState chargeIncomeTax(GameState game, int playerId);
GameState chargeCommunityFundTax(GameState game, int playerId);
GameState performBankTransaction(GameState game, int playerId);
GameState offerInsurance(GameState game, int playerId);
GameState processRoundFinances(GameState game);
GameState performMaintenance(GameState game, int playerId);
GameState renovateOwnedProperty(GameState game, int playerId, int squareIndex);
GameState declareBankruptcy(GameState game, int playerId, int creditorId);
int calculateNetWorth(GameState game, int playerId);
int calculateMaximumLoan(GameState game, int playerId);
int currentIncomeTaxBasisPoints(GameState game);
int currentCommunityFundBasisPoints(GameState game);

#endif
