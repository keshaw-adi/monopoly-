#include <stdlib.h>
#include <time.h>
#include "game.h"

int main(void) {
    GameState game;
    srand((unsigned int)time(0));
    game = createGame();
    game = runGame(game);
    if (game.gameOver ==1){
        return 0;
    }else{
        return 1;
    }
}
