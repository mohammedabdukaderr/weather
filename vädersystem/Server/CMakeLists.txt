# Server CMakeLists.txt
project(VaderServer C)

# Samla alla C-filer i src/
file(GLOB SERVER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Skapa server-executable
add_executable(vaderserver ${SERVER_SOURCES})

# Inkludera server headers
target_include_directories(vaderserver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform-specifika länkningar
if(WIN32)
    target_link_libraries(vaderserver ws2_32)  # Winsock på Windows
else()
    target_link_libraries(vaderserver pthread)  # POSIX threads på Linux
endif()

# Installation
install(TARGETS vaderserver DESTINATION bin)
