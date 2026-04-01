# PhysicsSim Architecture

This document provides a high-level overview of the PhysicsSim architecture, module interactions, threading model, and core design patterns. It is intended to help developers navigate the codebase and understand the flow of data.

## 1. System Overview

PhysicsSim is a GUI-based application that simulates, visualizes, and compares physical environments using numerical integrations. The system is divided into four primary layers: Application, Core Data, Physics Engine, and Graphics.

```mermaid
graph TD
    subgraph Application Layer (src/app)
        App[PhysicApp] --> Context[AppContext]
        App --> Slides[Slides: Editor, Simulator, Player]
        Slides --> UI[ImGui Widgets]
    end

    subgraph Data Layer (src/core)
        Context --> Univ[Universe]
        Univ --> Env[Environment]
        Univ --> Config[PhysicConfig]
    end

    subgraph Graphics Layer (src/graphics)
        Render[Renderer] -.-> |Reads State| Env
        Render --> Transform[Transform2D]
        Render --> Glad[OpenGL / GladWrap]
    end

    subgraph Physics Layer (src/physics)
        Sim[Simulator] --> Func[PhysicFunctions]
        Sim -.-> |Updates State| Env
        Sim --> Kinematics[Kinematics]
    end

    UI --> |Modifies| Univ
    Slides --> |Controls| Sim
    Slides --> |Directs| Render
```

## 2. Module Responsibilities

### Core Layer (`src/core`)
*   **Universe & Environment:** The central data models. `Environment` holds the state of all bodies (position, velocity, mass) at a given point in time. `Universe` contains the `Environment` and the `PhysicConfig` (settings for numerical integration, delta time, etc.), acting as the Single Source of Truth.
*   **Units:** Provides constants (e.g., $G$) and utilities for unit conversions.

### Physics Layer (`src/physics`)
*   **Simulator:** Manages the threading logic for the simulation. It runs the physics updates in background threads (`std::thread`) to ensure the main UI thread remains responsive (targeting 60 FPS). It handles live simulations and pre-calculating recordings for the Player.
*   **PhysicFunctions:** Implements numerical integration algorithms (Semi-implicit Euler, Verlet, RK4) using a functional approach (`std::function`). This allows the simulation to hot-swap integration methods based on the `PhysicConfig`.
*   **Kinematics:** Provides analytical solutions for standard environments (e.g., Circular Orbits, Free Fall) to compare the accuracy of the numerical integrations against theoretically perfect paths.

### Graphics Layer (`src/graphics`)
*   **Renderer:** Handles all rendering logic using OpenGL. It is completely decoupled from the physics logic and only reads the current `Environment` to draw bodies, grids, and skyboxes.
*   **Transform2D & Camera:** Manages coordinate space transformations. It converts world-space coordinates (e.g., meters or Astronomical Units) into screen-space pixels and handles camera projection (Orthographic or Perspective).
*   **GladWrap:** A lightweight C++ wrapper around OpenGL constructs (FrameBuffers, Shaders, Textures) to enforce RAII and simplify resource management.

### Application Layer (`src/app`)
*   **PhysicApp & Slides:** The main application lifecycle. The UI is built around a "Slide" concept (like a state machine), where the user transitions between the `Editor` (setting up the universe), the `Simulator` (running and analyzing), and the `Player` (viewing recorded data).
*   **AppContext:** A shared context struct passed to slides and renderers, containing pointers to the global `Universe`, the `Simulator`, and UI resources.
*   **Widgets:** Modular ImGui components (e.g., `AnalyzeWidget`, `SceneWidget`, `UniverseWidget`) that provide the user interface for interacting with the `Universe`.

## 3. Threading Model and Data Flow

PhysicsSim uses a Producer-Consumer pattern to separate the heavy computational workload from the rendering loop:

1.  **UI Thread (Main):** Handles SFML event polling, ImGui rendering, and OpenGL draw calls via the `Renderer`.
2.  **Configuration:** The user modifies the `Universe` configuration via the UI.
3.  **Worker Thread (Simulation):** The `Simulator` spawns a background thread.
4.  **Integration:** The worker thread copies the current `Environment`, applies the selected `PhysicFunctions` over the configured `delta_time`, and computes the new state.
5.  **Synchronization:** The worker thread safely updates the `Universe`'s `Environment` pointer using atomic operations or mutex locks (e.g., `setEnvironment_safe`).
6.  **Rendering:** On the next frame, the `Renderer` (Main Thread) fetches the latest `Environment` and draws it.

## 4. Key Design Patterns

*   **State Machine (Slides):** The application flow is cleanly divided into specific modes (`Editor`, `Simulator`, `Player`), each responsible for its own UI and interactions with the `Simulator`.
*   **Functional Physics:** Integrators (Euler, Verlet, RK4) and Force calculations are abstracted into `std::function` objects. This makes the physics engine highly extensible; adding a new integration method requires no changes to the `Simulator` threading logic.
*   **Data Decoupling:** The `Renderer` has no knowledge of how physics is calculated, and the `Simulator` has no knowledge of how things are drawn. They only communicate through the shared `Environment` data structure.
*   **RAII Resource Management:** OpenGL handles and SFML resources are wrapped in classes to ensure proper cleanup upon destruction.
