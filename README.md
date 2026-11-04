# Scott Pilgrim: New Chaos — PS Vita prototype

An unofficial, fan-made, clean-room 2D beat-'em-up prototype for PS Vita homebrew.
This repository contains original placeholder graphics drawn in code. It does **not**
include assets, music, source code, or data from the commercial Scott Pilgrim game.

## Prototype controls

- D-pad / left stick: move
- Cross: punch
- Circle: kick
- Start: restart after a knockout / return to play
- Select: quit

The prototype currently includes a pixel-style street scene, a player, one enemy,
basic melee attacks, enemy health, player health, score, and a simple game-over state.

## Build on Windows 10/11

The recommended route is WSL2 (Ubuntu) with VitaSDK.

1. Install WSL2 and Ubuntu.
2. In Ubuntu, install prerequisites:
   ```bash
   sudo apt update
   sudo apt install -y git cmake make python3
   ```
3. Install VitaSDK (official instructions):
   ```bash
   git clone https://github.com/vitasdk/vdpm
   cd vdpm
   ./bootstrap-vitasdk.sh
   ```
4. Add the SDK to your shell environment (adjust if you installed elsewhere):
   ```bash
   echo 'export VITASDK=/usr/local/vitasdk' >> ~/.bashrc
   echo 'export PATH=$VITASDK/bin:$PATH' >> ~/.bashrc
   source ~/.bashrc
   ```
5. Install SDL2:
   ```bash
   vdpm install sdl2
   ```
6. From this project directory:
   ```bash
   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```
7. The build creates `NewChaos.vpk` in the project build directory.

## Install on a Vita

You need a Vita capable of running homebrew. Copy `NewChaos.vpk` to the Vita using
VitaShell (USB or FTP), then open the VPK in VitaShell to install it. Only install
homebrew you trust. This project does not hack or modify the Vita.

## GitHub

Create an empty GitHub repository named `scott-pilgrim-new-chaos`, then upload
these files or push this folder:

```bash
git init
git add .
git commit -m "Initial PS Vita beat-'em-up prototype"
git branch -M main
git remote add origin https://github.com/YOUR-USERNAME/scott-pilgrim-new-chaos.git
git push -u origin main
```

## Status

This is an early playable prototype, not a finished game. The current code has not
been compiled against VitaSDK in this environment. Next milestones: sprite sheets,
animation states, multiple enemy types, scrolling levels, sound effects, and save data.

## Legal note

This is an unofficial fan project and is not affiliated with or endorsed by the
rights holders. Use original art, audio, names, and code, or assets you have permission
to distribute. Do not bundle extracted commercial-game files.
