#!/bin/bash

set -xe

g++ -Iinclude/ -O0 -DDEBUG -std=c++17 -g -o main *.cpp include/scope.hpp
gdb --args ./main ../input.earl