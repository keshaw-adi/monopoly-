#include <stdio.h>
#include "game.h"
#include "board.h"
#include "events.h"
#include "finance.h"
#include "players.h"
#include "report.h"

static GameState determineTurnOrder(GameState game) {
    int rolls[PLAYER_COUNT];
    int candidates[PLAYER_COUNT] = {1, 1, 1, 1};
    int highest;
    int highestPlayer;
    int candidatesRemaining = PLAYER_COUNT;
    int playerId;
    int orderIndex;

    while (candidatesRemaining > 1) {
        highest = 0;
        for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
            if (!candidates[playerId]) continue;
            rolls[playerId] = rollDice().total;
            printPlayerName(game.players[playerId].strategy);
            printf(" rolls %d.\n", rolls[playerId]);
            if (rolls[playerId] > highest) highest = rolls[playerId];
        }
        candidatesRemaining = 0;
        for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
            if (candidates[playerId] && rolls[playerId] < highest)
                candidates[playerId] = 0;
            if (candidates[playerId]) candidatesRemaining++;
        }
        if (candidatesRemaining > 1)
            printf("Highest roll tied. Only the tied players reroll.\n\n");
    }
    highestPlayer = 0;
    for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
        if (candidates[playerId]) highestPlayer = playerId;
    }

    printf("\n");
    printPlayerName(game.players[highestPlayer].strategy);
    printf(" will begin the game.\n\nTurn order:\n");
    for (orderIndex = 0; orderIndex < PLAYER_COUNT; orderIndex++) {
        game.turnOrder[orderIndex] = (highestPlayer + orderIndex) % PLAYER_COUNT;
        printPlayerName(game.players[game.turnOrder[orderIndex]].strategy);
        printf("\n");
    }
    return game;
}

static GameState purchaseAsset(GameState game, int playerId, int squareIndex) {
    int price = adjustedPurchasePrice(game, squareIndex);
    if (wantsToBuy(game, playerId, squareIndex)) {
        game.players[playerId].cash -= price;
        game.board[squareIndex].owner = playerId;
        printPlayerName(game.players[playerId].strategy);
        printf(" purchased %s for LKR %d.\nRemaining Balance : LKR %d.\n",
               game.board[squareIndex].name, price, game.players[playerId].cash);
    } else {
        printPlayerName(game.players[playerId].strategy);
        printf(" declined to purchase %s.\n", game.board[squareIndex].name);
        game = runAuction(game, squareIndex);
    }
    return game;
}

static GameState sendToJail(GameState game, int playerId) {
    game.players[playerId].position = 10;
    game.players[playerId].jailed = 1;
    game.players[playerId].jailTurns = 0;

    game.players[playerId].passedGoThisRound = 0;
    printPlayerName(game.players[playerId].strategy);
    printf(" was sent directly to Jail. No GO money was collected.\n");
    return game;
}

static GameState resolveLanding(GameState game, int playerId, int diceTotal) {
    int squareIndex = game.players[playerId].position;
    SpaceType type = game.board[squareIndex].type;
    printf("Landed on Square %d : %s.\n", squareIndex, game.board[squareIndex].name);

    if (isPurchasable(type)) {
        if (game.board[squareIndex].owner == BANK_OWNER)
            game = purchaseAsset(game, playerId, squareIndex);
        else if (game.board[squareIndex].owner != playerId)
            game = payRent(game, playerId, squareIndex, diceTotal);
        else
            game = renovateOwnedProperty(game, playerId, squareIndex);
    } else if (type == SPACE_EVENT) {
        game = drawNationalCard(game, playerId);
    } else if (type == SPACE_COMMUNITY_FUND) {
        game = chargeCommunityFundTax(game, playerId);
    } else if (type == SPACE_TAX) {
        game = chargeIncomeTax(game, playerId);
    } else if (type == SPACE_INSURANCE) {
        game = offerInsurance(game, playerId);
    } else if (type == SPACE_BANK) {
        game = performBankTransaction(game, playerId);
    } else if (type == SPACE_GO_TO_JAIL) {
        game = sendToJail(game, playerId);
    } else if (type == SPACE_FREE_PARKING) {
        printf("Free Parking: no financial action is required.\n");
    } else if (type == SPACE_JAIL) {
        printf("The player is just visiting Jail.\n");
    }
    return game;
}

