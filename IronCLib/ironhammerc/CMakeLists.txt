cmake_minimum_required(VERSION 3.20)
project(PortabilityTest C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(hammer_ironclib main.c)

if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(hammer_ironclib PRIVATE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

if (MSVC)
    target_compile_options(hammer_ironclib PRIVATE /W4 /WX)
endif()