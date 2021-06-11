cmake_minimum_required(VERSION 3.15)
project(smash C)
add_executable(smash smash.c includes/linenoise-master/linenoise.c includes/methods.c)

