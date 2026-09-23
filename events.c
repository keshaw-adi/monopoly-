#include <stdio.h>
#include <stdlib.h>
#include "events.h"
#include "board.h"
#include "finance.h"
#include "players.h"

static int applyPercent(int value, int percent) {
    int result = value * (100 + percent) / 100;
    if (result < 1) {
        result = 1;
    }
    return result;
}

static int isSouthernProperty(int squareIndex) {
    return squareIndex == 26 || squareIndex == 27 || squareIndex == 29;
}

static int isCoastalProperty(int squareIndex) {
    return squareIndex == 6 || squareIndex == 8 || squareIndex == 9 || squareIndex == 16 || squareIndex == 18 || squareIndex == 19 ||
           squareIndex == 26 || squareIndex == 27 || squareIndex == 29 ||
           squareIndex == 39;
}

static void printGroupName(int group) {
    switch (group) {
        case GROUP_BROWN: printf("Colombo Distrcit(mid)"); break;
        case GROUP_LIGHT_BLUE: printf("Colombo District(west)"); break;
        case GROUP_PINK: printf("Colombo District(east)"); break;
        case GROUP_ORANGE: printf("Gampaha District"); break;
        case GROUP_RED: printf("Central Province"); break;
        case GROUP_YELLOW: printf("Southern Province"); break;
        case GROUP_GREEN: printf("Nothern Province"); break;
        case GROUP_DARK_BLUE: printf("Famous"); break;
        default: printf("None"); break;
    }
}

static void printEconomicEventName(int event) {
    switch (event) {
        case ECONOMY_TOURISM_BOOM: printf("Tourism Boom"); break;
        case ECONOMY_FUEL_CRISIS: printf("Fuel Crisis"); break;
        case ECONOMY_HEAVY_MONSOON: printf("Heavy Monsoon"); break;
        case ECONOMY_RECESSION: printf("Economic Recession"); break;
        case ECONOMY_STOCK_BOOM: printf("Stock Market Boom"); break;
        case ECONOMY_HOUSING_PROGRAMME: printf("Government Housing Programme"); break;
        case ECONOMY_FOREIGN_INVESTMENT: printf("Foreign Investment"); break;
        case ECONOMY_POLITICAL_UNREST: printf("Political Unrest"); break;
        default: printf("None"); break;
    }
}

static void printRegulationName(int regulation) {
    switch (regulation) {
        case REGULATION_PROPERTY_TAX: printf("Increase Property Tax"); break;
        case REGULATION_REDUCE_INTEREST: printf("Reduce Loan Interest"); break;
        case REGULATION_HOUSING_SUBSIDY: printf("Housing Subsidy"); break;
        case REGULATION_LUXURY_TAX: printf("Luxury Property Tax"); break;
        case REGULATION_RAILWAY_MODERNIZATION: printf("Railway Modernization"); break;
        case REGULATION_ELECTRICITY_TARIFF: printf("Electricity Tariff Revision"); break;
        case REGULATION_INSURANCE: printf("Insurance Regulation"); break;
        case REGULATION_ANTI_SPECULATION: printf("Anti-Speculation Act"); break;
        default: printf("None"); break;
    }
}

static void printRegionalEventName(int event) {
    switch (event) {
        case REGION_SOUTHERN_TOURISM: printf("Southern Tourism Boom\n(+40%%)"); break;
        case REGION_PORT_CITY: printf("Port City Expansion\n(+25%%)"); break;
        case REGION_IT_GROWTH: printf("IT Industry Growth\n(+20%%)"); break;
        case REGION_NORTHERN_DEVELOPMENT: printf("Northern Development Programme\n(+30%%)"); break;
        case REGION_TEA_EXPORT: printf("Tea Export Boom\n(+35%%)"); break;
        case REGION_AIRPORT_EXPANSION: printf("Airport Expansion\n(+30%%)"); break;
        case REGION_UNIVERSITY_GROWTH: printf("University City Growth\n(+20%%)"); break;
        case REGION_BEACH_POLLUTION: printf("Beach Pollution\n(-30%%)"); break;
        case REGION_FLOOD_DAMAGE: printf("Flood Damage\n(-20%%)"); break;
        case REGION_TRANSPORT_STRIKE: printf("Transport Strike\n(-40%%)"); break;
        case REGION_ELECTRICITY_INCREASE: printf("Electricity Tariff Increase\n(+25%%)"); break;
        case REGION_WATER_SHORTAGE: printf("Water Shortage\n(+20%%) ; (-10%%)"); break;
        default: printf("None"); break;
    }
}

