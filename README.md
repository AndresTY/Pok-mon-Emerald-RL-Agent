# Pokémon Emerald RL Agent

<div align="center">

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![PyTorch](https://img.shields.io/badge/LibTorch-PyTorch-orange.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![CI](https://github.com/AndresTY/Pok-mon-Emerald-RL-Agent/actions/workflows/ci.yml/badge.svg)

*A deep reinforcement learning agent trained with DQN to autonomously play and complete **Pokémon Emerald***

</div>

## Overview

This project implements a **Deep Reinforcement Learning** agent using a **Deep Q-Network (DQN)** capable of autonomously playing **Pokémon Emerald**.
The agent interacts directly with the **mGBA** emulator, processes real-time frames, and takes actions based on a custom reward system designed specifically for the game.

This project was inspired by the YouTube video *"Training AI to Play Pokémon with Reinforcement Learning"* ([https://www.youtube.com/watch?v=DcYLT37ImBY](https://www.youtube.com/watch?v=DcYLT37ImBY)), but adapted for Pokémon Emerald. I used the video and its concepts as a way to practice and revisit DQN, the explore-vs-exploit strategy, and other AI fundamentals.

## Features

### Deep Learning

* **DQN** architecture with Experience Replay
* CNN optimized for GBA frame processing
* Target Network for more stable learning
* Adaptive **epsilon-greedy** exploration

### Game Integration

* Powered by the **mGBA** emulator
* Direct access to in-game RAM
* Complete event detection system for story progression
* Frame hashing to identify unique states

## Architecture

### Project Structure

```
pokemon-rl-agent/
├── src/
│   ├── main.cpp
│   ├── pokemon.cpp
│   ├── GameMemory.cpp
│   └── RewardSystem.cpp
├── include/
│   ├── pokemon.h
│   ├── GameMemory.h
│   ├── RewardSystem.h
│   └── EmeraldEvents.h
└── CMakeLists.txt
```

### Action Space

| ID | Action | Description        |
| -- | ------ | ------------------ |
| 0  | NONE   | No input           |
| 1  | A      | Confirm / interact |
| 2  | B      | Cancel / run       |
| 3  | UP     | D-Pad up           |
| 4  | DOWN   | D-Pad down         |
| 5  | LEFT   | D-Pad left         |
| 6  | RIGHT  | D-Pad right        |

## Results

I ran the project on an HP laptop with no dedicated GPU and very limited RAM, so expectations were modest due to hardware constraints.
Even so, it was able to train for several days during Christmas, and the results were surprisingly interesting.

The agent managed to follow the early storyline, assemble a full team, obtain the first badge, and even evolve the starter Pokémon (since it always fought with it and never learned to switch party members).

Although there is still significant room for improvement, the project successfully fulfilled its purpose as a way to practice and reinforce DQN concepts.

## TODO

* Improve reward and penalty design
* Fix a bug in the battle reward system
* Re-check in-game memory addresses (one or two may be incorrect, potentially causing issues)
* Consider implementing a more robust DQN variant to enhance behavior
* More unit tests

## References

* *Playing Atari with Deep Reinforcement Learning* — Mnih et al. (2013)
* *Human-level control through deep reinforcement learning* — Mnih et al. (2015)
* mGBA Emulator Project
* LibTorch (PyTorch C++ API)
* Pokémon Emerald Decompilation Project (pret)

## License

This project is distributed under the **MIT License**.
ROMs are **not** included. You must legally obtain your own copy.
