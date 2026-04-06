#!/bin/sh
git submodule update --init --recursive
cmake -B build -S . --preset debug