static void printCardName(int card) {
    switch (card) {
        case CARD_TOURISM_HYPE: printf("Tourism Hype"); break;
        case CARD_FUEL_SHORTAGE: printf("Fuel Shortage"); break;
        case CARD_HEAVY_FLOODS: printf("Heavy Floods"); break;
        case CARD_POLITICAL_RALLY: printf("Political Rally"); break;
        case CARD_STOCK_MARKET_RISE: printf("Stock Market Rise"); break;
        case CARD_ECONOMIC_DOWNTURN: printf("Economic Downturn"); break;
        case CARD_HOUSING_SUBSIDY: printf("Housing Subsidy"); break;
        case CARD_INTEREST_RATE_CUT: printf("Interest Rate Cut"); break;
        case CARD_INTEREST_RATE_INCREASE: printf("Interest Rate Increase"); break;
        case CARD_TAX_AMNESTY: printf("Tax Amnesty"); break;
        case CARD_POWER_FAILURE: printf("Power Failure"); break;
        case CARD_FOREIGN_FUNDING: printf("Foreign Funding"); break;
        case CARD_PORT_EXPANSION: printf("Port Expansion"); break;
        case CARD_FESTIVAL_SEASON: printf("Festival Season"); break;
        case CARD_LABOUR_STRIKE: printf("Labour Strike"); break;
        case CARD_INSURANCE_DISCOUNT: printf("Insurance Discount"); break;
        case CARD_PROPERTY_REVALUATION: printf("Property Revaluation"); break;
        case CARD_CURRENCY_DEPRECIATION: printf("Currency Depreciation"); break;
        case CARD_GOVERNMENT_GRANT: printf("Government Grant"); break;
        case CARD_NATIONAL_DISASTER: printf("National Disaster"); break;
        default: printf("Unknown Card"); break;
    }
}

static void printDisasterName(int disaster) {
    switch (disaster) {
        case DISASTER_FIRE: printf("Fire"); break;
        case DISASTER_FLOOD: printf("Flood"); break;
        case DISASTER_RIOT: printf("Riot"); break;
        case DISASTER_BUILDING_COLLAPSE: printf("Building Collapse"); break;
        case DISASTER_ELECTRICAL_FAILURE: printf("Electrical Failure"); break;
        default: printf("Disaster"); break;
    }
}

GameState initializeEvents(GameState game) {
    int index;
    game.economy.inflationPercent = 0;
    game.economy.currentLoanRate = 8;
    game.economy.activeEconomicEvent = NO_EVENT;
    game.economy.economicRoundsRemaining = 0;
    game.economy.activeRegulation = NO_EVENT;
    game.economy.regulationRoundsRemaining = 0;
    game.economy.activeRegionalEvent = NO_EVENT;
    game.economy.regionalRoundsRemaining = 0;
    game.economy.boomGroup = NO_GROUP;
    game.economy.declineGroup = NO_GROUP;
    game.economy.marketRoundsRemaining = 0;
    game.economy.deckTop = 0;

    for (index = 0; index < GROUP_COUNT; index++) {
        game.economy.lastAffectedRound[index] = -100;
    }
    for (index = 0; index < NATIONAL_CARD_COUNT; index++) {
        game.economy.nationalDeck[index] = index;
    }
    for (index = NATIONAL_CARD_COUNT - 1; index > 0; index--) {
        int other = rand() % (index + 1);
        int temporary = game.economy.nationalDeck[index];
        game.economy.nationalDeck[index] = game.economy.nationalDeck[other];
        game.economy.nationalDeck[other] = temporary;
    }
    return game;
}

