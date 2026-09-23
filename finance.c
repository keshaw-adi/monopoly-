#include <stdio.h>
#include "finance.h"
#include "board.h"
#include "events.h"
#include "players.h"

//conservative always maintain the max cash reserve. opertunistic is mid.Aggesive 
//spend saving cash only for the rent.so max rent price assigned to him.
static int minimumCashReserve(Strategy strategy) {
    if (strategy == STRATEGY_CONSERVATIVE) return 10000;
    if (strategy == STRATEGY_OPPORTUNISTIC) return 6000;
    if (strategy == STRATEGY_AGGRESSIVE) return 1200;
    return 0;
}

static int rentConditionPercent(int condition) {
    if (condition >= 90) return 100;
    if (condition >= 75) return 90;
    if (condition >= 50) return 75;
    if (condition >= 25) return 50;
    return 0;
}

int calculateRent(GameState game, int squareIndex, int diceTotal) {
    Space space = game.board[squareIndex];
    int owner = space.owner;
    int rent = 0;
    int owned;

    if (owner == BANK_OWNER || space.mortgaged || space.damaged || space.closedRounds > 0)
        return 0;

    if (space.type == SPACE_PROPERTY) {
        int multiplier = 1;
        if (space.houses == 1) multiplier = 2;
        if (space.houses == 2) multiplier = 3;
        if (space.houses == 3) multiplier = 5;
        if (space.houses == 4) multiplier = 7;
        if (space.hotel) multiplier = 10;
        rent = adjustedRentBase(game, squareIndex) * multiplier;
        if (space.houses > 0 || space.hotel)
            rent = rent * rentConditionPercent(space.buildingCondition) / 100;
        if (space.hotel && game.players[owner].activeCard == CARD_TOURISM_HYPE)
            rent *= 2;
        if (space.hotel && game.players[owner].activeCard == CARD_FESTIVAL_SEASON)
            rent = rent * 150 / 100;
    } else if (space.type == SPACE_RAILWAY) {
        owned = countOwnedRailways(game, owner);
        if (owned == 1) rent = 250;
        if (owned == 2) rent = 500;
        if (owned == 3) rent = 1000;
        if (owned == 4) rent = 2000;
        rent = rent * space.baseRent / 250;
        if (game.economy.activeEconomicEvent == ECONOMY_FUEL_CRISIS) rent *= 2;
        if (game.players[owner].activeCard == CARD_FUEL_SHORTAGE) rent *= 2;
        if (game.economy.activeRegulation == REGULATION_RAILWAY_MODERNIZATION)
            rent = rent * 125 / 100;
        if (game.economy.activeRegionalEvent == REGION_TRANSPORT_STRIKE)
            rent = rent * 60 / 100;
    } else if (space.type == SPACE_UTILITY) {
        owned = countOwnedUtilities(game, owner);
        if(owned == 2){
            rent = diceTotal* 10;
        }else{
            rent = diceTotal * 4;
        }
        rent = rent * space.baseRent / 100;
        if (game.players[owner].activeCard == CARD_POWER_FAILURE) rent /= 2;
        if (game.economy.activeRegulation == REGULATION_ELECTRICITY_TARIFF)
            rent = rent * 120 / 100;
        if (game.economy.activeRegionalEvent == REGION_ELECTRICITY_INCREASE && squareIndex == 12)
            rent = rent * 125 / 100;
        if (game.economy.activeRegionalEvent == REGION_WATER_SHORTAGE && squareIndex == 28)
            rent = rent * 120 / 100;
    }
    return rent;
}