static GameState takeJailTurn(GameState game, int playerId) {
    DiceRoll roll;
    if (shouldPayBail(game, playerId)) {
        game.players[playerId].cash -= BAIL_AMOUNT;
        game.players[playerId].jailed = 0;
        game.players[playerId].jailTurns = 0;
        printPlayerName(game.players[playerId].strategy);
        printf(" paid bail of LKR %d and left Jail.\n", BAIL_AMOUNT);
        return game;
    }

    roll = rollDice();
    printPlayerName(game.players[playerId].strategy);
    printf(" rolled %d while in Jail.\n", roll.total);
    if (roll.isDoubles) {
        game.players[playerId].jailed = 0;
        game.players[playerId].jailTurns = 0;
        printf("Doubles were rolled. The player leaves Jail.\n");
        game = movePlayer(game, playerId, roll.total);
        game = resolveLanding(game, playerId, roll.total);
    } else {
        game.players[playerId].jailTurns++;
        if (game.players[playerId].jailTurns >= 3) {
            game.players[playerId].jailed = 0;
            game.players[playerId].jailTurns = 0;
            printf("Three jailed turns are complete. The player leaves Jail.\n");
            game = movePlayer(game, playerId, roll.total);
            game = resolveLanding(game, playerId, roll.total);
        } else {
            printf("The player remains in Jail (%d of 3 turns).\n",
                   game.players[playerId].jailTurns);
        }
    }
    return game;
}

static GameState playTurn(GameState game, int playerId) {
    DiceRoll roll;
    if (game.players[playerId].bankrupt) return game;

    printf("\n---------------------------------------------\n");
    printf("Round %d - ", game.currentRound);
    printPlayerName(game.players[playerId].strategy);
    printf("'s Turn\n");
    game = performMaintenance(game, playerId);

    if (game.players[playerId].jailed) {
        int wasJailed = game.players[playerId].jailed;
        game = takeJailTurn(game, playerId);
        if (wasJailed && game.players[playerId].jailed) return game;
        if (game.players[playerId].position != 10 || game.players[playerId].bankrupt) {
            if (!game.players[playerId].bankrupt)
                game = developMonopolies(game, playerId);
            return game;
        }
    }

    roll = rollDice();
    printPlayerName(game.players[playerId].strategy);
    printf(" rolled %d.\n", roll.total);
    game = movePlayer(game, playerId, roll.total);
    game = resolveLanding(game, playerId, roll.total);
    if (!game.players[playerId].bankrupt && !game.players[playerId].jailed)
        game = developMonopolies(game, playerId);
    return game;
}

static int chooseWinner(GameState game) {
    int winner = -1;
    int highestNetWorth = 0;
    int playerId;
    for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
        int netWorth;
        if (game.players[playerId].bankrupt) continue;
        netWorth = calculateNetWorth(game, playerId);
        if (winner == -1 || netWorth > highestNetWorth) {
            highestNetWorth = netWorth;
            winner = playerId;
        }
    }
    return winner;
}

GameState createGame(void) {
    GameState game = {0};
    game = initializeBoard(game);
    game = initializePlayers(game);
    game = initializeEvents(game);
    game.currentRound = 0;
    game.gameOver = 0;
    game.winner = -1;
    return game;
}

static int allActivePlayersPassedGo(GameState game) {
    int playerId;

    for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
        if (!game.players[playerId].bankrupt) {
            if (game.players[playerId].jailed ||
                !game.players[playerId].passedGoThisRound) {
                return 0;
            }
        }
    }

    return 1;
}

static GameState resetGoStatus(GameState game) {
    int playerId;

    for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
        game.players[playerId].passedGoThisRound = 0;
    }

    return game;
}

static void printRoundHeader(int round) {
    printf("\n\n=============================================\n");
    printf("ROUND %d\n", round);
    printf("=============================================\n");
}

GameState runGame(GameState game) {
    int orderIndex = 0;

    printIntroduction();
    game = determineTurnOrder(game);

    game.currentRound = 1;
    printRoundHeader(game.currentRound);

    while (game.currentRound <= MAX_ROUNDS &&
           game.activePlayers > 1) {
        int playerId = game.turnOrder[orderIndex];

        game = playTurn(game, playerId);

        orderIndex++;
        if (orderIndex >= PLAYER_COUNT) {
            orderIndex = 0;
        }

        if (game.activePlayers <= 1) {
            break;
        }

        /*
         * Complete the round only when every active player
         * has passed GO and nobody is waiting in Jail.
         */
        if (allActivePlayersPassedGo(game)) {
            game = processRoundFinances(game);
            game = processEventTimers(game);
            game = processScheduledEvents(game);

            printRoundSummary(game);
            printMarketConditions(game);

            if (game.activePlayers <= 1 ||
                game.currentRound >= MAX_ROUNDS) {
                break;
            }

            game = resetGoStatus(game);
            game.currentRound++;

            printRoundHeader(game.currentRound);
        }
    }

    game.winner = chooseWinner(game);
    game.gameOver = 1;
    printGameOver(game);

    return game;
}