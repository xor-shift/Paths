# Paths

A path/ray tracer

## Libraries needed

- [{fmt}](https://github.com/fmtlib/fmt), Used for text output
- [TinyEXR](https://github.com/syoyo/tinyexr) For image output
- [LodePNG](https://github.com/lvandeve/lodepng) For image output likewise
- [GoogleTest](https://github.com/google/googletest) For tests (i swear i'll add some more tests later:tm:)
- [Lua](https://github.com/lua/lua) For configuration (see main.lua)
- [Sol2](https://github.com/ThePhD/sol2) As a (fast) Lua wrapper for C++

## Usage

- Compile the project
- Check out `main.lua` for an example camera & scene configuration file
- Run the project like so: `./a.out main.lua`

Notes:

- Interrupting the program will make it gracefully quit after saving resume files (this can be used to convert a resume binary to a png, heh)
- Loading of resume files does not run checks for the scene the resume file was made on, a single check for the image dimensions is made

## Eye candy

![render](https://github.com/xor-shift/Paths/blob/master/example%20render/304795_6019ms_14_8_4096.png?raw=true)

## Project layout

TODO: add layout

## To-Do

- [ ] Better documentation
- [x] Expression templates for linear algebra
    - [x] Vectors and vector operations
    - [x] Matrices and matrix operations
    - [x] vector matrix operations
    - [ ] Quaternions?
- [x] Preview mode (simplified Whitted)
- [ ] Proper BRDFs and importance sampling (>mfw mathlet) (well i am importance sampling if the BRDF is purely diffuse and samples are uniform, aren't i? checkmate mathheads)
- [ ] Use the GPU to some extent (probably infeasible at this point, would need a complete port(?))