static GameState liquidateForDebt(GameState game, int playerId, int requiredCash) {
    int index;
    for (index = 0; index < BOARD_SIZE && game.players[playerId].cash < requiredCash; index++) {
        if (game.board[index].owner == playerId &&
            (game.board[index].houses > 0 || game.board[index].hotel)) {
            int proceeds = game.board[index].houses * game.board[index].houseCost / 2;
            if (game.board[index].hotel) proceeds += game.board[index].hotelCost / 2;
            game.players[playerId].cash += proceeds;
            game.board[index].houses = 0;
            game.board[index].hotel = 0;
            game.board[index].insurance = INSURANCE_NONE;
            game.board[index].insuranceRounds = 0;
            printf("%s developments were sold for LKR %d.\n",
                   game.board[index].name, proceeds);
        }
    }
    for (index = 0; index < BOARD_SIZE && game.players[playerId].cash < requiredCash; index++) {
        if (game.board[index].owner == playerId && isPurchasable(game.board[index].type) &&
            !game.board[index].mortgaged && !game.board[index].loanLocked) {
            int proceeds = adjustedMortgageValue(game, index);
            game.board[index].mortgaged = 1;
            game.players[playerId].cash += proceeds;
            game.players[playerId].mortgageLiability += proceeds;
            printf("%s was mortgaged for LKR %d.\n", game.board[index].name, proceeds);
        }
    }
    return game;
}

GameState declareBankruptcy(GameState game, int playerId, int creditorId) {
    int index;
    if (game.players[playerId].bankrupt) return game;

    printf("\n");
    printPlayerName(game.players[playerId].strategy);
    printf(" has been declared bankrupt.\n");
    game.players[playerId].bankrupt = 1;
    game.activePlayers--;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId) {
            int returnsToBank = game.board[index].mortgaged || game.board[index].loanLocked;
            game.board[index].houses = 0;
            game.board[index].hotel = 0;
            game.board[index].insurance = INSURANCE_NONE;
            game.board[index].insuranceRounds = 0;
            game.board[index].loanLocked = 0;
            game.board[index].damaged = 0;
            game.board[index].closedRounds = 0;
            if (creditorId >= 0 && creditorId < PLAYER_COUNT && game.players[creditorId].bankrupt ==0 && returnsToBank ==0) {
                game.board[index].owner = creditorId;
            } else {
                game.board[index].owner = BANK_OWNER;
                game.board[index].mortgaged = 0;
                game = runAuction(game, index);
            }
        }
    }
    game.players[playerId].cash = 0;
    game.players[playerId].loanBalance = 0;
    game.players[playerId].loanRoundsRemaining = 0;
    game.players[playerId].mortgageLiability = 0;
    game.players[playerId].taxesDue = 0;
    printf("Remaining assets transferred to the Bank .\n");
    return game;
}

PaymentResult transferPayment(GameState game, int payerId, int receiverId, int amount) {
    PaymentResult result;
    result.game = game;
    result.paid = 1;
    if (amount <= 0 || game.players[payerId].bankrupt) return result;

    if (result.game.players[payerId].cash < amount)
        result.game = liquidateForDebt(result.game, payerId, amount);

    if (result.game.players[payerId].cash >= amount) {
        result.game.players[payerId].cash -= amount;
        if (receiverId >= 0 && receiverId < PLAYER_COUNT &&
            !result.game.players[receiverId].bankrupt) {
            result.game.players[receiverId].cash += amount;
        }
    } else {
        int remainingCash = result.game.players[payerId].cash;
        if (receiverId >= 0 && receiverId < PLAYER_COUNT &&
            !result.game.players[receiverId].bankrupt) {
            result.game.players[receiverId].cash += remainingCash;
        }
        result.game.players[payerId].cash = 0;
        result.game = declareBankruptcy(result.game, payerId, receiverId);
        result.paid = 0;
    }
    return result;
}

