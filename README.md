# aether game utils
A collection of cross platform utilities for quickly creating small game prototypes in C++.

Modules and utilities include:
* Graphics and windowing
* Input
* Math
* C++ reflection
* Data structures
* Allocation
* Frame rate control and timing
* Logging

Dependencies:
* [SDL2](https://www.libsdl.org/)
* [libpng](http://www.libpng.org/pub/png/)
* [Assimp](https://github.com/assimp/assimp)
* [Catch2](https://github.com/catchorg/Catch2)

## Linux Setup
Installing dependencies:
```
sudo apt install libsdl2-2.0-0

sudo apt install libassimp-dev

mkdir ~/catch2 && cd ~/catch2
git clone https://github.com/catchorg/Catch2.git .
mkdir ./build && cd ./build
cmake ..
make
sudo make install
```

## MacOSX Setup
Installing dependencies:
```
brew install sdl2

brew install assimp

brew install libpng

mkdir ~/catch2 && cd ~/catch2
git clone https://github.com/catchorg/Catch2.git .
mkdir ~/catch2/build && cd ~/catch2/build
cmake ..
make
make install
```