int adjustedPurchasePrice(GameState game, int squareIndex) {
    int value = game.board[squareIndex].purchasePrice;
    if (game.board[squareIndex].group != NO_GROUP &&
        game.board[squareIndex].group == game.economy.boomGroup) {
        value = applyPercent(value, 15);
    }
    return value;
}

int adjustedMarketValue(GameState game, int squareIndex) {
    int value = game.board[squareIndex].marketValue;
    int group = game.board[squareIndex].group;
    int regional = game.economy.activeRegionalEvent;
    int owner = game.board[squareIndex].owner;

    if (group != NO_GROUP && group == game.economy.boomGroup) value = applyPercent(value, 20);
    if (group != NO_GROUP && group == game.economy.declineGroup) value = applyPercent(value, -15);

    if (game.economy.activeEconomicEvent == ECONOMY_TOURISM_BOOM && isSouthernProperty(squareIndex))
        value = applyPercent(value, 15);
    if (game.economy.activeEconomicEvent == ECONOMY_HEAVY_MONSOON && isCoastalProperty(squareIndex))
        value = applyPercent(value, -10);
    if (game.economy.activeEconomicEvent == ECONOMY_RECESSION)
        value = applyPercent(value, -15);
    if (game.economy.activeEconomicEvent == ECONOMY_STOCK_BOOM)
        value = applyPercent(value, 10);
    if (game.economy.activeEconomicEvent == ECONOMY_FOREIGN_INVESTMENT &&
        game.board[squareIndex].type == SPACE_PROPERTY)
        value = applyPercent(value, 20);

    if (regional == REGION_PORT_CITY && (squareIndex == 1 || squareIndex == 3 || squareIndex == 5))
        value = applyPercent(value, 25);
    if (regional == REGION_IT_GROWTH && (squareIndex == 11 || squareIndex == 13 || squareIndex == 14))
        value = applyPercent(value, 20);
    if (regional == REGION_NORTHERN_DEVELOPMENT && (squareIndex == 31 || squareIndex == 32 || squareIndex == 34))
        value = applyPercent(value, 30);
    if (regional == REGION_TEA_EXPORT && squareIndex == 37)
        value = applyPercent(value, 35);
    if (regional == REGION_UNIVERSITY_GROWTH && (squareIndex == 21 || squareIndex == 23))
        value = applyPercent(value, 20);
    if (regional == REGION_FLOOD_DAMAGE && isCoastalProperty(squareIndex))
        value = applyPercent(value, -20);
    if (regional == REGION_WATER_SHORTAGE &&
        (squareIndex == 6 || squareIndex == 8 || squareIndex == 9))
        value = applyPercent(value, -10);

    if (owner >= 0 && owner < PLAYER_COUNT) {
        if (game.players[owner].activeCard == CARD_STOCK_MARKET_RISE)
            value = applyPercent(value, 10);
        if (game.players[owner].activeCard == CARD_ECONOMIC_DOWNTURN)
            value = applyPercent(value, -15);
        if (game.players[owner].activeCard == CARD_FOREIGN_FUNDING &&
            game.board[squareIndex].type == SPACE_PROPERTY)
            value = applyPercent(value, 15);
        if (game.players[owner].activeCard == CARD_PORT_EXPANSION &&
            game.board[squareIndex].type == SPACE_RAILWAY)
            value = applyPercent(value, 20);
        if (game.players[owner].activeCard == CARD_PROPERTY_REVALUATION &&
            group == game.players[owner].cardTargetGroup)
            value = applyPercent(value, 15);
    }
    return value;
}

