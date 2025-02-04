@echo off

if exist "%cd%/Submodules/Assimp/lib" (
    echo .
) else (
    cd Submodules/Assimp
    cmake CMakeLists.txt -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_INSTALL=OFF -DASSIMP_BUILD_DRACO=ON -DASSIMP_BUILD_ZLIB=ON -DCMAKE_CXX_FLAGS="/MD /EHsc" -DCMAKE_C_FLAGS="/MD" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=./lib
    cmake --build .
    cd ../../
)

premake5 vs2022