GameState payRent(GameState game, int playerId, int squareIndex, int diceTotal) {
    int owner = game.board[squareIndex].owner;
    int rent = calculateRent(game, squareIndex, diceTotal);
    PaymentResult result;
    if (owner == BANK_OWNER || owner == playerId || game.players[owner].bankrupt) return game;

    printPlayerName(game.players[playerId].strategy);
    printf(" landed on %s.\n\n", game.board[squareIndex].name);
    if (rent == 0) {
        printf("No rent is collected because the asset is mortgaged, closed, or damaged.\n");
        return game;
    }

    result = transferPayment(game, playerId, owner, rent);
    game = result.game;
    printf("Rent paid : LKR  ");
    if(result.paid){
        printf("%d", rent);
    }else{
        printf("0");
    }
    printf(".\nOwner :");
    printPlayerName(game.players[owner].strategy);
    printf(".\n");
    return game;
}   

int currentIncomeTaxBasisPoints(GameState game) {
    int rate = BASE_INCOME_TAX_RATE * 100;
    rate = rate * (100 + game.economy.inflationPercent) / 100;
    if (game.economy.activeRegulation == REGULATION_PROPERTY_TAX)
        rate = rate * 150 / 100;
    if (rate < 0) rate = 0;
    return rate;
}

int currentCommunityFundBasisPoints(GameState game) {
    int rate = BASE_COMMUNITY_FUND_RATE * 100;
    rate = rate * (100 + game.economy.inflationPercent) / 100;
    if (rate < 0) rate = 0;
    return rate;
}

GameState chargeIncomeTax(GameState game, int playerId) {
    int rate = currentIncomeTaxBasisPoints(game);
    int taxableCash = game.players[playerId].cash;
    int tax = taxableCash * rate / 10000;
    PaymentResult result;
    game.players[playerId].taxesDue += tax;
    result = transferPayment(game, playerId, BANK_OWNER, tax);
    game = result.game;
    if (result.paid) game.players[playerId].taxesDue -= tax;
    printPlayerName(game.players[playerId].strategy);
    printf(" paid Income Tax of LKR %d at %d.%02d%% on LKR %d cash.\n",
           result.paid ? tax : 0, rate / 100, rate % 100, taxableCash);
    return game;
}

GameState chargeCommunityFundTax(GameState game, int playerId) {
    int taxablePropertyAssets = 0;
    int rate = currentCommunityFundBasisPoints(game);
    int tax;
    int index;
    PaymentResult result;

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].type == SPACE_PROPERTY &&
            game.board[index].owner == playerId) {
            taxablePropertyAssets += adjustedMarketValue(game, index);
        }
    }

    tax = taxablePropertyAssets * rate / 10000;
    game.players[playerId].taxesDue += tax;
    result = transferPayment(game, playerId, BANK_OWNER, tax);
    game = result.game;
    if (result.paid) game.players[playerId].taxesDue -= tax;

    printPlayerName(game.players[playerId].strategy);
    printf(" paid LKR %d to the Community Development Fund.\n",
           result.paid ? tax : 0);
    printf("Taxable residential property assets : LKR %d.\n",
           taxablePropertyAssets);
    printf("Effective Community Fund rate : %d.%02d%%.\n",
           rate / 100, rate % 100);
    return game;
}

int calculateMaximumLoan(GameState game, int playerId) {
    int collateral = 0;
    int index;
    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId && isPurchasable(game.board[index].type) &&
            !game.board[index].mortgaged) {
            collateral += adjustedMortgageValue(game, index);
        }
    }
    return collateral * 75 / 100;
}

static GameState lockLoanCollateral(GameState game, int playerId) {
    int index;
    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId && isPurchasable(game.board[index].type) &&
            !game.board[index].mortgaged) {
            game.board[index].loanLocked = 1;
        }
    }
    return game;
}

static GameState unlockLoanCollateral(GameState game, int playerId) {
    int index;
    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId) game.board[index].loanLocked = 0;
    }
    return game;
}