int adjustedRentBase(GameState game, int squareIndex) {
    int value = game.board[squareIndex].baseRent;
    int group = game.board[squareIndex].group;
    int regional = game.economy.activeRegionalEvent;

    if (group != NO_GROUP && group == game.economy.boomGroup) value = applyPercent(value, 25);
    if (group != NO_GROUP && group == game.economy.declineGroup) value = applyPercent(value, -20);
    if (game.economy.activeEconomicEvent == ECONOMY_RECESSION) value = applyPercent(value, -10);
    if (game.economy.activeEconomicEvent == ECONOMY_POLITICAL_UNREST && game.board[squareIndex].hotel)
        value = applyPercent(value, -50);

    if (regional == REGION_SOUTHERN_TOURISM && isSouthernProperty(squareIndex)) value = applyPercent(value, 40);
    if (regional == REGION_AIRPORT_EXPANSION && (squareIndex == 16 || squareIndex == 18 || squareIndex == 19))
        value = applyPercent(value, 30);
    if (regional == REGION_BEACH_POLLUTION && isSouthernProperty(squareIndex)) value = applyPercent(value, -30);
    return value;
}

int adjustedHouseCost(GameState game, int playerId, int squareIndex) {
    int value = game.board[squareIndex].houseCost;
    if (game.board[squareIndex].group == game.economy.boomGroup) value = applyPercent(value, 10);
    if (game.economy.activeEconomicEvent == ECONOMY_FUEL_CRISIS) value = applyPercent(value, 20);
    if (game.economy.activeEconomicEvent == ECONOMY_HOUSING_PROGRAMME) value = applyPercent(value, -25);
    if (game.economy.activeRegulation == REGULATION_HOUSING_SUBSIDY) value = applyPercent(value, -30);
    if (game.players[playerId].activeCard == CARD_HOUSING_SUBSIDY) value = applyPercent(value, -30);
    if (game.players[playerId].activeCard == CARD_CURRENCY_DEPRECIATION) value = applyPercent(value, 10);
    return value;
}

int adjustedHotelCost(GameState game, int playerId, int squareIndex) {
    int value = game.board[squareIndex].hotelCost;
    if (game.board[squareIndex].group == game.economy.boomGroup) value = applyPercent(value, 10);
    if (game.economy.activeEconomicEvent == ECONOMY_FUEL_CRISIS) value = applyPercent(value, 20);
    if (game.players[playerId].activeCard == CARD_CURRENCY_DEPRECIATION) value = applyPercent(value, 10);
    return value;
}

int adjustedMortgageValue(GameState game, int squareIndex) {
    int value = game.board[squareIndex].mortgageValue;
    if (game.board[squareIndex].group != NO_GROUP &&
        game.board[squareIndex].group == game.economy.boomGroup)
        value = applyPercent(value, 15);
    if (game.board[squareIndex].group != NO_GROUP &&
        game.board[squareIndex].group == game.economy.declineGroup)
        value = applyPercent(value, -10);
    return value;
}

int adjustedInsurancePremium(GameState game, int playerId, int amount) {
    int value = amount;
    if (game.economy.activeEconomicEvent == ECONOMY_HEAVY_MONSOON) value = applyPercent(value, 20);
    if (game.economy.activeRegulation == REGULATION_INSURANCE) value = applyPercent(value, -15);
    if (game.players[playerId].activeCard == CARD_INSURANCE_DISCOUNT) value = applyPercent(value, -20);
    return value;
}

int effectiveLoanRate(GameState game, int playerId) {
    int rate = game.economy.currentLoanRate;
    if (game.economy.activeEconomicEvent == ECONOMY_RECESSION) rate = applyPercent(rate, 15);
    if (game.economy.activeEconomicEvent == ECONOMY_STOCK_BOOM) rate = applyPercent(rate, -10);
    if (game.economy.activeRegulation == REGULATION_REDUCE_INTEREST) rate -= 2;
    if (game.players[playerId].activeCard == CARD_INTEREST_RATE_CUT) rate -= 2;
    if (game.players[playerId].activeCard == CARD_INTEREST_RATE_INCREASE) rate += 2;
    if (rate < 1) rate = 1;
    return rate;
}

