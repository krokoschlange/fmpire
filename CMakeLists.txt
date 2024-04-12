cmake_minimum_required(VERSION 3.0)

project(fmpire)

add_executable(fmpire main.cpp)

install(TARGETS fmpire RUNTIME DESTINATION bin)