static void printCollateralNames(GameState game, int playerId) {
    int index;
    int first = 1;

    printf("Collaterals : \n");

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId &&
            game.board[index].loanLocked) {

            if (first != 0) {
                printf("\n ");
            }

            printf("%s\n", game.board[index].name);
            first = 0;
        }
    }

    printf("\n");
}
static GameState obtainOrIncreaseLoan(GameState game, int playerId, int amount) {
    if (amount <= 0) return game;
    if (game.players[playerId].loanBalance == 0) {
        game.players[playerId].loanRate = effectiveLoanRate(game, playerId);
        game.players[playerId].loanRoundsRemaining = 20;
        game = lockLoanCollateral(game, playerId);
        printf("\n");
        printPlayerName(game.players[playerId].strategy);
        printf(" obtained a secured loan.\n\nLoan Amount : LKR %d.\n\n",
               amount);
        printCollateralNames(game,playerId);
        printf("\n\nInterest Rate : %d%%\nDuration : 20 Rounds\n\n",game.players[playerId].loanRate);

    } 
    else
    {
        printf("\n");
        printPlayerName(game.players[playerId].strategy);
        printf(" increased the secured loan by LKR %d.\n", amount);
    }
    game.players[playerId].loanBalance += amount;
    game.players[playerId].cash += amount;
    return game;
}

static GameState repayLoan(GameState game, int playerId, int amount) {
    if (amount <= 0) return game;
    if (amount > game.players[playerId].loanBalance) amount = game.players[playerId].loanBalance;
    if (amount > game.players[playerId].cash) amount = game.players[playerId].cash;
    game.players[playerId].cash -= amount;
    game.players[playerId].loanBalance -= amount;
    printPlayerName(game.players[playerId].strategy);
    printf(" repaid LKR %d.\n\nOutstanding Balance : \nLKR %d.\n",
           amount, game.players[playerId].loanBalance);
    if (game.players[playerId].loanBalance == 0) {
        game.players[playerId].loanRoundsRemaining = 0;
        game = unlockLoanCollateral(game, playerId);
    }
    return game;
}

GameState performBankTransaction(GameState game, int playerId) {
    int maximum = calculateMaximumLoan(game, playerId);
    int available = maximum - game.players[playerId].loanBalance;
    int amount = 0;
    Strategy strategy = game.players[playerId].strategy;

    printf("\n");
    printPlayerName(strategy);
    printf(" visits the Bank of Ceylon.\n");

    if (game.players[playerId].loanBalance > 0) {
        if (strategy == STRATEGY_CONSERVATIVE && game.players[playerId].cash > 10000)
            return repayLoan(game, playerId, game.players[playerId].cash - 10000);
        if (strategy == STRATEGY_AGGRESSIVE &&
            game.players[playerId].cash > game.players[playerId].loanBalance * 2)
            return repayLoan(game, playerId, game.players[playerId].loanBalance);
        if (strategy == STRATEGY_RISK_TAKER && available > 0)
            return obtainOrIncreaseLoan(game, playerId, available);
        if (strategy == STRATEGY_OPPORTUNISTIC) {
            if (game.players[playerId].loanRate > 10 && game.players[playerId].cash > 6000)
                return repayLoan(game, playerId, game.players[playerId].cash - 6000);
            if (game.players[playerId].loanRate < 10 && available > 0)
                return obtainOrIncreaseLoan(game, playerId, available / 2);
        }
        printf("No loan transaction was selected.\n");
        return game;
    }

    if (strategy == STRATEGY_RISK_TAKER) amount = maximum;
    if (strategy == STRATEGY_AGGRESSIVE && game.players[playerId].cash < 12000) amount = maximum;
    if (strategy == STRATEGY_CONSERVATIVE && game.players[playerId].cash < 1000) amount = maximum / 2;
    if (strategy == STRATEGY_OPPORTUNISTIC && effectiveLoanRate(game, playerId) < 10 &&
        game.players[playerId].cash < 10000) amount = maximum / 2;
    if (amount > 0) return obtainOrIncreaseLoan(game, playerId, amount);

    printf("No loan was required.\n");
    return game;
}

