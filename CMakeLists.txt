cmake_minimum_required(VERSION 3.20)

project(Lab1_GroupProject CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    Lab1App
    main.cpp
    shared_types.h
    student_a.cpp
    student_b.cpp
)

# Файли з кирилицею в UTF-8: без цього MSVC може неправильно прочитати рядки.
if(MSVC)
    target_compile_options(Lab1App PRIVATE /utf-8)
endif()
