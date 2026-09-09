# ActiveSitePredictor
Capstone Project worked on by Duncan Ritchie and Ian Carr for CS4800 @ App State Uni.

# Building
1. Install Haskell and CMake
2. CD into this project
3. Run `cmake -S . -B build --preset windows-debug` 
	* 'build' can be the build folder name of choice
	* valid presets are (windows/linux)-(debug/release)
4. Run `cmake --build build`
5. Compiled Executable should be in `build/bin`
