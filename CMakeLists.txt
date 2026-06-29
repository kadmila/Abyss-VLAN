cmake_minimum_required(VERSION 4.3)
project(AbyssVlan)

add_subdirectory(platform)

add_executable(AbyssVlan
    src/main.c
)

target_link_libraries(AbyssVlan PRIVATE platform)