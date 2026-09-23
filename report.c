#include <stdio.h>
#include "report.h"
#include "board.h"
#include "events.h"
#include "finance.h"
#include "players.h"

void printIntroduction(void) {
    int playerId;
    printf("\nMONOPOLY-LK Simulation\n\n");
    for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
        printf("Player %d : ", playerId + 1);
        printPlayerName(playerId);
        printf("\n");
    }
    printf("\nEach player begins with LKR 30,000.\n\n");
}

void printRoundSummary(GameState game) {
    int playerId;
    printf("\n=============================================\n");
    printf("Round %d Summary\n", game.currentRound);
    printf("=============================================\n\n");
    for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
        printPlayerName(game.players[playerId].strategy);
        printf("\n\n");
        if (game.players[playerId].bankrupt) {
            printf("Status : BANKRUPT\n");
        } else {
            printf("Cash : LKR %d\n\n", game.players[playerId].cash);
            printf("Net Worth : LKR %d\n\n", calculateNetWorth(game, playerId));
            printf("Properties : %d\n\n", countOwnedProperties(game, playerId));
            printf("Hotels : %d\n\n", countHotels(game, playerId));
            if (game.players[playerId].loanBalance > 0)
                printf("Outstanding Loan : LKR %d\n", game.players[playerId].loanBalance);
            else
                printf("Outstanding Loan : None\n\n");
        }
        printf("---------------------------------------------\n\n");
    }
    printf("=============================================\n\n");
}

void printGameOver(GameState game) {
    int index;
    int propertyValue = 0;
    int winner = game.winner;
    printf("\n=============================================\n");
    printf("GAME OVER\n\n");
    if (winner < 0) {
        printf("No solvent player remains.\n");
        printf("=============================================\n");
        return;
    }

    printf("Winner\n\n");
    printPlayerName(game.players[winner].strategy);
    printf("\n\nTotal Cash\nLKR %d\n\n", game.players[winner].cash);
    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == winner)
            propertyValue += adjustedMarketValue(game, index);
    }
    printf("Total Property Value\nLKR %d\n\n", propertyValue);
    if (game.players[winner].loanBalance > 0)
        printf("Outstanding Loans\n\nLKR %d\n\n", game.players[winner].loanBalance);
    else
        printf("Outstanding Loans\n\nNone\n\n");
    printf("Net Worth\n\nLKR %d\n\n", calculateNetWorth(game, winner));
    printf("=============================================\n");
}