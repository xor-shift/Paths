#!/bin/bash

scc \
  CMakeLists.txt \
  conanfile.py \
  paths/ \
  lib/ \
  tests/ \
  main.lua