int constructionIsSuspended(GameState game, int playerId) {
    return game.players[playerId].activeCard == CARD_LABOUR_STRIKE &&
           game.players[playerId].cardRoundsRemaining > 0;
}

GameState causeDisaster(GameState game) {
    int developed[BOARD_SIZE];
    int count = 0;
    int index;
    int squareIndex;
    int owner;
    int disaster = rand() % 5;
    int repairCost;
    int compensation = 0;
    int ownerCost;
    PaymentResult result;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].type == SPACE_PROPERTY &&
            (game.board[index].houses > 0 || game.board[index].hotel) &&
            game.board[index].owner != BANK_OWNER) {
            developed[count] = index;
            count++;
        }
    }

    printf("\n");
    printDisasterName(disaster);
    printf(" occurred.\n");
    if (count == 0) {
        printf("No developed property was available to be affected.\n");
        return game;
    }

    squareIndex = developed[rand() % count];
    owner = game.board[squareIndex].owner;
    repairCost = game.board[squareIndex].houses * game.board[squareIndex].houseCost / 5;
    if (game.board[squareIndex].hotel) repairCost += game.board[squareIndex].hotelCost / 4;
    if (repairCost < 500) repairCost = 500;

    game.board[squareIndex].damaged = 1;
    game.board[squareIndex].repairCost = repairCost;
    printf("Affected Property : %s.\n", game.board[squareIndex].name);

    if (game.board[squareIndex].insurance == INSURANCE_BASIC &&
        (disaster == DISASTER_FIRE || disaster == DISASTER_FLOOD)) {
        compensation = repairCost * 80 / 100;
    }
    if (game.board[squareIndex].insurance == INSURANCE_COMPREHENSIVE &&
        (disaster == DISASTER_FIRE || disaster == DISASTER_FLOOD || disaster == DISASTER_RIOT)) {
        compensation = repairCost;
    }
    if (game.board[squareIndex].insurance == INSURANCE_BUSINESS && game.board[squareIndex].hotel) {
        compensation = repairCost + adjustedRentBase(game, squareIndex) * 5;
    }

    if (compensation > 0) {
        game.players[owner].cash += compensation;
        printf("Insurance Claim Approved.\nCompensation Paid : LKR %d.\n", compensation);
    } else {
        printf("No applicable insurance compensation.\n");
    }

    ownerCost = repairCost - compensation;
    if (ownerCost < 0) ownerCost = 0;
    result = transferPayment(game, owner, BANK_OWNER, ownerCost);
    game = result.game;
    if (result.paid && !game.players[owner].bankrupt) {
        game.board[squareIndex].damaged = 0;
        game.board[squareIndex].repairCost = 0;
        printf("Repairs completed for LKR %d.\n", ownerCost);
    } else if (!game.players[owner].bankrupt) {
        game.players[owner].hasExperiencedLoss = 1;
        printf("The property remains closed until repairs can be paid.\n");
    }
    return game;
}

