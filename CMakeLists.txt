cmake_minimum_required(VERSION 3.13)
project(ep13 C)

set(CMAKE_C_STANDARD 11)

add_executable(ep13
        scanner.c)

add_executable(ep13full
        scanner-full.c)

add_executable(cust
        custpars.c)