GameState offerInsurance(GameState game, int playerId) {
    int index;
    int purchased = 0;
    Strategy strategy = game.players[playerId].strategy;
    printf("\n");
    printPlayerName(strategy);
    printf(" visits an insurance company.\n");

    for (index = 0; index < BOARD_SIZE; index++) {
        InsuranceType policy = INSURANCE_NONE;
        int premiumRate = 0;
        int premium;
        if (game.board[index].owner != playerId || game.board[index].type != SPACE_PROPERTY ||
            (game.board[index].houses == 0 && !game.board[index].hotel)) continue;

        if (strategy == STRATEGY_AGGRESSIVE){
            if(game.board[index].hotel){
                policy = INSURANCE_COMPREHENSIVE;
            }
            else{
                policy = INSURANCE_BASIC;
            }   
        }
        if (strategy == STRATEGY_CONSERVATIVE) policy = INSURANCE_COMPREHENSIVE;
        if (strategy == STRATEGY_RISK_TAKER && game.players[playerId].hasExperiencedLoss) {
             if (game.board[index].hotel) {
             policy = INSURANCE_COMPREHENSIVE;
        } else {
            policy = INSURANCE_BASIC;
        }
        }
        if (strategy == STRATEGY_OPPORTUNISTIC &&
            (game.board[index].hotel || adjustedMarketValue(game, index) >= 8000))
            policy = INSURANCE_COMPREHENSIVE;
        if (policy == INSURANCE_NONE) continue;

        if (policy == INSURANCE_BASIC) premiumRate = 5;
        if (policy == INSURANCE_COMPREHENSIVE) premiumRate = 10;
        if (policy == INSURANCE_BUSINESS) premiumRate = 15;
        premium = adjustedMarketValue(game, index) * premiumRate / 100;
        premium = adjustedInsurancePremium(game, playerId, premium);

        if (game.players[playerId].cash >= premium + minimumCashReserve(strategy)) {
            game.players[playerId].cash -= premium;
            game.board[index].insurance = policy;
            game.board[index].insuranceRounds = 20;
            purchased++;
            if (policy == INSURANCE_BASIC) printf("Basic Property Insurance purchased.\n\n");
            if (policy == INSURANCE_COMPREHENSIVE) printf("Comprehensive Insurance purchased.\n\n");
            if (policy == INSURANCE_BUSINESS) printf("Business Interruption Insurance purchased.\n\n");
            printf("Property : %s\n\nPremium : LKR %d.\n\n", game.board[index].name, premium);
        }
    }
    if (!purchased) printf("No suitable policy was purchased.\n");
    return game;
}

static GameState forecloseLoan(GameState game, int playerId) {
    int index;
    printf("\n");
    printPlayerName(game.players[playerId].strategy);
    printf(" has defaulted.\n\nCollateral has been foreclosed.\n\nOutstanding debt cleared.\n\n");
    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId && game.board[index].loanLocked) {
            game.board[index].owner = BANK_OWNER;
            game.board[index].loanLocked = 0;
            game.board[index].houses = 0;
            game.board[index].hotel = 0;
            game.board[index].insurance = INSURANCE_NONE;
            game.board[index].insuranceRounds = 0;
            game = runAuction(game, index);
        }
    }
    game.players[playerId].loanBalance = 0;
    game.players[playerId].loanRoundsRemaining = 0;
    if (countOwnedAssets(game, playerId) == 0 && game.players[playerId].cash <= 0)
        game = declareBankruptcy(game, playerId, BANK_OWNER);
    return game;
}

static GameState attemptAutomaticRepair(GameState game, int squareIndex) {
    int owner = game.board[squareIndex].owner;
    int cost = game.board[squareIndex].repairCost;
    if (owner != BANK_OWNER && game.board[squareIndex].damaged &&
        game.players[owner].cash >= cost) {
        game.players[owner].cash -= cost;
        game.board[squareIndex].damaged = 0;
        game.board[squareIndex].repairCost = 0;
        printf("%s was automatically repaired for LKR %d.\n",
               game.board[squareIndex].name, cost);
    }
    return game;
}

