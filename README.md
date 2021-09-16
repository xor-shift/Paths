# Paths

A path/ray tracer (no path tracing as of now)

## Libraries needed

- [{fmt}](https://github.com/fmtlib/fmt), Used for text output
- [LodePNG](https://github.com/lvandeve/lodepng), Used for image output
- [N. Lohmann's JSON library](https://github.com/nlohmann/json), Used for scene configurations and resume metadata
- [GCEM](https://github.com/kthohr/gcem), I think I used it somewhere?

{fmt} and json are managed by conan and lodepng is a submodule, you need to get fmt from your favourite package manager.

## Usage

- Compile the project
- Check out `conf.example.json` for an example camera & scene configuration file
- Run the project like so: `./a.out conf.json`

Notes:

- Interrupting the program will make it gracefully quit after saving resume files (this can be used to convert a resume binary to a png, heh)
- Loading of resume files does not run checks for the scene the resume file was made on, a single check for the image dimensions is made

## Eye candy

10k samples, 1280x720, a few thousand triangles and 3 disc lights, no explicit light sampling  
![render from commit 5319a06 or around that one](https://github.com/xor-shift/Paths/blob/master/example%20render/out.png?raw=true)

## Project layout

TODO: add layout

## To-Do

- [ ] Better documentation
- [x] Expression templates for linear algebra
    - [x] Vectors and vector operations
    - [x] Matrices and matrix operations
    - [x] Vector matrix operations
- [x] Preview mode (simplified Whitted)
- [x] Non-preview mode (actual MC path tracing)
    - [ ] MLT for PT (very late goal)
    - [ ] Volume interactions to some extent (this is also low priority, unfeasible to do without MLT)
- [ ] Use the GPU to some extent (probably infeasible at this point, would need a complete port)
