cmake_minimum_required(VERSION 3.15)
project(VadersystemProjekt VERSION 1.0.0 LANGUAGES C CXX)

# Standard inställningar
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Kompilatorvarningar
if(MSVC)
    add_compile_options(/W4)  # Windows: Hög varningsnivå
else()
    add_compile_options(-Wall -Wextra -pedantic)  # Linux/GCC: Alla varningar
endif()

# Platform-specifika definitioner
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7+
    add_definitions(-DWIN32_LEAN_AND_MEAN)  # Minimal Windows headers
endif()

# Inkludera gemensamma headers
include_directories(${CMAKE_SOURCE_DIR}/gemensam/include)

# Bygg server, klienter och tester
add_subdirectory(Server)
add_subdirectory(klient_c)
add_subdirectory(klient_cpp)
add_subdirectory(tests)

# Installation
install(DIRECTORY docs/ DESTINATION docs)
