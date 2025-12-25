# C++-klient CMakeLists.txt
project(VaderKlientCpp CXX)

# Samla alla C++-filer
file(GLOB KLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Skapa executable
add_executable(vaderklient_cpp ${KLIENT_SOURCES})

# Inkludera headers
target_include_directories(vaderklient_cpp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform-specifik länkning
if(WIN32)
    target_link_libraries(vaderklient_cpp ws2_32)
endif()

# Installation
install(TARGETS vaderklient_cpp DESTINATION bin)
