# Paths

A path/ray tracer (no path tracing as of now)

## Libraries needed

- [{fmt}](https://github.com/fmtlib/fmt), Used for text output
- [SFML](https://github.com/SFML/sfml), Used to display rendered images to the screen, might switch to something lighter that gets this job done later on
- ImGui, Used for UI alongside:
- ImGui-SFML

## Project layout

TODO: add layout

## To-Do

- [ ] Better documentation
- [ ] Separation of SFML components from lib/gfx into its own sub folder somewhere
- [ ] Expression templates for linear algebra
    - [x] Vectors and vector operations
    - [ ] Matrices and matrix operations
    - [ ] Vector matrix operations
- [x] Preview mode (simplified Whitted)
- [ ] Non-preview mode (actual MC path tracing)
    - [ ] MLT for PT (very late goal)
    - [ ] Volume interactions to some extent (this is also low priority)
