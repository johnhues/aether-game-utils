SET PATH=%PATH%;C:\Program Files\CMake\bin;
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=C:\Library ..
cmake --build . --config RelWithDebInfo --target INSTALL
cmake --build . --config Debug --target INSTALL
cd ..
