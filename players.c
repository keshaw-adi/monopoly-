#include <stdio.h>
#include "players.h"
#include "board.h"
#include "events.h"

GameState initializePlayers(GameState game) {
    int index;

    for (index = 0; index < PLAYER_COUNT; index++) {
        game.players[index].strategy = index;
        game.players[index].cash = STARTING_CASH;
        game.players[index].position = 0;
        game.players[index].jailed = 0;
        game.players[index].jailTurns = 0;
        game.players[index].bankrupt = 0;
        game.players[index].hasExperiencedLoss = 0;

        game.players[index].loanBalance = 0;
        game.players[index].loanRate = 0;
        game.players[index].loanRoundsRemaining = 0;
        game.players[index].mortgageLiability = 0;
        game.players[index].insuranceClaimsReceivable = 0;
        game.players[index].taxesDue = 0;

        game.players[index].activeCard = NO_EVENT;
        game.players[index].cardTargetGroup = NO_GROUP;
        game.players[index].cardRoundsRemaining = 0;
        game.players[index].passedGoThisRound = 0;
    }

    game.activePlayers = PLAYER_COUNT;

    return game;
}

void printPlayerName(Strategy strategy) {
    switch (strategy) {
        case STRATEGY_AGGRESSIVE: printf("Aggressive Investor"); break;
        case STRATEGY_CONSERVATIVE: printf("Conservative Banker"); break;
        case STRATEGY_RISK_TAKER: printf("Risk Taker"); break;
        case STRATEGY_OPPORTUNISTIC: printf("Opportunistic Trader"); break;
        default: printf("Unknown Player"); break;
    }
}

static int countUndevelopedProperties(GameState game, int playerId) {
    int count = 0;
    int index;
    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId &&
            game.board[index].type == SPACE_PROPERTY &&
            game.board[index].houses == 0 && !game.board[index].hotel) {
            count++;
        }
    }
    return count;
}

int wantsToBuy(GameState game, int playerId, int squareIndex) {
    int price = adjustedPurchasePrice(game, squareIndex);
    int cash = game.players[playerId].cash;
    Strategy strategy = game.players[playerId].strategy;

    if (cash < price || game.players[playerId].bankrupt) return 0;
    if (game.economy.activeRegulation == REGULATION_ANTI_SPECULATION &&
        game.board[squareIndex].type == SPACE_PROPERTY &&
        countUndevelopedProperties(game, playerId) >= 3 &&
        !ownsMonopoly(game, playerId, game.board[squareIndex].group)) {
        return 0;
    }

    if (strategy == STRATEGY_AGGRESSIVE) {
        return cash - price >= 1200;
    }
    if (strategy == STRATEGY_CONSERVATIVE) {
        if (game.economy.activeEconomicEvent == ECONOMY_RECESSION) return 0;
        return cash - price >= cash / 2;
    }
    if (strategy == STRATEGY_RISK_TAKER) {
        return 1;
    }

    if (game.board[squareIndex].type == SPACE_PROPERTY) {
        int appreciation = adjustedMarketValue(game, squareIndex) - price;
        int projectedReturn = appreciation + adjustedRentBase(game, squareIndex) * 8;
        return projectedReturn > game.board[squareIndex].houseCost && cash - price >= 5000;
    }
    return cash - price >= 5000;
}

int maximumAuctionBid(GameState game, int playerId, int squareIndex) {
    int marketValue = adjustedMarketValue(game, squareIndex);
    int cash = game.players[playerId].cash;
    Strategy strategy = game.players[playerId].strategy;
    int maximum = 0;

    if (game.players[playerId].bankrupt) return 0;
    if (strategy == STRATEGY_AGGRESSIVE) maximum = marketValue * 120 / 100;
    if (strategy == STRATEGY_CONSERVATIVE) {
        maximum = marketValue - 250;
        if (game.board[squareIndex].type == SPACE_RAILWAY ||
            game.board[squareIndex].type == SPACE_UTILITY) maximum = marketValue;
        if (maximum > cash - 10000) maximum = cash - 10000;
    }
    if (strategy == STRATEGY_RISK_TAKER) maximum = cash;
    if (strategy == STRATEGY_OPPORTUNISTIC) {
        maximum = marketValue * 85 / 100;
        if (maximum > cash - 5000) maximum = cash - 5000;
    }
    if (maximum > cash) maximum = cash;
    if (maximum < 0) maximum = 0;
    return maximum;
}

