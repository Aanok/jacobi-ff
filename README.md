# jacobi-ff
FastFlow and C++11 implementations of Jacobi iterative method for linear systems.

## Running
`make testmic` will start a test run on the Knight's Corner machine.

`make testxeon` will start a test run on the local Xeon E5.

Test runs will build all necessary binaries, gather data and automatically generate plots in the `tests` subfolder.

All binaries can be run manually with a syntax like `program size max_iterations nworkers`.

## Building
A simple `make` will generate all binaries. `make build_xeon` and `make build_mic` can be used to only target a specific architecture.

Please note the Makefile is preconfigured with the location of FastFlow as is on the r720 machine: it will need to be manually edited to compile on a different host.
