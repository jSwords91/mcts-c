#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

// ----- CONFIGURATION -----
#define ROWS 6
#define COLS 7
#define SIMULATION_COUNT 10000      // Number of rollouts per AI move
#define EXPLORATION_PARAM 1.414     // sqrt(2) — balances exploration/exploitation
#define EPSILON 1e-6                // Small constant to avoid division by zero

// ----- GAME STATE STRUCT -----
typedef struct {
    char board[ROWS][COLS];
    char currentPlayer;             // 'X' or 'O'
} GameState;

// ----- MCTS NODE STRUCT -----
typedef struct Node {
    GameState state;                // The board configuration at this node
    struct Node* parent;            // Parent in the tree
    struct Node** children;         // Dynamically allocated child array
    int childCount;
    int visitCount;
    double totalScore;              // Cumulative value from rollouts
    int winCount;                   // Convenience metric for debugging
    int* untriedMoves;              // Legal moves yet to be expanded
    int untriedMovesCount;
    int moveThatLedHere;            // Column index (0–6) that produced this state
    char playerJustMoved;           // Who played that move
} Node;


// ==========================================================
//                    GAME LOGIC HELPERS
// ==========================================================

GameState createInitialState() {
    GameState state;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            state.board[r][c] = ' ';
    state.currentPlayer = 'X';
    return state;
}

