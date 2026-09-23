#ifndef TYPES_H
#define TYPES_H

#define BOARD_SIZE 40
#define PLAYER_COUNT 4
#define GROUP_COUNT 8
#define NATIONAL_CARD_COUNT 20
#define MAX_ROUNDS 500
#define STARTING_CASH 30000
#define GO_REWARD 2000
#define BAIL_AMOUNT 300
#define BASE_INCOME_TAX_RATE 15
#define BASE_COMMUNITY_FUND_RATE 10
#define BANK_OWNER -1
#define NO_GROUP -1
#define NO_EVENT -1
#define NAME_LENGTH 64

typedef enum {
    SPACE_GO,
    SPACE_PROPERTY,
    SPACE_EVENT,
    SPACE_COMMUNITY_FUND,
    SPACE_TAX,
    SPACE_RAILWAY,
    SPACE_JAIL,
    SPACE_UTILITY,
    SPACE_INSURANCE,
    SPACE_FREE_PARKING,
    SPACE_GO_TO_JAIL,
    SPACE_BANK
} SpaceType;

typedef enum {
    STRATEGY_AGGRESSIVE,
    STRATEGY_CONSERVATIVE,
    STRATEGY_RISK_TAKER,
    STRATEGY_OPPORTUNISTIC
} Strategy;

typedef enum {
    GROUP_BROWN,
    GROUP_LIGHT_BLUE,
    GROUP_PINK,
    GROUP_ORANGE,
    GROUP_RED,
    GROUP_YELLOW,
    GROUP_GREEN,
    GROUP_DARK_BLUE
} PropertyGroup;

typedef enum {
    INSURANCE_NONE,
    INSURANCE_BASIC,
    INSURANCE_COMPREHENSIVE,
    INSURANCE_BUSINESS
} InsuranceType;

typedef enum {
    ECONOMY_TOURISM_BOOM,
    ECONOMY_FUEL_CRISIS,
    ECONOMY_HEAVY_MONSOON,
    ECONOMY_RECESSION,
    ECONOMY_STOCK_BOOM,
    ECONOMY_HOUSING_PROGRAMME,
    ECONOMY_FOREIGN_INVESTMENT,
    ECONOMY_POLITICAL_UNREST
} EconomicEvent;

typedef enum {
    REGULATION_PROPERTY_TAX,
    REGULATION_REDUCE_INTEREST,
    REGULATION_HOUSING_SUBSIDY,
    REGULATION_LUXURY_TAX,
    REGULATION_RAILWAY_MODERNIZATION,
    REGULATION_ELECTRICITY_TARIFF,
    REGULATION_INSURANCE,
    REGULATION_ANTI_SPECULATION
} GovernmentRegulation;

typedef enum {
    REGION_SOUTHERN_TOURISM,
    REGION_PORT_CITY,
    REGION_IT_GROWTH,
    REGION_NORTHERN_DEVELOPMENT,
    REGION_TEA_EXPORT,
    REGION_AIRPORT_EXPANSION,
    REGION_UNIVERSITY_GROWTH,
    REGION_BEACH_POLLUTION,
    REGION_FLOOD_DAMAGE,
    REGION_TRANSPORT_STRIKE,
    REGION_ELECTRICITY_INCREASE,
    REGION_WATER_SHORTAGE
} RegionalEvent;

typedef enum {
    CARD_TOURISM_HYPE,
    CARD_FUEL_SHORTAGE,
    CARD_HEAVY_FLOODS,
    CARD_POLITICAL_RALLY,
    CARD_STOCK_MARKET_RISE,
    CARD_ECONOMIC_DOWNTURN,
    CARD_HOUSING_SUBSIDY,
    CARD_INTEREST_RATE_CUT,
    CARD_INTEREST_RATE_INCREASE,
    CARD_TAX_AMNESTY,
    CARD_POWER_FAILURE,
    CARD_FOREIGN_FUNDING,
    CARD_PORT_EXPANSION,
    CARD_FESTIVAL_SEASON,
    CARD_LABOUR_STRIKE,
    CARD_INSURANCE_DISCOUNT,
    CARD_PROPERTY_REVALUATION,
    CARD_CURRENCY_DEPRECIATION,
    CARD_GOVERNMENT_GRANT,
    CARD_NATIONAL_DISASTER
} NationalCard;

typedef enum {
    DISASTER_FIRE,
    DISASTER_FLOOD,
    DISASTER_RIOT,
    DISASTER_BUILDING_COLLAPSE,
    DISASTER_ELECTRICAL_FAILURE
} DisasterType;

typedef struct {
    int die1;
    int die2;
    int total;
    int isDoubles;
} DiceRoll;

typedef struct {
    int index;
    SpaceType type;
    char name[NAME_LENGTH];
    int group;
    int purchasePrice;
    int marketValue;
    int baseRent;
    int houseCost;
    int hotelCost;
    int mortgageValue;
    int owner;
    int mortgaged;
    int loanLocked;
    int houses;
    int hotel;
    InsuranceType insurance;
    int insuranceRounds;
    int age;
    int depreciationPercent;
    int buildingCondition;
    int maintenanceIgnoredRounds;
    int structurallyDamaged;
    int damaged;
    int repairCost;
    int closedRounds;
} Space;

typedef struct {
    Strategy strategy;
    int cash;
    int position;
    int jailed;
    int jailTurns;
    int bankrupt;
    int hasExperiencedLoss;
    int loanBalance;
    int loanRate;
    int loanRoundsRemaining;
    int mortgageLiability;
    int insuranceClaimsReceivable;
    int taxesDue;
    int activeCard;
    int cardTargetGroup;
    int cardRoundsRemaining;
    int passedGoThisRound;
} Player;

typedef struct {
    int inflationPercent;
    int currentLoanRate;
    int activeEconomicEvent;
    int economicRoundsRemaining;
    int activeRegulation;
    int regulationRoundsRemaining;
    int activeRegionalEvent;
    int regionalRoundsRemaining;
    int boomGroup;
    int declineGroup;
    int marketRoundsRemaining;
    int lastAffectedRound[GROUP_COUNT];
    int nationalDeck[NATIONAL_CARD_COUNT];
    int deckTop;
} EconomyState;

typedef struct {
    Space board[BOARD_SIZE];
    Player players[PLAYER_COUNT];
    EconomyState economy;
    int turnOrder[PLAYER_COUNT];
    int currentRound;
    int activePlayers;
    int gameOver;
    int winner;
} GameState;

typedef struct {
    GameState game;
    int paid;
} PaymentResult;

#endif
