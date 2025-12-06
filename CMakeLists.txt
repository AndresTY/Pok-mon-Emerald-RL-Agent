cmake_minimum_required(VERSION 3.18)
project(PokemonRLAgent VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()

list(APPEND CMAKE_PREFIX_PATH "/opt/libtorch")

find_package(Torch REQUIRED)
if(NOT Torch_FOUND)
    message(FATAL_ERROR 
        "LibTorch not found /opt/libtorch\n"
        "Download: https://pytorch.org/get-started/locally/\n"
        "Install : sudo mv libtorch /opt/\n"
        "Or : -DCMAKE_PREFIX_PATH=/path/to/libtorch"
    )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

find_package(SDL2 REQUIRED)
if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found.")
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2_TTF SDL2_ttf)

if(NOT SDL2_TTF_FOUND)
    find_path(SDL2_TTF_INCLUDE_DIRS
        NAMES SDL_ttf.h
        PATHS 
            /usr/include/SDL2
            /usr/local/include/SDL2
    )
    
    find_library(SDL2_TTF_LIBRARIES
        NAMES SDL2_ttf
        PATHS
            /usr/lib
            /usr/local/lib
    )
    
    if(SDL2_TTF_INCLUDE_DIRS AND SDL2_TTF_LIBRARIES)
        set(SDL2_TTF_FOUND TRUE)
        message(STATUS "SDL2_ttf found manually")
    else()
        message(FATAL_ERROR 
            "SDL2_ttf not found.\n"
        )
    endif()
endif()

find_path(MGBA_INCLUDE_DIR
    NAMES mgba/core/core.h
    PATHS 
        /usr/include
        /usr/local/include
        /opt/local/include
    DOC "Directorio de headers de mGBA"
)

find_library(MGBA_LIBRARY
    NAMES mgba
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
    DOC "Librería de mGBA"
)

if(NOT MGBA_INCLUDE_DIR OR NOT MGBA_LIBRARY)
    message(FATAL_ERROR 
        "mGBA not found.\n"
        "Install: https://archlinux.org/packages/extra/x86_64/mgba/\n"
        "Or: -DMGBA_INCLUDE_DIR=/path -DMGBA_LIBRARY=/path/libmgba.so"
    )
endif()

set(HEADERS
    ${CMAKE_SOURCE_DIR}/libraries/pokemon.h
    ${CMAKE_SOURCE_DIR}/libraries/MemoriaJuego.h
    ${CMAKE_SOURCE_DIR}/libraries/SistemaRecompensas.h
    ${CMAKE_SOURCE_DIR}/libraries/EventosEmerald.h
)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/pokemon.cpp
    ${CMAKE_SOURCE_DIR}/src/MemoriaJuego.cpp
    ${CMAKE_SOURCE_DIR}/src/SistemaRecompensas.cpp
)

add_executable(pokemon_rl_agent ${SOURCES} ${HEADERS})

target_include_directories(pokemon_rl_agent PRIVATE
    ${CMAKE_SOURCE_DIR}/libraries      
    ${CMAKE_SOURCE_DIR}/src           
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${MGBA_INCLUDE_DIR}
    ${TORCH_INCLUDE_DIRS}
)

target_link_libraries(pokemon_rl_agent PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${MGBA_LIBRARY}
    ${TORCH_LIBRARIES}
    pthread
)

target_compile_options(pokemon_rl_agent PRIVATE
    -Wall
    -Wextra
    -pedantic
)

file(GLOB ROM_FILES "${CMAKE_SOURCE_DIR}/*.gba")
if(ROM_FILES)
    foreach(ROM_FILE ${ROM_FILES})
        get_filename_component(ROM_NAME ${ROM_FILE} NAME)
        configure_file(${ROM_FILE} ${CMAKE_BINARY_DIR}/${ROM_NAME} COPYONLY)
        message(STATUS "ROM copied: ${ROM_NAME}")
    endforeach()
else()
    message(STATUS "No .gba in root file")
endif()

install(TARGETS pokemon_rl_agent
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include/pokemon_rl_agent
)

message(STATUS "═══════════════════════════════════════")
message(STATUS "")
message(STATUS "Project Structure:")
message(STATUS "  pokemon-rl/")
message(STATUS "  ├── CMakeLists.txt")
message(STATUS "  ├── libraries/")
message(STATUS "  │   └── *.h      (interfaces)")
message(STATUS "  ├── src/")
message(STATUS "  │   └── *.cpp")
message(STATUS "  ├── build/                    (generated during compilation)")
message(STATUS "  └── *.gba                     (ROM files)")
message(STATUS "")
message(STATUS "Build Instructions:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make -j$(nproc)")
message(STATUS "")
message(STATUS "═══════════════════════════════════════")
message(STATUS "")

