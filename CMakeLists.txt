cmake_minimum_required(VERSION 4.3)
project(AbyssVlan C)

add_subdirectory(platform)

add_executable(AbyssVlan
    src/main.c
    platform/windows/argparse.c

    third_party/argtable3/argtable3.c
)

target_link_libraries(AbyssVlan PRIVATE platform)