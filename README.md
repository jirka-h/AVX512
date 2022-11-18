## Set of simple C programs to study packed double performance with AVX-512

Programs are using the core extension AVX-512F (AVX-512 Foundation) - see [Wikipedia article on AVX-512.](https://en.wikipedia.org/wiki/AVX-512#Legacy_instructions_with_EVEX-encoded_versions)

### Compilation
```
make
```

It will create binaries in `bin` directory and disassembly of binaries in [objdump](objdump) folder.

You can compile and study the generated assembly on any x86_64 system. To run the binaries, AVX-512 enabled CPU is required. See [list of supported CPUs](https://en.wikipedia.org/wiki/AVX-512#CPUs_with_AVX-512) or run [bin/detect_avx](detect_avx.c) to find out. 

### Measure runtime
The main program is [harmonic_series.c](harmonic_series.c) - it will sum up [Harmonic series](https://en.wikipedia.org/wiki/Harmonic_series_(mathematics)) using
* AVX-512F (8 packed doubles)
* AVX2 (4 packed doubles)
* no AVX

You can measure runtime either manually
*  AVX-512 `./harmonic_series 0 4e9` - this will compute sum up the fist 8x4e9=3.2e10 terms of the series using operations on {8 packed doubles}.
*  AVX2 -   `./harmonic_series 1 4e9` - this will compute sum up the fist 3.2e10 terms of the series using oprations on {4 packed doubles}.
*  no AVX - `./harmonic_series 1 4e9` - this will compute sum up the fist 3.2e10 terms of the series using oprations on normal doubles (no SIMD instructions). 

You can also run the measurement automatically with provided script [test_harmonic_series.sh](results/test_harmonic_series.sh):

```
cd results
./test_harmonic_series.sh
```

It requires [GNU Parallel](https://www.gnu.org/s/parallel). On Fedora system, you can install it with `dnf install parallel`. It's also relies on `sensors`
(lm_sensors rpm package) to measure CPU and system temperature, `turbostat` (kernel-tools package) to report processor frequency and idle statistics,
`perf stat` (perf package) to gather performance counter statistics and finally on GNU [`time` program](https://www.gnu.org/software/time/) (package time). 

It will run a different copies of the `harmonic_series` binary in parallel to test how the system behaves at different load. See example of results
[here](results/Intel_Platinum_8351N_CPU_2.40GHz_harmonic_series/)
(generated on [Intel Platinum 8351N CPU ](https://www.intel.com/content/www/us/en/products/sku/212288/intel-xeon-platinum-8351n-processor-54m-cache-2-40-ghz/specifications.html).

There are also simple programs to measure the performance of simple operations like
* vaddpd
* vmulpd
* vdivpd
* vfmadd132pd

These are hard programmed to perform 4e9 operations with  AVX-512 (8 packed doubles) and 8e9 with AVX2 (aka AVX-256) (4 packed doubles) so that results can be compared.
You can run these manually like this:

```
cd bin
/usr/bin/time -v div_avx256f
/usr/bin/time -v div_avx512f
```

or automatically using [measure_runtime.sh](results/measure_runtime.sh) script.
```
cd results
./measure_runtime.sh
```

It will run all binaries including [harmonic_series.c](harmonic_series.c) and summarize the results in file `results.txt`. See example of results
[here.](results/Intel_Platinum_8351N_CPU_2.40GHz/results.txt)

### Discussion of results
Zen4 double pumped (two AVX-256 units in parallel)
Skylake & Icelake - no change in performance for vdivpd
harmonic_series - pipelining