GameState drawNationalCard(GameState game, int playerId) {
    int card = game.economy.nationalDeck[game.economy.deckTop];
    int index;
    int target;
    game.economy.deckTop = (game.economy.deckTop + 1) % NATIONAL_CARD_COUNT;
    game.players[playerId].activeCard = card;
    game.players[playerId].cardTargetGroup = NO_GROUP;
    game.players[playerId].cardRoundsRemaining = 15;

    if (card == CARD_TOURISM_HYPE || card == CARD_FUEL_SHORTAGE || card == CARD_FESTIVAL_SEASON)
        game.players[playerId].cardRoundsRemaining = 5;
    if (card == CARD_POWER_FAILURE) game.players[playerId].cardRoundsRemaining = 3;
    if (card == CARD_POLITICAL_RALLY || card == CARD_LABOUR_STRIKE)
        game.players[playerId].cardRoundsRemaining = 2;

    printf("\nNational Event Card\n");
    printPlayerName(game.players[playerId].strategy);
    printf(" drew ");
    printCardName(card);
    printf(".\n");

    if (card == CARD_HEAVY_FLOODS || card == CARD_NATIONAL_DISASTER) {
        game = causeDisaster(game);
    } else if (card == CARD_POLITICAL_RALLY) {
        target = rand() % BOARD_SIZE;
        while (game.board[target].type != SPACE_PROPERTY) target = rand() % BOARD_SIZE;
        game.board[target].closedRounds = 2;
        printf("%s is closed for 2 rounds.\n", game.board[target].name);
    } else if (card == CARD_TAX_AMNESTY) {
        for (index = 0; index < PLAYER_COUNT; index++) {
            if (!game.players[index].bankrupt) game.players[index].cash += 2000;
        }
        printf("Each solvent player received LKR 2,000.\n");
    } else if (card == CARD_PROPERTY_REVALUATION) {
        int group = rand() % GROUP_COUNT;
        game.players[playerId].cardTargetGroup = group;
        printf("The ");
        printGroupName(group);
        printf(" group appreciated by 15%%.\n");
    } else if (card == CARD_GOVERNMENT_GRANT) {
        target = rand() % PLAYER_COUNT;
        while (game.players[target].bankrupt) target = rand() % PLAYER_COUNT;
        game.players[target].cash += 5000;
        printPlayerName(game.players[target].strategy);
        printf(" received LKR 5,000.\n");
    }
    return game;
}

static GameState applyInflation(GameState game) {
    int rates[6] = {-3, 0, 2, 5, 8, 12};
    int index;
    int rate = rates[rand() % 6];
    game.economy.inflationPercent = rate;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (isPurchasable(game.board[index].type)) {
            game.board[index].purchasePrice = applyPercent(game.board[index].purchasePrice, rate);
            game.board[index].marketValue = applyPercent(game.board[index].marketValue, rate);
            game.board[index].mortgageValue = applyPercent(game.board[index].mortgageValue, rate);
            if (game.board[index].baseRent > 0)
                game.board[index].baseRent = applyPercent(game.board[index].baseRent, rate);
            if (game.board[index].houseCost > 0)
                game.board[index].houseCost = applyPercent(game.board[index].houseCost, rate);
            if (game.board[index].hotelCost > 0)
                game.board[index].hotelCost = applyPercent(game.board[index].hotelCost, rate);
        }
    }

    if (rate <= 0) game.economy.currentLoanRate = 8;
    else if (rate <= 2) game.economy.currentLoanRate = 8;
    else if (rate <= 5) game.economy.currentLoanRate = 10;
    else if (rate <= 8) game.economy.currentLoanRate = 12;
    else game.economy.currentLoanRate = 15;

    printf("\nInflation Review\nInflation rate is %+d%%. Financial values were compounded.\n", rate);
    return game;
}

static int chooseEligibleGroup(GameState game, int excludedGroup) {
    int attempt;
    int group;
    for (attempt = 0; attempt < 100; attempt++) {
        group = rand() % GROUP_COUNT;
        if (group != excludedGroup &&
            game.currentRound - game.economy.lastAffectedRound[group] >= 30) {
            return group;
        }
    }
    for (group = 0; group < GROUP_COUNT; group++) {
        if (group != excludedGroup) return group;
    }
    return GROUP_BROWN;
}

static GameState reviewPropertyMarket(GameState game) {
    int boom = chooseEligibleGroup(game, NO_GROUP);
    int decline = chooseEligibleGroup(game, boom);
    game.economy.boomGroup = boom;
    game.economy.declineGroup = decline;
    game.economy.marketRoundsRemaining = 10;
    game.economy.lastAffectedRound[boom] = game.currentRound;
    game.economy.lastAffectedRound[decline] = game.currentRound;
    printf("\nProperty Market Review\nMarket Boom : ");
    printGroupName(boom);
    printf(".\nMarket Decline : ");
    printGroupName(decline);
    printf(".\n");
    return game;
}

