# Monte Carlo Tree Search (MCTS) Connect-4 — in Pure C

This project implements a **Connect-4 AI using Monte Carlo Tree Search (MCTS)** written entirely in C.  
It’s designed as a **pedagogical reference** — clear, self-contained, and heavily commented.  
The code demonstrates every phase of the MCTS algorithm in a working game loop you can play from the terminal.

---


## Overview

Monte Carlo Tree Search (MCTS) is one of the most powerful algorithms for **decision-making under uncertainty** — used in systems like **AlphaGo**, **AlphaZero**, and modern **game-playing AIs**.

At its core, MCTS is about **sampling the future**.  
Instead of exhaustively enumerating all possible moves like minimax, it **simulates random games** (rollouts) from the current position and uses statistics to guide exploration toward better moves.

---

## How MCTS Works

Each iteration of MCTS consists of **four stages**:

| Stage | Name | Description | Purpose |
|--------|------|-------------|----------|
| 1 | **Selection** | Traverse the tree using a formula (UCB1) to choose the most promising child at each step | Balance exploration vs. exploitation |
| 2 | **Expansion** | Add one new child node for an untried move | Grow the search tree |
| 3 | **Simulation** | Play random moves from that state until the game ends | Estimate the value of the state |
| 4 | **Backpropagation** | Propagate the simulation result back up the tree | Update node statistics |

### Step 1 — Selection

Start from the root node (the current board).  
Keep selecting child nodes according to the **UCB1 formula**:

\[
UCB1 = \frac{Q_i}{n_i} + c \sqrt{\frac{\ln N}{n_i}}
\]

Where:
- \( Q_i \) = total reward from simulations for child *i*
- \( n_i \) = number of times child *i* has been visited
- \( N \) = number of times the parent has been visited
- \( c \) = exploration constant (≈ √2 is common)

Intuition:
- The first term (`Q_i / n_i`) rewards moves that have *performed well so far*  
- The second term (`c * sqrt(ln N / n_i)`) rewards *less explored* moves  
- The balance between them lets MCTS explore intelligently

### Step 2 — Expansion

When you reach a node that:
- still has untried moves, and  
- is not a terminal (win/draw) state  

… you add a new child corresponding to one of those untried moves.  
This expands the tree by one node.

### Step 3 — Simulation (Rollout)

From that newly expanded node, you **simulate random moves** until the game ends.  
The result is +1 if X wins, −1 if O wins, or 0 for a draw.

This random playout gives a *rough statistical estimate* of how promising the position is.

### Step 4 — Backpropagation

You take that result and **propagate it upward** through all ancestor nodes.  
Each node updates its:
- `visitCount` (number of times visited)
- `totalScore` (cumulative simulation reward)

Importantly, because the players alternate turns, the sign of the result flips at each level:
- A win for X is a loss for O, and vice versa.

---

## Why It Works

MCTS gradually builds an asymmetric search tree that focuses computational effort where it matters most:
- **Good moves get explored more**
- **Bad moves get ignored over time**
- Yet, the **exploration term ensures no move is completely ignored**

This makes MCTS robust and general — it doesn’t need a perfect evaluation function or exhaustive search.

---

## Features

- Complete **Connect-4** implementation (6×7 board)
- Full **MCTS (UCT)** search loop:
  - UCB1 selection  
  - Node expansion  
  - Random rollouts  
  - Backpropagation of results
- **Terminal interface** to play against the AI
- **Colorised board rendering** for clarity
- Debug output summarising visit counts and win rates

---

## Running

### 1. Compile

Use any standard C compiler:

```bash
gcc c4.c -o c4 -lm
```

### 2. Run

```bash
./c4
```

## Note on Exploration

Exploration Constant:

```EXPLORATION_PARAM = 1.414```, (√2) is a good default.

Larger → more exploration
Smaller → greedier search