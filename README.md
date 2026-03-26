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
* Multi threaded

External Libraries:
* SFML https://github.com/SFML/SFML is under zlib License
* ImGui https://github.com/ocornut/imgui is under MIT license
* imgui-sfml https://github.com/SFML/imgui-sfml is under MIT license
* glad https://github.com/Dav1dde/glad is under is under MIT license
* glm https://github.com/g-truc/glm is under MIT license
* OpenXLSX https://github.com/troldal/OpenXLSX  is under BSD 3-Clause "New" or "Revised" License
* Par (Shapes) https://github.com/prideout/par/tree/master  is under MIT License
* IconsFontAwesome https://github.com/juliettef/IconFontCppHeaders is under zlib License
* KHR https://github.com/KhronosGroup/EGL-Registry is under MIT License

Project Details:
* Programmed in C++20 / 23 (As much as i can)
* Pretty much no AI written code!
* Most code in this project was written in one week. (No joke)
* But i had previously already written and planned out an scrapped app similar to this. 


Build Guide for windows:

1. Download Visual Studio: https://visualstudio.microsoft.com/
When setting up Visual Studio you need to select "Desktop development with C++".
2. Download Git: https://git-scm.com/install/windows
3. Download the projects source code and extract zip: https://github.com/Larjak-Dev/physics-sim/archive/refs/heads/master.zip
4. Open the source code folder in Visual Studio and click Build.

Build for c++ cmake console environment:
1. git clone https://github.com/Larjak-Dev/physics-sim.git
2. cd physics-sim
3. cmake ..
4. mkdir build
5. cd build
6. cmake --build .

Project is built using CMake and the correct libraries are automatically fetched, built and statically linked when running Cmake. The reason it's statically linked is because i had issues with SFML linking with incompatible shared libraries when building in an MinGW environment. 

