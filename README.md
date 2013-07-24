primekincpp
===========

My first experience with C++ and a naive implementation of the Sieve of Atkin for finding primes.

It uses OpenMP for parallelisation and needs to be compiled with `-fopenmp` in GCC or the equivalent with other compilers.

It stores the sieve as bits and only odds, using 1/16th bits of the limit of primes found.
Example: With a limit of 1e6 it will take `0.5e9 bits = 60 mb`.

On a hexacore pc and compiled with gcc, it takes 2.4 seconds to reach 1e9 and 29.8 seconds for 1e10.

This program isn't fast and it isn't memory efficient, but I had fun writing it and having some fun with bitwise operators and thinking in parallel (thank you `omp atomic`!).