GameState runAuction(GameState game, int squareIndex) {
    int maximumBids[PLAYER_COUNT];
    int openingBid = adjustedMarketValue(game, squareIndex) / 2;
    int highestBid = 0;
    int secondHighest = 0;
    int winner = BANK_OWNER;
    int price;
    int playerId;

    if (game.board[squareIndex].group == game.economy.declineGroup)
        openingBid = openingBid * 75 / 100;
    openingBid = openingBid / 250 * 250;
    if (openingBid < 250) openingBid = 250;

    printf("\nAuction Started.\nProperty : %s\nOpening Bid : LKR %d.\n",
           game.board[squareIndex].name, openingBid);

    for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
        maximumBids[playerId] = maximumAuctionBid(game, playerId, squareIndex);
        printPlayerName(game.players[playerId].strategy);
        if (game.players[playerId].bankrupt || maximumBids[playerId] < openingBid) {
            printf(" withdraws.\n");
        } else {
            printf(" bids up to LKR %d.\n", maximumBids[playerId]);
            if (maximumBids[playerId] > highestBid) {
                secondHighest = highestBid;
                highestBid = maximumBids[playerId];
                winner = playerId;
            } else if (maximumBids[playerId] > secondHighest) {
                secondHighest = maximumBids[playerId];
            }
        }
    }

    if (winner == BANK_OWNER) {
        printf("No player bids. Ownership remains with the Bank.\n");
        return game;
    }

    price = openingBid;
    if (secondHighest >= openingBid) price = secondHighest + 250;
    if (price > highestBid) price = highestBid;
    game.players[winner].cash -= price;
    game.board[squareIndex].owner = winner;
    game.board[squareIndex].mortgaged = 0;
    game.board[squareIndex].loanLocked = 0;
    printPlayerName(game.players[winner].strategy);
    printf(" wins the auction for LKR %d.\n", price);
    return game;
}

static int developmentLevel(GameState game, int squareIndex) {
    if (game.board[squareIndex].hotel) return 5;
    return game.board[squareIndex].houses;
}

static int reserveForStrategy(Strategy strategy) {
    if (strategy == STRATEGY_AGGRESSIVE) return 1200;
    if (strategy == STRATEGY_CONSERVATIVE) return 10000;
    if (strategy == STRATEGY_OPPORTUNISTIC) return 6000;
    return 0;
}

GameState developMonopolies(GameState game, int playerId) {
    int maximumBuilds;
    int completed = 0;
    int group;
    int index;
    Strategy strategy = game.players[playerId].strategy;
    int reserve = reserveForStrategy(strategy);

    if (constructionIsSuspended(game, playerId)) {
        printf("Construction is suspended for ");
        printPlayerName(strategy);
        printf(".\n");
        return game;
    }
    if (strategy == STRATEGY_OPPORTUNISTIC && game.economy.inflationPercent > 0 &&
        game.economy.activeRegulation != REGULATION_HOUSING_SUBSIDY) {
        return game;
    }

    maximumBuilds = 1;
    if (strategy == STRATEGY_AGGRESSIVE || strategy == STRATEGY_RISK_TAKER)
        maximumBuilds = 20;
    if (strategy == STRATEGY_OPPORTUNISTIC) maximumBuilds = 2;

    while (completed < maximumBuilds) {
        int builtSomething = 0;
        for (group = 0; group < GROUP_COUNT && completed < maximumBuilds; group++) {
            int candidate = -1;
            int minimumLevel = 6;
            int cost;
            int level;

            if (!ownsMonopoly(game, playerId, group)) continue;
            for (index = 0; index < BOARD_SIZE; index++) {
                if (game.board[index].type == SPACE_PROPERTY &&
                    game.board[index].group == group &&
                    game.board[index].owner == playerId &&
                    !game.board[index].mortgaged) {
                    level = developmentLevel(game, index);
                    if (level < minimumLevel) {
                        minimumLevel = level;
                        candidate = index;
                    }
                }
            }
            if (candidate < 0 || minimumLevel >= 5) continue;

            if (minimumLevel < 4) {
                cost = adjustedHouseCost(game, playerId, candidate);
                if (game.players[playerId].cash - cost < reserve) continue;
                game.players[playerId].cash -= cost;
                game.board[candidate].houses++;
                game.board[candidate].buildingCondition = 100;
                printPlayerName(strategy);
                printf(" constructed one house on %s.\nConstruction Cost : LKR %d.\n",
                       game.board[candidate].name, cost);
            } else {
                if (strategy == STRATEGY_CONSERVATIVE && game.players[playerId].loanBalance > 0)
                    continue;
                cost = adjustedHotelCost(game, playerId, candidate);
                if (game.players[playerId].cash - cost < reserve) continue;
                game.players[playerId].cash -= cost;
                game.board[candidate].houses = 0;
                game.board[candidate].hotel = 1;
                game.board[candidate].buildingCondition = 100;
                printPlayerName(strategy);
                printf(" upgraded %s to a Hotel.\nConstruction Cost : LKR %d.\n",
                       game.board[candidate].name, cost);
            }
            completed++;
            builtSomething = 1;
        }
        if (!builtSomething) break;
    }
    return game;
}

int shouldPayBail(GameState game, int playerId) {
    Strategy strategy = game.players[playerId].strategy;
    if (game.players[playerId].cash < BAIL_AMOUNT) return 0;
    if (strategy == STRATEGY_CONSERVATIVE) return 1;
    if (strategy == STRATEGY_AGGRESSIVE && game.players[playerId].cash > 3000) return 1;
    if (strategy == STRATEGY_OPPORTUNISTIC && game.players[playerId].jailTurns >= 1) return 1;
    return 0;
}
