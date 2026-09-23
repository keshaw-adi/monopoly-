#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "players.h"

#define SIMPLE_SPACE(number, kind, label) \
    (Space){number, kind, label, NO_GROUP, 0, 0, 0, 0, 0, 0, BANK_OWNER, 0, 0, 0, 0, \
            INSURANCE_NONE, 0, 0, 0, 100, 0, 0, 0, 0, 0}

#define PROPERTY_SPACE(number, label, colour, price, rent, house, hotel, mortgage) \
    (Space){number, SPACE_PROPERTY, label, colour, price, price, rent, house, hotel, mortgage, \
            BANK_OWNER, 0, 0, 0, 0, INSURANCE_NONE, 0, 0, 0, 100, 0, 0, 0, 0, 0}

#define RAILWAY_SPACE(number, label) \
    (Space){number, SPACE_RAILWAY, label, NO_GROUP, 1500, 1500, 250, 0, 0, 750, \
            BANK_OWNER, 0, 0, 0, 0, INSURANCE_NONE, 0, 0, 0, 100, 0, 0, 0, 0, 0}

#define UTILITY_SPACE(number, label) \
    (Space){number, SPACE_UTILITY, label, NO_GROUP, 1500, 1500, 100, 0, 0, 750, \
            BANK_OWNER, 0, 0, 0, 0, INSURANCE_NONE, 0, 0, 0, 100, 0, 0, 0, 0, 0}

GameState initializeBoard(GameState game) {
    game.board[0] = SIMPLE_SPACE(0, SPACE_GO, "GO");
    game.board[1] = PROPERTY_SPACE(1, "Pettah", GROUP_BROWN, 1500, 100, 500, 2000, 750) ; 
    game.board[2] = SIMPLE_SPACE(2, SPACE_COMMUNITY_FUND, "Community Development Fund");
    game.board[3] = PROPERTY_SPACE(3, "Maradana", GROUP_BROWN, 1800, 120, 500, 2000, 750);
    game.board[4] = SIMPLE_SPACE(4, SPACE_TAX, "Income Tax");
    game.board[5] = RAILWAY_SPACE(5, "Colombo Fort Railway Station");
    game.board[6] = PROPERTY_SPACE(6, "Bambalapitiya", GROUP_LIGHT_BLUE, 2500, 180, 750, 3000, 1250);
    game.board[7] = SIMPLE_SPACE(7, SPACE_EVENT, "National Event Card");
    game.board[8] = PROPERTY_SPACE(8, "Wellawatte", GROUP_LIGHT_BLUE, 2700, 200, 750, 3000, 1250);
    game.board[9] = PROPERTY_SPACE(9, "Mount Lavinia", GROUP_LIGHT_BLUE, 3000, 220, 750, 3000, 1250);
    game.board[10] = SIMPLE_SPACE(10, SPACE_JAIL, "Jail / Just Visiting");
    game.board[11] = PROPERTY_SPACE(11, "Nugegoda", GROUP_PINK, 3500, 260, 1000, 4000, 1750);
    game.board[12] = UTILITY_SPACE(12, "Ceylon Electricity Board");
    game.board[13] = PROPERTY_SPACE(13, "Maharagama", GROUP_PINK, 3800, 280, 1000, 4000, 1750);
    game.board[14] = PROPERTY_SPACE(14, "Kottawa", GROUP_PINK, 4000, 300, 1000, 4000, 1750);
    game.board[15] = RAILWAY_SPACE(15, "Kandy Railway Station");
    game.board[16] = PROPERTY_SPACE(16, "Negombo", GROUP_ORANGE, 4500, 350, 1250, 5000, 2250);
    game.board[17] = SIMPLE_SPACE(17, SPACE_INSURANCE, "Sri Lanka Insurance");
    game.board[18] = PROPERTY_SPACE(18, "Katunayake", GROUP_ORANGE, 4700, 370, 1250, 5000, 2250);
    game.board[19] = PROPERTY_SPACE(19, "Ja-Ela", GROUP_ORANGE, 5000, 400, 1250, 5000, 2250);
    game.board[20] = SIMPLE_SPACE(20, SPACE_FREE_PARKING, "Free Parking");
    game.board[21] = PROPERTY_SPACE(21, "Kandy City", GROUP_RED, 5500, 450, 1500, 6000, 2750);
    game.board[22] = SIMPLE_SPACE(22, SPACE_EVENT, "National Event Card");
    game.board[23] = PROPERTY_SPACE(23, "Peradeniya", GROUP_RED, 5800, 480, 1500, 6000, 2750);
    game.board[24] = PROPERTY_SPACE(24, "Katugastota", GROUP_RED, 6000, 500, 1500, 6000, 2750);
    game.board[25] = RAILWAY_SPACE(25, "Galle Railway Station");
    game.board[26] = PROPERTY_SPACE(26, "Galle Fort", GROUP_YELLOW, 6500, 600, 2000, 8000, 3250);
    game.board[27] = PROPERTY_SPACE(27, "Unawatuna", GROUP_YELLOW, 6800, 620, 2000, 8000, 3250);
    game.board[28] = UTILITY_SPACE(28, "National Water Supply and Drainage Board");
    game.board[29] = PROPERTY_SPACE(29, "Hikkaduwa", GROUP_YELLOW, 7000, 650, 2000, 8000, 3250);
    game.board[30] = SIMPLE_SPACE(30, SPACE_GO_TO_JAIL, "Go To Jail");
    game.board[31] = PROPERTY_SPACE(31, "Jaffna Town", GROUP_GREEN, 8000, 750, 2500, 10000, 4000);
    game.board[32] = PROPERTY_SPACE(32, "Nallur", GROUP_GREEN, 8300, 780, 2500, 10000, 4000);
    game.board[33] = SIMPLE_SPACE(33, SPACE_INSURANCE, "Ceylinco Insurance");
    game.board[34] = PROPERTY_SPACE(34, "Trincomalee", GROUP_GREEN, 8500, 800, 2500, 10000, 4000);
    game.board[35] = RAILWAY_SPACE(35, "Jaffna Railway Station");
    game.board[36] = SIMPLE_SPACE(36, SPACE_EVENT, "National Event Card");
    game.board[37] = PROPERTY_SPACE(37, "Nuwara Eliya", GROUP_DARK_BLUE, 10000, 1000, 3000, 12000, 5000);
    game.board[38] = SIMPLE_SPACE(38, SPACE_BANK, "Bank of Ceylon");
    game.board[39] = PROPERTY_SPACE(39, "Galle Face", GROUP_DARK_BLUE, 12000, 1200, 3000, 12000, 5000);
    return game;
}

