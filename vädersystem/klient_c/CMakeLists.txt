# C-klient CMakeLists.txt
project(VaderKlientC C)

# Samla alla C-filer
file(GLOB KLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Skapa executable
add_executable(vaderklient_c ${KLIENT_SOURCES})

# Inkludera headers
target_include_directories(vaderklient_c PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform-specifik länkning
if(WIN32)
    target_link_libraries(vaderklient_c ws2_32)
endif()

# Installation
install(TARGETS vaderklient_c DESTINATION bin)
