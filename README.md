# GroupIt Auto-Solver

The bot uses **ADB (Android Debug Bridge)** for device control and simple algorithms to analyze the board, detect sets, and execute moves in real-time.

## Prerequisites

To run this solver, you need:

1.  **ADB**
2.  **ImageMagick:**


## Installation & Build

1.  **Clone the repository**
    ```bash
    git clone https://github.com/nikostsio/GroupIt.git
    cd GroupIt
    ```

2.  **Build using CMake**
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

## Usage

1.  Open the **GroupIt** app on your phone and start a game.
2.  Run the bot:
    ```bash
    cd ..
    ./build/Debug/GroupItSolver.exe
    ```