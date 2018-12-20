#include "ai.h"

// bool
typedef int BOOL;
#define TRUE 1
#define FALSE 0

const int DI[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

BOOL InBound(int x, int y) {
  return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

/*
 * YOUR CODE BEGIN
 * 你的代码开始
 */
 
/* 
 * You can define your own struct and variable here
 * 你可以在这里定义你自己的结构体和变量
 */
typedef struct SnextStep{
	int x;
	int y;
	int dir;
	struct SnextStep* next;
} nextStep;

struct Result {
	int evalute;
	struct Command command;
} Result;

int gMeFlag = 0;
const int MAX =  1000;
const int MIN = -1000;
const int LEAF_NODE = 1;


#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) <= (b) ? (a) : (b))

BOOL isGameoOver(char board[BOARD_SIZE][BOARD_SIZE]);
/*
 * 你可以在这里初始化你的AI
 */
 
//The limitation of time is 180s
//The limitation of memory is 350MB
//The maximum number of round is 120

void initAI(int me)
{
	gMeFlag = me;
}

void updateBoard(char board[BOARD_SIZE][BOARD_SIZE], int nextx, int nexty, int meFlag)
{
	int otherFlag = 3 - meFlag;
    // Yak
    if (inBoard(nextx - 1, nexty - 1) && inBoard(nextx + 1, nexty + 1) && board[nextx - 1][nexty - 1] == otherFlag && board[nextx + 1][nexty + 1] == otherFlag)
    {
        board[nextx - 1][nexty - 1] = board[nextx + 1][nexty + 1] = meFlag;
    }
    if (inBoard(nextx + 1, nexty - 1) && inBoard(nextx - 1, nexty + 1) && board[nextx + 1][nexty - 1] == otherFlag && board[nextx - 1][nexty + 1] == otherFlag)
    {
        board[nextx + 1][nexty - 1] = board[nextx - 1][nexty + 1] = meFlag;
    }
	if (nexty - 1 >= 0 && nexty + 1 < BOARD_SIZE && board[nextx][nexty - 1] == otherFlag && board[nextx][nexty + 1] == otherFlag)
	{
		board[nextx][nexty - 1] = board[nextx][nexty + 1] = meFlag;
	}
	if (nextx - 1 >= 0 && nextx + 1 < BOARD_SIZE && board[nextx - 1][nexty] == otherFlag && board[nextx + 1][nexty] == otherFlag)
	{
		board[nextx - 1][nexty] = board[nextx + 1][nexty] = meFlag;
	}
    
    // Mak
    for (int i = 0; i < 8; i++)
    {
		if (inBoard(nextx + 2 * DI[i][0], nexty + 2 * DI[i][1]) &&
			board[nextx + DI[i][0]][nexty + DI[i][1]] == otherFlag && board[nextx + 2 * DI[i][0]][nexty + 2 * DI[i][1]] == meFlag)
		{
			board[nextx + DI[i][0]][nexty + DI[i][1]] = meFlag;
		}
	}		
}

BOOL isGameOver(char board[BOARD_SIZE][BOARD_SIZE])
{
	int count[3] = {0};
	
	for (int x = 0; x < BOARD_SIZE; x++) {
    	for (int y = 0; y < BOARD_SIZE; y++) {
      		count[board[x][y]]++;	
		}
	}
	
	if (count[1] == 0 || count[2] == 0) {
		return TRUE;
	}
	
	return FALSE;
}

int evaluteBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
	int count[3] = {0};
	
	for (int x = 0; x < BOARD_SIZE; x++) {
    	for (int y = 0; y < BOARD_SIZE; y++) {
      		count[board[x][y]]++;	
		}
	}
	
	return count[gMeFlag] - count[3-gMeFlag];
}

struct Result AlphaBeta(int depth, const int player, 
            char board[BOARD_SIZE][BOARD_SIZE], 
			int alpha, int beta) {
	if (LEAF_NODE == depth || TRUE == isGameOver(board)) {
		struct Result val = {0, {0,0,0}};
		val.evalute = evaluteBoard(board);
		return val;
	}
	
	nextStep* moveList = 0;
	
	for (int x = 0; x < BOARD_SIZE; x++) {
    	for (int y = 0; y < BOARD_SIZE; y++) {
	        if (board[x][y] != player) continue;
	        
	        for (int i = 0; i < 8; i++) {
				const int* delta = DI[i];
				int new_x = x + delta[0];
		        int new_y = y + delta[1];
		        if (InBound(new_x, new_y) && board[new_x][new_y] == EMPTY) {
		        	nextStep* nextMove = (nextStep*)malloc(sizeof(nextStep));
		        	nextMove->x = x;
		        	nextMove->y = y;
		        	nextMove->dir = i;
		        	nextMove->next = 0;
		        	if (0 != moveList) nextMove->next = moveList;
					moveList = nextMove;
					//printf("x-%d, y-%d, d-%d\n", x, y, i);
		        }
			}
		}
    }
	
	if (player == gMeFlag) 
	{
		int best = MIN;
		struct Command command = {0, 0, 0};
	    while (moveList) {
	    	char curBoard[BOARD_SIZE][BOARD_SIZE] = {0};
			memcpy(curBoard, board, BOARD_SIZE*BOARD_SIZE);
			const int* delta = DI[moveList->dir];
			curBoard[moveList->x][moveList->y] = EMPTY;
			curBoard[moveList->x + delta[0]][moveList->y + delta[1]] = player;
			updateBoard(curBoard, moveList->x + delta[0], moveList->y + delta[1], player);
        	struct Result val = AlphaBeta(depth + 1, 3 - player, curBoard, alpha, beta);
			best  = MAX(best, val.evalute);
			alpha = MAX(alpha, best);
			command.x = moveList->x;
	        command.y = moveList->y;
	        command.option = moveList->dir;
	        
			if (alpha >= beta) {
				break;
			}
			
			nextStep* tmp = moveList;
			moveList = moveList->next;
			free(tmp);
		}
		
		struct Result rel = {best, command};
		return rel;
	}
	else
	{
		int best = MAX;
		struct Command command = {0, 0, 0};
		while (moveList) {
	    	char curBoard[BOARD_SIZE][BOARD_SIZE] = {0};
			memcpy(curBoard, board, BOARD_SIZE*BOARD_SIZE);
			const int* delta = DI[moveList->dir];
			curBoard[moveList->x][moveList->y] = EMPTY;
			curBoard[moveList->x + delta[0]][moveList->y + delta[1]] = player;
			updateBoard(curBoard, moveList->x + delta[0], moveList->y + delta[1], player);
        	struct Result val = AlphaBeta(depth + 1, 3 - player, curBoard, alpha, beta);
			best = MIN(best, val.evalute);
			beta = MIN(beta, best);
			command.x = moveList->x;
	        command.y = moveList->y;
	        command.option = moveList->dir;
			if (beta <= alpha) {
				break;
			}
			
			nextStep* tmp = moveList;
			moveList = moveList->next;
			free(tmp);
		}
		
		struct Result rel = {best, command};
		return rel;
	}
}

struct Command findValidPos(const char board[BOARD_SIZE][BOARD_SIZE], int flag) {
  struct Command command = {0, 0, 0};
  int k, x, y;
  for (k = 0; k < 8; k++) {
    const int* delta = DI[k];
    for (x = 0; x < BOARD_SIZE; x++) {
      for (y = 0; y < BOARD_SIZE; y++) {
        if (board[x][y] != flag) {
          continue;
        }
        int new_x = x + delta[1];
        int new_y = y + delta[0];
        if (InBound(new_x, new_y) && board[new_x][new_y] == EMPTY) {
          command.x = x;
          command.y = y;
          command.option = k;
          return command;
        }
      }
    }
  }
  return command;
}


/*
 * 轮到你落子。
 * 棋盘上0表示空白，1表示黑棋，2表示白旗
 * me表示你所代表的棋子(1或2) 
 * 你需要返回一个结构体Position，在x属性和y属性填上你想要落子的位置。 
 */
struct Command aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int me) {
    /*
     * TODO：在这里写下你的AI。 
     * 这里有一个示例AI，它只会寻找第一个可下的位置进行落子。 
     */
     char curBoard[BOARD_SIZE][BOARD_SIZE] = {0};
    //struct Command preferedPos = findValidPos(board, me);
    memcpy(curBoard, board, BOARD_SIZE*BOARD_SIZE);
    struct Result rel = AlphaBeta(0, me, curBoard, MIN, MAX);
	struct Command preferedPos = rel.command;
	return preferedPos;
}

/*
 * 你的代码结束 
 */
