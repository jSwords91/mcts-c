#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define ROWS 6
#define COLS 7
#define SIMULATION_COUNT 10000
#define EXPLORATION_PARAM 1.414

typedef struct {
    char board[ROWS][COLS];
    char currentPlayer;
} GameState;

typedef struct Node {
    GameState state;
    struct Node* parent;
    struct Node** children;
    int childCount;
    int visitCount;
    double totalScore;
    int winCount;
    int* untriedMoves;
    int untriedMovesCount;
    int moveThatLedHere;
    char playerJustMoved;
} Node;

// ---------- Game Utilities ----------

GameState createInitialState() {
    GameState state;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            state.board[r][c] = ' ';
    state.currentPlayer = 'X';
    return state;
}

void printBoard(GameState state) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++)
            printf(" %c ", state.board[r][c]);
        printf("\n");
    }
    for (int c = 1; c <= COLS; c++) printf(" %d ", c);
    printf("\n\n");
}

bool isFull(GameState state) {
    for (int c = 0; c < COLS; c++)
        if (state.board[0][c] == ' ')
            return false;
    return true;
}

bool isWinningLine(char a, char b, char c, char d) {
    return a != ' ' && a == b && b == c && c == d;
}

char getWinner(GameState state) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) {
            if (c + 3 < COLS && isWinningLine(state.board[r][c], state.board[r][c + 1], state.board[r][c + 2], state.board[r][c + 3]))
                return state.board[r][c];
            if (r + 3 < ROWS && isWinningLine(state.board[r][c], state.board[r + 1][c], state.board[r + 2][c], state.board[r + 3][c]))
                return state.board[r][c];
            if (r + 3 < ROWS && c + 3 < COLS && isWinningLine(state.board[r][c], state.board[r + 1][c + 1], state.board[r + 2][c + 2], state.board[r + 3][c + 3]))
                return state.board[r][c];
            if (r + 3 < ROWS && c - 3 >= 0 && isWinningLine(state.board[r][c], state.board[r + 1][c - 1], state.board[r + 2][c - 2], state.board[r + 3][c - 3]))
                return state.board[r][c];
        }
    return ' ';
}

bool isGameOver(GameState state) {
    return getWinner(state) != ' ' || isFull(state);
}

int* getValidMoves(GameState state, int* moveCount) {
    int* moves = malloc(COLS * sizeof(int));
    *moveCount = 0;
    for (int c = 0; c < COLS; c++) {
        if (state.board[0][c] == ' ') {
            moves[(*moveCount)++] = c;
        }
    }
    return moves;
}

bool applyMove(GameState* state, int col) {
    for (int r = ROWS - 1; r >= 0; r--) {
        if (state->board[r][col] == ' ') {
            state->board[r][col] = state->currentPlayer;
            state->currentPlayer = (state->currentPlayer == 'X') ? 'O' : 'X';
            return true;
        }
    }
    return false;
}

// ---------- MCTS ----------

Node* createNode(GameState state, Node* parent, int moveThatLedHere) {
    Node* node = malloc(sizeof(Node));
    node->state = state;
    node->parent = parent;
    node->children = NULL;
    node->childCount = 0;
    node->visitCount = 0;
    node->totalScore = 0.0;
    node->winCount = 0;
    node->untriedMoves = getValidMoves(state, &node->untriedMovesCount);
    node->moveThatLedHere = moveThatLedHere;
    node->playerJustMoved = (state.currentPlayer == 'X') ? 'O' : 'X';
    return node;
}

Node* selectChild(Node* node) {
    Node* bestChild = NULL;
    double bestScore = -INFINITY;
    for (int i = 0; i < node->childCount; i++) {
        Node* child = node->children[i];
        double exploit = child->totalScore / (child->visitCount + 1e-6);
        double explore = EXPLORATION_PARAM * sqrt(log((double)node->visitCount + 1) / (child->visitCount + 1e-6));
        double score = exploit + explore;
        if (score > bestScore) {
            bestScore = score;
            bestChild = child;
        }
    }
    return bestChild;
}

