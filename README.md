Physics-Sim is an GUI based application used to simulate, visualize and compare environments using different types of numerical integrations like Euler, Velet and RK4.

Features:
* Numerical intergrations: Semi-implicit Euler, Verlet and Rk4.
* Standard Environments: Free Fall and Circular Orbit.
* Simulate and compare any delta time.
* Simulate in realtime.
* Simulate the entire simulation in to an recording.
* View the recording frame by frame using the Player.
* Compare multiple selected recordings side by side.
* Export selected recordings into an Excel file.
* Automatically calculates delta position and total energy between simulated body and calculated kinematic body. (Only if you use one of the environment presets)

External Libraries:
SFML https://github.com/SFML/SFML
ImGui https://github.com/ocornut/imgui 
imgui-sfml https://github.com/SFML/imgui-sfml
glad https://github.com/Dav1dde/glad
glm https://github.com/g-truc/glm

Build Guide for windows:

1. Download Visual Studio: https://visualstudio.microsoft.com/
When setting up Visual Studio you need to select "Desktop development with C++".

3. Download the projects source code and extract zip: https://github.com/Larjak-Dev/physics-sim/archive/refs/heads/master.zip
   
4. Open the source code folder in Visual Studio and click Build.

Build:
Project is built using CMake and the correct libraries are automatically fetched, built and statically linked when running Cmake. The reason it's statically linked is because i had issues with SFML linking with incompatible shared libraries when building in an MinGW environment. Build instructions for console cmake environment:
1. git clone https://github.com/Larjak-Dev/physics-sim.git
2. cd physics-sim
3. 

