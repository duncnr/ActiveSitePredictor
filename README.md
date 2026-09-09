# ActiveSitePredictor
Capstone Project worked on by Duncan Ritchie and Ian Carr for CS4800 @ App State Uni.

### Project Background
Duncan has experience in biology, which led to the idea of this project. 

Haskell is used for the initial 'half' while C++/Vulkan is used for the other.

Haskell is used for functional parsing and computational prediction, while
C++ is used for a protein model viewer.

The project proposal can be found in the 'project_prep' directory. It shows 
the necessary background information needed to understand what this project
is used for.

### Building
1. Install Haskell and CMake
2. CD into this project
3. Run `cmake -S . -B build --preset windows-debug` 
	* 'build' can be the build folder name of choice
	* valid presets are (windows/linux)-(debug/release)
4. Run `cmake --build build`
5. Compiled Executable should be in `build/bin`