GameState processRoundFinances(GameState game) {
    int playerId;
    int index;
    for (playerId = 0; playerId < PLAYER_COUNT; playerId++) {
        if (game.players[playerId].bankrupt) continue;
        if (game.players[playerId].loanBalance > 0) {
            int interest = game.players[playerId].loanBalance * game.players[playerId].loanRate / 100;
            game.players[playerId].loanBalance += interest;
            game.players[playerId].loanRoundsRemaining--;
            printf("Loan interest for ");
            printPlayerName(game.players[playerId].strategy);
            printf(" : LKR %d (balance LKR %d, %d rounds remain).\n\n", interest,
                   game.players[playerId].loanBalance,
                   game.players[playerId].loanRoundsRemaining);
            if (game.players[playerId].loanRoundsRemaining <= 0)
                game = forecloseLoan(game, playerId);
        }
    }

    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].type != SPACE_PROPERTY) continue;
        game.board[index].age++;

        if (game.board[index].insuranceRounds > 0) {
            game.board[index].insuranceRounds--;
            if (game.board[index].insuranceRounds == 3)
                printf("Insurance policy on %s expires in 3 rounds.\n\n", game.board[index].name);
            if (game.board[index].insuranceRounds == 0) {
                game.board[index].insurance = INSURANCE_NONE;
                printf("Insurance policy on %s has expired.\n\n", game.board[index].name);
            }
        }

        if (game.board[index].age > 50 && game.board[index].age % 5 == 0 &&
            game.board[index].depreciationPercent < 30) {
            game.board[index].depreciationPercent++;
            game.board[index].marketValue = game.board[index].marketValue * 99 / 100;
            printf("Property \n\n%s \n\nhas depreciated by %d%%. \n\nCurrent Value \n\nLKR %d.\n",
                   game.board[index].name, game.board[index].depreciationPercent,
                   game.board[index].marketValue);
        }

        if (game.board[index].houses > 0 || game.board[index].hotel) {
            if (game.board[index].buildingCondition > 0)
                game.board[index].buildingCondition -= 2;
            game.board[index].maintenanceIgnoredRounds++;
            if (game.board[index].maintenanceIgnoredRounds > 20 &&
                !game.board[index].structurallyDamaged) {
                game.board[index].structurallyDamaged = 1;
                game.board[index].marketValue = game.board[index].marketValue * 85 / 100;
                game.board[index].baseRent = game.board[index].baseRent * 75 / 100;
                printf("Structural damage occurred at %s.\n\n", game.board[index].name);
            }
        }
        game = attemptAutomaticRepair(game, index);
    }

    if (game.currentRound % 20 == 0 &&
        game.economy.activeRegulation == REGULATION_LUXURY_TAX) {
        for (index = 0; index < BOARD_SIZE; index++) {
            if (game.board[index].hotel && game.board[index].owner != BANK_OWNER) {
                int owner = game.board[index].owner;
                int tax = adjustedMarketValue(game, index) * 25 / 100;
                PaymentResult result = transferPayment(game, owner, BANK_OWNER, tax);
                game = result.game;
                int paidTax;
                if(result.paid){
                    paidTax = tax;
                }else{
                    paidTax = 0;
                }
                printf("Luxury property Tax on %s : \nLKR %d.\n", game.board[index].name,paidTax);
            }
        }
        }

    return game;
}

