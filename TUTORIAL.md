# Tutorial {#tutorial}

The [playable example](https://johnhues.github.io/aether-game-utils/examples/22_read_me/) is a ~200 line first person exploration game with keyboard, mouse, touch, and gamepad controls, textured geometry, and basic kinematic physics. It's a single source file, [`22_ReadMe.cpp`](https://github.com/johnhues/aether-game-utils/blob/main/examples/22_ReadMe.cpp), which only includes [`aether.h`](https://github.com/johnhues/aether-game-utils/blob/main/aether.h) (which handles linking the required system libraries), so it can be built with nothing but a compiler.

## Getting the Files
Create a folder containing the following, with the level files in a `data` folder:
```
aether.h
main.cpp (main.mm on Mac)
data/level.obj
data/level.tga
```
Download [aether.h](https://raw.githubusercontent.com/johnhues/aether-game-utils/main/aether.h), [22_ReadMe.cpp](https://raw.githubusercontent.com/johnhues/aether-game-utils/main/examples/22_ReadMe.cpp) (saved as `main.cpp`, or `main.mm` on Mac), [level.obj](https://raw.githubusercontent.com/johnhues/aether-game-utils/main/examples/data/level.obj) and [level.tga](https://raw.githubusercontent.com/johnhues/aether-game-utils/main/examples/data/level.tga), or from a terminal in the new folder run:
```
curl -o aether.h https://raw.githubusercontent.com/johnhues/aether-game-utils/main/aether.h
curl -o main.cpp https://raw.githubusercontent.com/johnhues/aether-game-utils/main/examples/22_ReadMe.cpp
curl --create-dirs -o data/level.obj https://raw.githubusercontent.com/johnhues/aether-game-utils/main/examples/data/level.obj
curl --create-dirs -o data/level.tga https://raw.githubusercontent.com/johnhues/aether-game-utils/main/examples/data/level.tga
```

## Building and Running on Mac
Rename `main.cpp` to `main.mm`. Open the `Terminal` application. Type `cd` (with a space after it) and then drag the folder containing `main.mm` into the terminal window and press enter. With Xcode installed run the following:
```
clang++ -std=c++17 -fmodules -fcxx-modules main.mm && ./a.out
```

## Building and Running on Windows
With Visual Studio installed, right click inside the folder and choose `Open in Terminal`. Run `"C:\Program Files\Microsoft Visual Studio\20XX\EDITION\VC\Auxiliary\Build\vcvars64.bat"`, replacing `20XX` with the year, and `EDITION` with `Community` etc. Finally build it with:
```
cl /std:c++17 -D_UNICODE -DUNICODE main.cpp
```
and run `main.exe`.

## Building and Running with Emscripten
Also download [index.html](https://raw.githubusercontent.com/johnhues/aether-game-utils/main/examples/index.html) into the folder. Change to the folder, and with `emsdk` activated run:
```
em++ -std=c++17 -s MIN_WEBGL_VERSION=3 -s MAX_WEBGL_VERSION=3 -s ALLOW_MEMORY_GROWTH main.cpp -o index.js
```
You can run and play the result of the above command by running this in the same folder:
```
emrun index.html
```

## Running an Example on iOS
The CMake-built examples can be deployed to a connected iOS device. Make sure your device is setup for development, trusted, connected, and unlocked. Set the following `AE_APPLE_DEVELOPMENT_TEAM` to your Apple "Organizational Unit" ID and `AE_IOS_DEVICE` to the device's UUID (find it with `xcrun devicectl list devices`). From a clone of the repository, the following configures, builds, installs, launches (held at entry), then attaches lldb:
```
export AE_APPLE_DEVELOPMENT_TEAM=XXXXXXXXXX
export AE_IOS_DEVICE=XXXXXXXX-XXXXXXXXXXXXXXXX
TARGET=06_triangle && \
  cmake --preset ios && \
  cmake --build --preset ios-debug --target $TARGET && \
  scripts/ios-install.sh $TARGET && \
  scripts/ios-launch.sh --console --start-paused $TARGET && \
  scripts/ios-debug.sh $TARGET
```
Device stdout/stderr is captured by `ios-launch.sh --console` into `/tmp/aether-ios-console.log`. Follow it in a second terminal:
```
tail -F /tmp/aether-ios-console.log
```