GameState processScheduledEvents(GameState game) {
    if (game.currentRound % 10 == 0) {
        game = applyInflation(game);
        game = reviewPropertyMarket(game);
        game = causeDisaster(game);
    }
    if (game.currentRound % 15 == 0) {
        game.economy.activeEconomicEvent = rand() % 8;
        game.economy.economicRoundsRemaining = 15;
        printf("\nEconomic Event\n");
        printEconomicEventName(game.economy.activeEconomicEvent);
        printf(" is active for 15 rounds.\n");

        game.economy.activeRegionalEvent = rand() % 12;
        game.economy.regionalRoundsRemaining = 15;
        printf("\nRegional Development\n");
        printRegionalEventName(game.economy.activeRegionalEvent);
        printf(" is active for 15 rounds.\n");
    }
    if (game.currentRound % 20 == 0) {
        game.economy.activeRegulation = rand() % 8;
        game.economy.regulationRoundsRemaining = 20;
        printf("\nGovernment Regulation\n");
        printRegulationName(game.economy.activeRegulation);
        printf(" is active for 20 rounds.\n");
    }
    return game;
}

GameState processEventTimers(GameState game) {
    int index;
    for (index = 0; index < PLAYER_COUNT; index++) {
        if (game.players[index].cardRoundsRemaining > 0) {
            game.players[index].cardRoundsRemaining--;
            if (game.players[index].cardRoundsRemaining == 0) {
                game.players[index].activeCard = NO_EVENT;
                game.players[index].cardTargetGroup = NO_GROUP;
            }
        }
    }
    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].closedRounds > 0) game.board[index].closedRounds--;
    }

    if (game.economy.economicRoundsRemaining > 0) {
        game.economy.economicRoundsRemaining--;
        if (game.economy.economicRoundsRemaining == 0) game.economy.activeEconomicEvent = NO_EVENT;
    }
    if (game.economy.regulationRoundsRemaining > 0) {
        game.economy.regulationRoundsRemaining--;
        if (game.economy.regulationRoundsRemaining == 0) game.economy.activeRegulation = NO_EVENT;
    }
    if (game.economy.regionalRoundsRemaining > 0) {
        game.economy.regionalRoundsRemaining--;
        if (game.economy.regionalRoundsRemaining == 0) game.economy.activeRegionalEvent = NO_EVENT;
    }
    if (game.economy.marketRoundsRemaining > 0) {
        game.economy.marketRoundsRemaining--;
        if (game.economy.marketRoundsRemaining == 0) {
            game.economy.boomGroup = NO_GROUP;
            game.economy.declineGroup = NO_GROUP;
        }
    }
    return game;
}

void printMarketConditions(GameState game) {
    printf("\n=========================================\n");
    printf("Current Market Conditions\n");
    printf("=========================================\n\n");
    printf("Market Boom : \n------------------\n");
    printGroupName(game.economy.boomGroup);
    printf("  (+20%%) \nRounds Remaining : %d\n\n", game.economy.marketRoundsRemaining);
    printf("Market Decline :\n----------------\n ");
    printGroupName(game.economy.declineGroup);
    printf("  (-15%%)\n Rounds Remaining : %d\n\n", game.economy.marketRoundsRemaining);
    printf("Regional Development :\n-----------------------\n ");
    printRegionalEventName(game.economy.activeRegionalEvent);
    printf(" (%d Rounds Remaining)\n", game.economy.regionalRoundsRemaining);
    printf("Inflation\n----------------\n %+d%%\n\n", game.economy.inflationPercent);
    printf("Current Loan Interest \n-----------------------------\n%d%%\n\n", game.economy.currentLoanRate);
    printf("=========================================\n\n");

}
