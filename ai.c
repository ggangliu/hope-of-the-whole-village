#include "ai.h"

// bool
typedef int BOOL;
#define TRUE 1
#define FALSE 0

/*
 * YOUR CODE BEGIN
 * 你的代码开始
 */
 
/* 
 * You can define your own struct and variable here
 * 你可以在这里定义你自己的结构体和变量
 */
extern const int DIR[8][2]; 
typedef struct SmoveNode {
	struct SmoveNode* preBrother;
	struct SmoveNode* posBrother;
	struct SmoveNode* children;
	int x, y, dir;
	int player;
	int curScore, bestScore;
} moveNode;
	
moveNode* rootMoveTree = 0;

typedef struct SnextStep{
	int x;
	int y;
	int dir;
	struct SnextStep* next;
} nextStep;

struct Result {
	int evalute;
	struct Command command;
	moveNode* childrenTree;
} Result;

int gMeFlag = 0;
const int MAX =  1000;
const int MIN = -1000;
const int LEAF_NODE = 3;
unsigned int maxMoveNode = 9830400;
unsigned int countMoveNode = 0;
#define NULL 0
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

BOOL inBound(int x, int y) {
  return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

void releaseMoveTreeBack(moveNode* mTree){
	moveNode* iter = mTree;
	while (iter) {
		//可以在之前的基础上，根据对方的走位进行更深度的递归，当前先释放
		moveNode* tmp = iter;
		if (tmp->children) releaseMoveTreeBack(tmp->children);
		iter = iter->posBrother; 
		free(tmp);
		countMoveNode--;
	}
}

void releaseMoveTreeForward(moveNode* mTree){
	moveNode* iter = mTree;
	while (iter) {
		//可以在之前的基础上，根据对方的走位进行更深度的递归，当前先释放
		moveNode* tmp = iter;
		if (tmp->children) releaseMoveTreeForward(tmp->children);
		iter = iter->preBrother; 
		free(tmp);
		countMoveNode--;
	}
}
	
void updateBoard(char board[BOARD_SIZE][BOARD_SIZE], int nextx, int nexty, int meFlag)
{
	int otherFlag = 3 - meFlag;
    // Yak
	int intervention_dir[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
  	for (int i = 0; i < 4; i++) {
	    int x1 = nextx + intervention_dir[i][0];
	    int y1 = nexty + intervention_dir[i][1];
	    int x2 = nextx - intervention_dir[i][0];
	    int y2 = nexty - intervention_dir[i][1];
	    if (inBound(x1, y1) && inBound(x2, y2) && board[x1][y1] == otherFlag && board[x2][y2] == otherFlag) {
	    	board[x1][y1] = meFlag;
	    	board[x2][y2] = meFlag;
	    }
    }
    
    // Mak
    for (int i = 0; i < 8; i++)
    {
		if (inBoard(nextx + 2 * DIR[i][0], nexty + 2 * DIR[i][1]) &&
			board[nextx + DIR[i][0]][nexty + DIR[i][1]] == otherFlag && board[nextx + 2 * DIR[i][0]][nexty + 2 * DIR[i][1]] == meFlag)
		{
			board[nextx + DIR[i][0]][nexty + DIR[i][1]] = meFlag;
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
		struct Result val = {0, {0,0,0}, 0};
NoEnoughMemory:
		val.evalute = evaluteBoard(board);
		return val;
	}
	
	moveNode* moveTree = NULL;
	
	for (int x = 0; x < BOARD_SIZE; x++) {
    	for (int y = 0; y < BOARD_SIZE; y++) {
	        if (board[x][y] != player) continue;
	        
	        for (int i = 0; i < 8; i++) {
				const int* delta = DIR[i];
				int new_x = x + delta[0];
		        int new_y = y + delta[1];
		        if (inBound(new_x, new_y) && board[new_x][new_y] == EMPTY) {
		        	if (countMoveNode >= maxMoveNode) goto Moving;
		        	moveNode* nextMove = (moveNode*)malloc(sizeof(moveNode));
		        	if (NULL == nextMove) goto Moving; //Have not enough memory.
		        	countMoveNode++;
		        	nextMove->preBrother = NULL;
		        	nextMove->posBrother = NULL;
		        	nextMove->children   = NULL;
		        	nextMove->x = x;
		        	nextMove->y = y;
		        	nextMove->dir = i;
		        	nextMove->player = player;
		        	nextMove->curScore = nextMove->bestScore = 0;
		        	
		        	if (NULL != moveTree) {
		        		nextMove->posBrother = moveTree;
		        		moveTree->preBrother = nextMove;
					}
					moveTree = nextMove;
					//printf("x-%d, y-%d, d-%d\n", x, y, i);
		        }
			}
		}
    }
	
Moving:
	if (NULL == moveTree) {
		printf("shit!\n");
		goto NoEnoughMemory;
	}
	
	if (player == gMeFlag) 
	{
		int best = MIN;
		struct Command command = {0, 0, 0};
		moveNode* bestMove = NULL;
		moveNode* iter = moveTree;
	    while (iter) {
	    	char curBoard[BOARD_SIZE][BOARD_SIZE] = {0};
	    	memset(curBoard, 0, sizeof(curBoard));
			memcpy(curBoard, board, sizeof(curBoard));
			const int* delta = DIR[iter->dir];
			curBoard[iter->x][iter->y] = EMPTY;
			curBoard[iter->x + delta[0]][iter->y + delta[1]] = player;
			updateBoard(curBoard, iter->x + delta[0], iter->y + delta[1], iter->player);
			iter->curScore = evaluteBoard(curBoard);
			struct Result val = {0, {0,0,0}, 0};
        	val = AlphaBeta(depth + 1, 3 - player, curBoard, alpha, beta);
        	iter->bestScore = val.evalute;
        	iter->children  = val.childrenTree;
        	if (iter->bestScore > best) { //等于的场景没考虑 
        		command.x = iter->x;
	        	command.y = iter->y;
	        	command.option = iter->dir;
	        	bestMove = iter;
			}
			best  = MAX(best, iter->bestScore);
			alpha = MAX(alpha, best);
	        
			if (alpha >= beta) {
				break;
			}
			
			iter = iter->posBrother;
		}
		
		struct Result rel = {best, command, bestMove};
		if (bestMove) {
			releaseMoveTreeForward(bestMove->preBrother);
			bestMove->preBrother = NULL;
		    if (iter) {
		    	releaseMoveTreeBack(bestMove->posBrother);
				bestMove->posBrother = NULL;	
			}
		} 
		
		return rel;
	}
	else
	{
		int best = MAX;
		struct Command command = {0, 0, 0};
		moveNode* bestMove = NULL;
		moveNode* iter = moveTree;
	    while (iter) {
	    	char curBoard[BOARD_SIZE][BOARD_SIZE] = {0};
	    	memset(curBoard, 0, sizeof(curBoard));
			memcpy(curBoard, board, sizeof(curBoard));
			const int* delta = DIR[iter->dir];
			curBoard[iter->x][iter->y] = EMPTY;
			curBoard[iter->x + delta[0]][iter->y + delta[1]] = player;
			updateBoard(curBoard, iter->x + delta[0], iter->y + delta[1], iter->player);
			iter->curScore = evaluteBoard(curBoard);
			struct Result val = {0, {0,0,0}, 0};
        	val = AlphaBeta(depth + 1, 3 - player, curBoard, alpha, beta);
        	iter->bestScore = val.evalute;
        	iter->children  = val.childrenTree;
        	if (iter->bestScore < best) {
        		command.x = iter->x;
	        	command.y = iter->y;
	        	command.option = iter->dir;
	        	bestMove = iter;
			}
			best = MIN(best, iter->bestScore);
			beta = MIN(beta, best);
			
			if (beta <= alpha) {
				break;
			}
			
			iter = iter->posBrother;
		}
		
		struct Result rel = {best, command, bestMove};
		if (bestMove) {
			releaseMoveTreeForward(bestMove->preBrother);
			bestMove->preBrother = NULL;
		    if (iter) {
		    	releaseMoveTreeBack(bestMove->posBrother);
				bestMove->posBrother = NULL;	
			}
		} 
		
		return rel;
	}
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
    moveNode* ValidMoveTree = NULL;
    //struct Command preferedPos = findValidPos(board, me);
    memcpy(curBoard, board, BOARD_SIZE*BOARD_SIZE);
    struct Result rel = AlphaBeta(0, me, curBoard, MIN, MAX);
    
	struct Command preferedPos = rel.command;
    rootMoveTree = rel.childrenTree;
    int bestScore = rel.evalute;
	int curScore  = rel.childrenTree->curScore;
    
    moveNode* iter = rootMoveTree->posBrother;  /*寻找多个全局同优走法中的当前最优走法*/
    BOOL hasMutipSolu = FALSE;
	while (iter) {
		if (iter->bestScore > bestScore || (iter->bestScore == bestScore && iter->curScore > curScore)) {
			bestScore = iter->bestScore;
			curScore = iter->curScore;
			preferedPos.x = iter->x;
			preferedPos.y = iter->y;
			preferedPos.option = iter->dir;
			ValidMoveTree = iter;
		}
		else if (iter->bestScore == bestScore && iter->curScore == curScore) {
			hasMutipSolu = TRUE;
		}
		else if (iter->bestScore < bestScore || (iter->bestScore == bestScore && iter->curScore < curScore)) {
			moveNode* tmp = iter;
			iter->preBrother->posBrother = iter->posBrother;
			if (iter->posBrother) {
				iter->posBrother->preBrother = iter->preBrother;
			}
			
			iter = iter->posBrother; 
			tmp->preBrother = NULL;
			tmp->posBrother = NULL;
			releaseMoveTreeBack(tmp);
			continue;
		}
		
		iter = iter->posBrother; 
	}
	
	//如果多个全局最优均一样，那么选择向中心聚拢
	if (hasMutipSolu && rootMoveTree->posBrother) {
		struct Command needMove = {0,0,0};
		moveNode* iter = rootMoveTree;
		int distance = 0;
		while (iter) {
			int temp = sqrt(pow(iter->x - 6, 2) + pow(iter->y - 6, 2));
			if (temp > distance) {
				distance = temp;
				needMove.x = iter->x;
				needMove.y = iter->y; 
			} 
			iter = iter->posBrother;
		}
		
		int dir = 0;
		int dir_x = (6 - needMove.x == 0) ? 0 : (6 - needMove.x) / abs(6 - needMove.x); 
		int dir_y = (6 - needMove.y == 0) ? 0 : (6 - needMove.y) / abs(6 - needMove.y);

		for (int d = 0; d < 8; d++) 
		{
			if (DIR[d][0] == dir_x && DIR[d][1] == dir_y) dir = d;
		}
		
		iter = rootMoveTree;
		while (iter) {
			if (iter->x == needMove.x && iter->y == needMove.y && iter->dir == dir) break;
			iter = iter->posBrother;
		}
		
		if (iter) {
			preferedPos.x = needMove.x;
			preferedPos.y = needMove.y;
			preferedPos.option = dir;
		}
	} 
	
	releaseMoveTreeBack(rootMoveTree);
	
	return preferedPos;
}

/*
 * 你的代码结束 
 */
