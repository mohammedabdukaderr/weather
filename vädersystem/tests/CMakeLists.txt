# Tests CMakeLists.txt
project(VaderTests C)

# Enkelt testprogram för protokollet
add_executable(test_protokoll test_protokoll.c)
target_include_directories(test_protokoll PRIVATE
    ${CMAKE_SOURCE_DIR}/gemensam/include
)

# Installation
install(TARGETS test_protokoll DESTINATION bin)