Node* expandNode(Node* node) {
    if (node->untriedMovesCount == 0) return NULL;
    int idx = rand() % node->untriedMovesCount;
    int move = node->untriedMoves[idx];
    node->untriedMoves[idx] = node->untriedMoves[node->untriedMovesCount - 1];
    node->untriedMovesCount--;

    GameState newState = node->state;
    applyMove(&newState, move);
    Node* child = createNode(newState, node, move);
    node->children = realloc(node->children, (node->childCount + 1) * sizeof(Node*));
    node->children[node->childCount++] = child;
    return child;
}

double simulateRandomPlayout(GameState state) {
    while (!isGameOver(state)) {
        int moveCount;
        int* validMoves = getValidMoves(state, &moveCount);
        if (moveCount == 0) break;
        int move = validMoves[rand() % moveCount];
        applyMove(&state, move);
        free(validMoves);
    }
    char winner = getWinner(state);
    if (winner == 'X') return 1.0;
    if (winner == 'O') return -1.0;
    return 0.0;
}

void backpropagate(Node* node, double result) {
    while (node) {
        node->visitCount++;
        double score = (node->playerJustMoved == 'X') ? result : -result;
        node->totalScore += score;
        if ((node->playerJustMoved == 'X' && result == 1.0) ||
            (node->playerJustMoved == 'O' && result == -1.0)) {
            node->winCount++;
        }
        node = node->parent;
    }
}

void freeNode(Node* node) {
    if (!node) return;
    for (int i = 0; i < node->childCount; i++)
        freeNode(node->children[i]);
    free(node->children);
    free(node->untriedMoves);
    free(node);
}

int getBestMove(GameState state) {
    Node* root = createNode(state, NULL, -1);

    for (int i = 0; i < SIMULATION_COUNT; i++) {
        Node* node = root;
        while (node->untriedMovesCount == 0 && node->childCount > 0 && !isGameOver(node->state))
            node = selectChild(node);
        if (!isGameOver(node->state) && node->untriedMovesCount > 0)
            node = expandNode(node);
        double result = simulateRandomPlayout(node->state);
        backpropagate(node, result);
    }

    // Debug output
    printf("\n--- MCTS Debug ---\n");
    for (int i = 0; i < root->childCount; i++) {
        Node* c = root->children[i];
        double avg = c->totalScore / c->visitCount;
        double winPct = 100.0 * c->winCount / c->visitCount;
        printf("Col %d: visits = %d, avgScore = %.2f, win%% = %.1f%%\n",
            c->moveThatLedHere + 1, c->visitCount, avg, winPct);
    }

    Node* bestChild = NULL;
    double bestScore = -INFINITY;
    for (int i = 0; i < root->childCount; i++) {
        Node* c = root->children[i];
        double score = c->totalScore / c->visitCount;
        if (score > bestScore) {
            bestScore = score;
            bestChild = c;
        }
    }

    int bestMove = bestChild ? bestChild->moveThatLedHere : -1;
    if (bestMove == -1) {
        int moveCount;
        int* validMoves = getValidMoves(state, &moveCount);
        bestMove = validMoves[rand() % moveCount];
        free(validMoves);
    }

    freeNode(root);
    return bestMove;
}

// ---------- Main Game Loop ----------

void playGame() {
    GameState state = createInitialState();
    char human = 'X', ai = 'O';

    printf("You are %c\n", human);

    while (!isGameOver(state)) {
        printBoard(state);
        if (state.currentPlayer == human) {
            int move;
            do {
                printf("Enter column (1-7): ");
                scanf("%d", &move);
                move--;
            } while (move < 0 || move >= COLS || !applyMove(&state, move));
        } else {
            printf("AI is thinking...\n");
            int aiMove = getBestMove(state);
            printf("AI plays column %d\n", aiMove + 1);
            applyMove(&state, aiMove);
        }
    }

    printBoard(state);
    char winner = getWinner(state);
    if (winner == human) printf("You win!\n");
    else if (winner == ai) printf("AI wins!\n");
    else printf("Draw.\n");
}

int main() {
    srand(time(NULL));
    playGame();
    return 0;
}