//getting 2 random numbers from dies
DiceRoll rollDice(void) {
    DiceRoll roll;
    roll.die1 = rand() % 6 + 1;
    roll.die2 = rand() % 6 + 1;
    roll.total = roll.die1 + roll.die2;
    roll.isDoubles = roll.die1 == roll.die2;
    return roll;
}

GameState movePlayer(GameState game, int playerId, int squares) {
    int oldPosition = game.players[playerId].position;
    int travelled = oldPosition + squares;

    if (travelled >= BOARD_SIZE) {
        game.players[playerId].cash += GO_REWARD;

        game.players[playerId].passedGoThisRound = 1;
        
        printf("\n");
        printPlayerName(game.players[playerId].strategy);
        printf(" passed GO.\nCollected LKR %d.\nCurrent Balance : LKR %d.\n",
               GO_REWARD, game.players[playerId].cash);
    }

    game.players[playerId].position = travelled % BOARD_SIZE;
    printPlayerName(game.players[playerId].strategy);
    printf(" moves from Square %d to Square %d.\n", oldPosition,
           game.players[playerId].position);
    return game;
}

int isPurchasable(SpaceType type) {
    if (type == SPACE_PROPERTY){
        return 1;
    }

    if (type == SPACE_RAILWAY) {
        return 1;
    }

    if (type == SPACE_UTILITY) {
        return 1;
    }

    return 0;
}

int groupPropertyCount(int group){
    if (group == GROUP_BROWN){
        return 2;
    }
    
    if (group == GROUP_DARK_BLUE){
        return 2;
    }

    return 3;
}

int countOwnedInGroup(GameState game, int playerId, int group) {
    int count = 0;
    int index;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].type == SPACE_PROPERTY) {
            if (game.board[index].group == group) {
                if (game.board[index].owner == playerId) {
                    count++;
                }
            }
        }
    }

    return count;
}


int ownsMonopoly(GameState game, int playerId, int group) {
    return countOwnedInGroup(game, playerId, group) == groupPropertyCount(group);
}

int countOwnedRailways(GameState game, int playerId) {
    int count = 0;
    int index;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].type == SPACE_RAILWAY) {
            if (game.board[index].owner == playerId) {
                count++;
            }
        }
    }

    return count;
}

int countOwnedUtilities(GameState game, int playerId){
    int count = 0;
    int index;
    for(index =0; index < BOARD_SIZE; index++){
        if(game.board[index].type == SPACE_UTILITY) {
            if(game.board[index].owner == playerId){
                count++;
            }
        }
    }

    return count; 
}


int countOwnedAssets(GameState game, int playerId) {
    int count = 0;
    int index;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (isPurchasable(game.board[index].type)) {
            if (game.board[index].owner == playerId) {
                count++;
            }
        }
    }

    return count;
}


int countOwnedProperties(GameState game, int playerId) {
    int count = 0;
    int index;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].type == SPACE_PROPERTY){
            if (game.board[index].owner == playerId) {
                count++;
            }
        }
    }

    return count;
}

int countHotels(GameState game, int playerId) {
    int count = 0;
    int index;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId) {
            if (game.board[index].hotel) {
                count++;
            }
        }
    }

    return count;
}


void printSquareName(GameState game, int squareIndex) {
    printf("%s", game.board[squareIndex].name);
}