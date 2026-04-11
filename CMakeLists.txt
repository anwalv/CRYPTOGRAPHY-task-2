cmake_minimum_required(VERSION 3.28)
project(CRYPTOGRAPHY_task_2 C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/O2 /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -funroll-loops)
endif()

include_directories(.)

add_executable(CRYPTOGRAPHY_task_2
        strumok_tables.c
        strumok_tables.h
        strumok.h
        strumok.c
        main.c
        strumok_tables.c
        use-example.c)