# Monte Carlo Tree Search (MCTS) Connect-4 — in Pure C

This project implements a **Connect-4 AI using Monte Carlo Tree Search (MCTS)** written entirely in C.  
It’s designed as a **pedagogical reference** — clear, self-contained, and heavily commented.  
The code demonstrates every phase of the MCTS algorithm in a working game loop you can play from the terminal.

---

## Overview

Monte Carlo Tree Search (MCTS) is a general decision-making algorithm for games and planning problems.  
It searches by repeatedly **simulating random games**, using those results to bias future exploration toward better moves.

The algorithm cycles through four key steps:

| Step | Name | Description | Key Idea |
|------|------|--------------|-----------|
| 1 | **Selection** | Descend the tree using the **UCB1** formula | Balance exploration vs. exploitation |
| 2 | **Expansion** | Add a new child node for one untried move | Grow the tree gradually |
| 3 | **Simulation** | Play random moves until the game ends | Estimate long-term reward |
| 4 | **Backpropagation** | Update statistics on the path back to the root | Share value upward |

Over many iterations, the tree concentrates rollouts on promising moves.  
The best move is usually the one visited most often at the root.

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

## 🧠 UCB Formula

During selection, the algorithm scores each child `i` as:

\[
\text{UCB1}_i = \frac{Q_i}{n_i} + c \sqrt{\frac{\ln N}{n_i}}
\]

Where:
- \( Q_i \): total accumulated reward of child  
- \( n_i \): visits to child  
- \( N \): total visits of parent  
- \( c \): exploration constant (≈ √2 works well)

This balances:
- **Exploration** (visit nodes with few samples)  
- **Exploitation** (visit nodes with high average reward)

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