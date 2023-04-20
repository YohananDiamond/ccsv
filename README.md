# ccsv

`ccsv` is a simple CSV parser library.

It was built so I could train my C and parser writing skills, but
nonetheless it aims to be functional.

## Compilation

- For everything: `make`
- To make the command-line utility: `make executable`
- To make the command-line utility: `make library`

Generated files are available in the `build/` folder, or wherever
`BUILD_DIR` was specified to be.

### Using as a C library

Just get `ccsv.c` and `ccsv.h`, and use them in your project.

### Using as a CLI executable

Compile the executable and grab it at `build/ccsv`.
