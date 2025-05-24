cmake_minimum_required(VERSION 3.0)

project(cbemu LANGUAGES C)

add_executable(cbemu main.c)

install(TARGETS cbemu RUNTIME DESTINATION bin)