void printBoard(GameState state) {
    printf("\n   ");
    for (int c = 0; c < COLS; c++)
        printf(" %d  ", c + 1);
    printf("\n");

    for (int r = 0; r < ROWS; r++) {
        printf("   ");
        for (int c = 0; c < COLS; c++) {
            char symbol = state.board[r][c];
            if (symbol == 'X') printf("| \033[1;31mX\033[0m ");
            else if (symbol == 'O') printf("| \033[1;34mO\033[0m ");
            else printf("|   ");
        }
        printf("|\n");
    }
    printf("   ");
    for (int c = 0; c < COLS; c++) printf("----");
    printf("-\n\n");
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
            if (c + 3 < COLS && isWinningLine(state.board[r][c], state.board[r][c + 1],
                                              state.board[r][c + 2], state.board[r][c + 3]))
                return state.board[r][c];
            if (r + 3 < ROWS && isWinningLine(state.board[r][c], state.board[r + 1][c],
                                              state.board[r + 2][c], state.board[r + 3][c]))
                return state.board[r][c];
            if (r + 3 < ROWS && c + 3 < COLS && isWinningLine(state.board[r][c], state.board[r + 1][c + 1],
                                                              state.board[r + 2][c + 2], state.board[r + 3][c + 3]))
                return state.board[r][c];
            if (r + 3 < ROWS && c - 3 >= 0 && isWinningLine(state.board[r][c], state.board[r + 1][c - 1],
                                                            state.board[r + 2][c - 2], state.board[r + 3][c - 3]))
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
    for (int c = 0; c < COLS; c++)
        if (state.board[0][c] == ' ')
            moves[(*moveCount)++] = c;
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


// ==========================================================
//                 MCTS CORE FUNCTIONS
// ==========================================================

// ---------- NODE CREATION ----------
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


// ---------- SELECTION ----------
// Traverses the tree until we reach a node with untried moves or a terminal state.
// Uses the UCB1 formula:  Q_i + c * sqrt( ln(N) / n_i )
Node* selectChild(Node* node) {
    Node* bestChild = NULL;
    double bestScore = -INFINITY;
    const double lnN = log((double)node->visitCount + 1.0);  // total visits of parent

    for (int i = 0; i < node->childCount; i++) {
        Node* child = node->children[i];

        // If a child is unvisited, pick it immediately (encourages exploration)
        if (child->visitCount == 0) {
            return child;
        }

        // Exploitation term: mean score from rollouts
        const double exploit = child->totalScore / (double)child->visitCount;
        // Exploration term: encourages visiting less explored nodes
        const double explore = EXPLORATION_PARAM * sqrt(lnN / (double)child->visitCount);

        const double score = exploit + explore;

        if (score > bestScore) {
            bestScore = score;
            bestChild = child;
        }
    }
    return bestChild;
}


// ---------- EXPANSION ----------
// Pick one untried move from the node, apply it, and create a child node.
// This adds one new node to the tree.
Node* expandNode(Node* node) {
    if (node->untriedMovesCount == 0) return NULL;

    // Randomly choose an untried move
    int idx = rand() % node->untriedMovesCount;
    int move = node->untriedMoves[idx];

    // Remove chosen move from list
    node->untriedMoves[idx] = node->untriedMoves[node->untriedMovesCount - 1];
    node->untriedMovesCount--;

    // Apply move to create a new game state
    GameState newState = node->state;
    applyMove(&newState, move);

    // Create a new node and link it as a child
    Node* child = createNode(newState, node, move);
    node->children = realloc(node->children, (node->childCount + 1) * sizeof(Node*));
    node->children[node->childCount++] = child;
    return child;
}


// ---------- SIMULATION ----------
// From the newly expanded state, play random moves until the game ends.
// Return +1 if X wins, -1 if O wins, or 0 for draw.
// (This is the Monte Carlo part.)
double simulateRandomPlayout(GameState state) {
    while (!isGameOver(state)) {
        int moveCount = 0;
        int* validMoves = getValidMoves(state, &moveCount);
        if (moveCount == 0) {
            free(validMoves);
            break;
        }
        int move = validMoves[rand() % moveCount];
        free(validMoves);
        applyMove(&state, move);
    }

    char winner = getWinner(state);
    if (winner == 'X') return 1.0;
    if (winner == 'O') return -1.0;
    return 0.0;
}


// ---------- BACKPROPAGATION ----------
// Propagate the simulation result up the tree.
// Each ancestor updates its statistics: visit count and cumulative score.
// The sign of the result is flipped at each level depending on who just moved.
void backpropagate(Node* node, double result) {
    while (node) {
        node->visitCount++;
        // Flip perspective: a win for X is a loss for O and vice versa.
        double score = (node->playerJustMoved == 'X') ? result : -result;
        node->totalScore += score;

        if ((node->playerJustMoved == 'X' && result == 1.0) ||
            (node->playerJustMoved == 'O' && result == -1.0))
            node->winCount++;

        node = node->parent;
    }
}


// ---------- MEMORY CLEANUP ----------
void freeNode(Node* node) {
    if (!node) return;
    for (int i = 0; i < node->childCount; i++)
        freeNode(node->children[i]);
    free(node->children);
    free(node->untriedMoves);
    free(node);
}


// ---------- MAIN MCTS LOOP ----------
// One full MCTS search cycle: run many (selection → expansion → simulation → backpropagation)
// iterations and return the move with the highest visit count.
int getBestMove(GameState state) {
    Node* root = createNode(state, NULL, -1);

    for (int i = 0; i < SIMULATION_COUNT; i++) {
        Node* node = root;

        // (1) SELECTION: descend the tree using UCB until a leaf node
        while (node->untriedMovesCount == 0 &&
               node->childCount > 0 &&
               !isGameOver(node->state))
            node = selectChild(node);

        // (2) EXPANSION: if not terminal, expand one new child
        if (!isGameOver(node->state) && node->untriedMovesCount > 0)
            node = expandNode(node);

        // (3) SIMULATION: play out to the end randomly
        const double result = simulateRandomPlayout(node->state);

        // (4) BACKPROPAGATION: update stats up the tree
        backpropagate(node, result);
    }

    // Debug output to see what MCTS learned
    printf("\n--- MCTS Debug ---\n");
    for (int i = 0; i < root->childCount; i++) {
        Node* c = root->children[i];
        int v = c->visitCount;
        double avg = v ? (c->totalScore / v) : 0.0;
        double winPct = v ? (100.0 * c->winCount / v) : 0.0;
        printf("Col %d: visits=%d  avgScore=%.3f  win%%=%.1f%%\n",
               c->moveThatLedHere + 1, v, avg, winPct);
    }

    // Final decision: choose the move with the most visits (robust criterion)
    Node* bestChild = NULL;
    int bestVisits = -1;
    for (int i = 0; i < root->childCount; i++) {
        Node* c = root->children[i];
        if (c->visitCount > bestVisits) {
            bestVisits = c->visitCount;
            bestChild = c;
        }
    }

    int bestMove = bestChild ? bestChild->moveThatLedHere : -1;
    if (bestMove == -1) {
        // Fallback: random valid move (shouldn’t happen often)
        int moveCount = 0;
        int* validMoves = getValidMoves(state, &moveCount);
        if (moveCount > 0)
            bestMove = validMoves[rand() % moveCount];
        free(validMoves);
    }

    freeNode(root);
    return bestMove;
}


// ==========================================================
//                      MAIN GAME LOOP
// ==========================================================

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
                if (scanf("%d", &move) != 1) exit(0);
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


// ==========================================================
//                           MAIN
// ==========================================================
int main() {
    srand((unsigned int)time(NULL));
    playGame();
    return 0;
}
