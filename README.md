# Monte Carlo Tree Search in C

A pure C implementation of Monte Carlo Tree Search over the game **"connect-4"**.

This will run a game of connect-4 (6x7 board) in your command line. The AI is ~unbeatable, it runs 10,000 simulations by default at each turn.

During AI "thinking" time, you'll see some stats such as the number of visits for a column option, the average score, and the win % from the subsequent rollouts.

MCTS agent with:
- UCT (Upper Confidence Bound for Trees)
- Random rollouts for simulation

## **Compilation**

Compile with ```gcc -o c4 c4.c```

Then run with ```/.c4```

## Monte Carlo Tree Search

Monte Carlo Tree Search consists of four steps:

- **Selection**: Traverse the tree from the root using UCT to select the most promising node.

- **Expansion**: If the node has untried moves, create a new child by applying one.

- **Simulation**: Play a random game from this new state until it ends.

- **Backpropagation**: Propagate the result of the simulation back up the tree.

The best move is chosen based on average reward from simulations.

In more detail:

Selection:
    - Uses UCB1 (exploitation + exploration) to traverse from root to a leaf node.

Expansion:

    - Expands one of the untried valid moves for a node.

Simulation (Rollout):

    - Runs a random playout until terminal state (X, O, or draw).

Backpropagation:

    - Rewards are propagated back up the tree, with reward +1 for win, -1 for loss.

Best Move Selection:

    - Chooses the move with the highest average value at root after SIMULATION_COUNT rollouts.



