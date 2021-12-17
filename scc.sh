#!/bin/bash

scc \
  CMakeLists.txt \
  conanfile.py \
  paths/ \
  Lib/ \
  tests/ \
  main.lua
