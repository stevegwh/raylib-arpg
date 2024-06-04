cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=/Applications/CLion.app/Contents/bin/ninja/mac/ninja -DBUILD_EDITOR:BOOL=ON -G Ninja -S /Users/steve/CLionProjects/BG_Raylib -B /Users/steve/CLionProjects/BG_Raylib/cmake-build-debug


cmake --build /Users/steve/CLionProjects/BG_Raylib/cmake-build-debug --target editor -j 6

cmake-build-debug/editor/editor



