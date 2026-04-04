Physics-Sim is an GUI based application used to simulate, visualize and compare environments using different types of numerical integrations like Euler, Velet and RK4.

Features:
* Numerical intergrations: Semi-implicit Euler, Verlet and Rk4.
* Preset Environments: Free Fall and Circular Orbit.
* Simulate and compare any delta time.
* Simulate in realtime.
* Simulate the entire simulation in to an recording.
* View the recording frame by frame using the Playback.
* Compare multiple selected recordings side by side.
* Export selected recordings into an Excel file.
* Automatically calculates delta position and total energy between simulated body and calculated kinematic body. (Only works if you use one of the kinematic supported environment presets: Free fall or Circular orbit)
* Multi threaded (Seperated window and simulating thread)
* Automatically import, cache and simulate the solar system from NASA JPL Horizons API.

<img width="1261" height="1390" alt="screenshot-2026-04-04_02-42-46" src="https://github.com/user-attachments/assets/1993f8dc-6e91-4eff-bbe4-ae10657f30a1" />

<img width="1261" height="1390" alt="screenshot-2026-04-04_02-43-25" src="https://github.com/user-attachments/assets/e06c1cf2-0d91-4a2f-843c-2c2e1d779bb0" />


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

API:
* This project uses the Horizons on-line solar system data service, provided by the Solar System Dynamics Group of the Jet Propulsion Laboratory. Planetary data is cached to save on api calls! Thank you NASA! ![NASA](https://img.shields.io/badge/Data%20Source-NASA%20JPL-blue)

Project Details:
* Programmed in C++20/23
* Pretty much no AI written code!
* Most code in this project was written in one week. (No joke)
* But i had previously already written and planned out an scrapped app similar to this one. 


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
