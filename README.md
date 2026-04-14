
# 🌌 Neon Lab: Cyberpunk Raycasting Engine

A high-fidelity, pseudo-3D raycasting game engine built with C++ and SFML. Step into a "Cyberpunk Neon Laboratory" featuring dynamic environments, interactive mechanics, and vibrant retro-future aesthetics.

![Cyberpunk Aesthetic](https://img.shields.io/badge/Aesthetic-Cyberpunk-blueviolet)
![Engine](https://img.shields.io/badge/Engine-Raycasting-cyan)
![Library](https://img.shields.io/badge/Library-SFML-green)

## 🚀 Features

- **Custom Raycasting Engine**: Implements classic rendering logic for a smooth, pseudo-3D experience.
- **Interactive Environments**: 
  - **Sliding Doors**: Fully interactive door system with smooth animations.
  - **Procedural Textures**: Neon-themed walls including *Blue Neon*, *Orange Industrial*, and *Emerald Glass*.
- **Combat & Particles**: 
  - Integrated firing system with point-of-impact particle bursts.
  - Dynamic enemy/target spawning system.
- **Optimized Rendering**:
  - Directional shading and distance-based "fog" effects.
  - Interactive 2D Minimap for navigation.

## 🕹️ Controls

| Key | Action |
| :--- | :--- |
| `W` | Move Forward |
| `S` | Move Backward |
| `A` | Rotate Left |
| `D` | Rotate Right |
| `E` | Interact (Open/Close Doors) |
| `Left Click` | Fire Weapon |

## 🛠️ Technical Setup

### Prerequisites
- **Compiler**: GCC/MinGW (C++17 or higher recommended).
- **Library**: [SFML 2.6+](https://www.sfml-dev.org/).
- **Font**: Requires `arial.ttf` in the project root or system path.

### Compilation Support
To compile the project, ensure SFML headers and libraries are correctly linked. Example command for MinGW:
```bash
g++ gamedev.cpp -o neon_lab -I"path/to/SFML/include" -L"path/to/SFML/lib" -lsfml-graphics -lsfml-window -lsfml-system
```

## 📜 License
This project is open-source and free to use.

---
*Created by Tanmay*