GameState performMaintenance(GameState game, int playerId) {
    int index;
    int threshold = 70; 
    Strategy strategy = game.players[playerId].strategy;
    if (strategy == STRATEGY_CONSERVATIVE) threshold = 90;
    if (strategy == STRATEGY_RISK_TAKER) threshold = 25;
    if (strategy == STRATEGY_OPPORTUNISTIC) threshold = 75;

    for (index = 0; index < BOARD_SIZE; index++) {
        int cost = 0;
        if (game.board[index].owner != playerId ||
            (game.board[index].houses == 0 && !game.board[index].hotel)) continue;
        if (game.board[index].buildingCondition >= threshold && !game.board[index].damaged)
            continue;
        if (game.board[index].hotel) cost = game.board[index].hotelCost * 8 / 100;
        else cost = game.board[index].houses * game.board[index].houseCost * 5 / 100;
        if (game.board[index].structurallyDamaged) cost = cost * 150 / 100;
        if (game.players[playerId].cash >= cost + minimumCashReserve(strategy)) {
            game.players[playerId].cash -= cost;
            game.board[index].buildingCondition = 100;
            game.board[index].maintenanceIgnoredRounds = 0;
            printPlayerName(strategy);
            printf(" maintained %s for LKR %d.\n", game.board[index].name, cost);
        }
    }
    return game;
}



GameState renovateOwnedProperty(GameState game, int playerId, int squareIndex) {
    int threshold = 10;
    int cost;
    Strategy strategy = game.players[playerId].strategy;
    if (strategy == STRATEGY_AGGRESSIVE) threshold = 20;
    if (strategy == STRATEGY_RISK_TAKER) threshold = 30;
    if (strategy == STRATEGY_OPPORTUNISTIC) threshold = 15;
    if (game.board[squareIndex].owner != playerId || game.board[squareIndex].type != SPACE_PROPERTY)
        return game;
    if (game.board[squareIndex].depreciationPercent < threshold &&
        !game.board[squareIndex].structurallyDamaged) return game;

    cost = adjustedMarketValue(game, squareIndex) * 10 / 100;
    if (game.board[squareIndex].structurallyDamaged) {
        int replacement = game.board[squareIndex].houses * game.board[squareIndex].houseCost;
        if (game.board[squareIndex].hotel) replacement += game.board[squareIndex].hotelCost;
        cost += replacement * 25 / 100;
    }
    if (game.players[playerId].cash < cost + minimumCashReserve(strategy)) return game;

    game.players[playerId].cash -= cost;
    if (game.board[squareIndex].depreciationPercent > 0) {
        int depreciation = game.board[squareIndex].depreciationPercent;
        game.board[squareIndex].marketValue =
            game.board[squareIndex].marketValue * 100 / (100 - depreciation);
        game.board[squareIndex].baseRent =
            game.board[squareIndex].baseRent * (100 + depreciation) / 100;
    }
    if (game.board[squareIndex].structurallyDamaged) {
        game.board[squareIndex].marketValue = game.board[squareIndex].marketValue * 100 / 85;
        game.board[squareIndex].baseRent = game.board[squareIndex].baseRent * 100 / 75;
    }
    game.board[squareIndex].age = 0;
    game.board[squareIndex].depreciationPercent = 0;
    game.board[squareIndex].buildingCondition = 100;
    game.board[squareIndex].maintenanceIgnoredRounds = 0;
    game.board[squareIndex].structurallyDamaged = 0;
    printf("%s was renovated for LKR %d.\n", game.board[squareIndex].name, cost);
    return game;
}


int calculateNetWorth(GameState game, int playerId) {
    int netWorth;
    int index;
    if (game.players[playerId].bankrupt) return 0;
    netWorth = game.players[playerId].cash + game.players[playerId].insuranceClaimsReceivable;
    for (index = 0; index < BOARD_SIZE; index++) {
        if (game.board[index].owner == playerId) {
            netWorth += adjustedMarketValue(game, index);
            netWorth += game.board[index].houses * game.board[index].houseCost;
            if (game.board[index].hotel) netWorth += game.board[index].hotelCost;
        }
    }
    netWorth -= game.players[playerId].loanBalance;
    netWorth -= game.players[playerId].mortgageLiability;
    netWorth -= game.players[playerId].taxesDue;
    return netWorth;
}