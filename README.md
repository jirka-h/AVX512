## Set of simple C programs to study packed double performance with AVX-512

Programs are using the core extension AVX-512F (AVX-512 Foundation) - see [Wikipedia article on AVX-512.](https://en.wikipedia.org/wiki/AVX-512#Legacy_instructions_with_EVEX-encoded_versions)

### Compilation
```
make
```

It will create binaries in the `bin` directory and disassembly of binaries in the [objdump](objdump) folder.

You can compile and study the generated assembly on any x86_64 system. To run the binaries, AVX-512 enabled CPU is required. See the [list of supported CPUs](https://en.wikipedia.org/wiki/AVX-512#CPUs_with_AVX-512) or run [bin/detect_avx](detect_avx.c) to find out. 

### Measure runtime
The main program is [harmonic_series.c](harmonic_series.c) - it will sum up [Harmonic series](https://en.wikipedia.org/wiki/Harmonic_series_(mathematics)) using:
* AVX-512F (8 packed doubles)
* AVX2 (4 packed doubles)
* no AVX

You can measure runtime either manually
*  AVX-512 `./harmonic_series 0 4e9` - this will compute the sum of the first 8x4e9=3.2e10 terms of the series using operations on {8 packed doubles}.
*  AVX2 -   `./harmonic_series 1 4e9` - this will sum up the first 3.2e10 terms of the series using operations on {4 packed doubles}.
*  no AVX - `./harmonic_series 1 4e9` - this will sum up the first 3.2e10 terms of the series using operations on standard doubles (no SIMD instructions). 

You can also run the measurement automatically with provided script [test_harmonic_series.sh](results/test_harmonic_series.sh):

```
cd results
./test_harmonic_series.sh
```

It requires [GNU Parallel](https://www.gnu.org/s/parallel). On the Fedora system, you can install it with `dnf install parallel`. It also relies on:
* `sensors` (lm_sensors rpm package) to measure CPU and system temperature
* `turbostat` (kernel-tools rpm package) to report processor frequency and idle statistics
* `perf stat` (perf rpm package) to gather performance counter statistics
* GNU [`time` program](https://www.gnu.org/software/time/) (rpm package `time`). 

It will run different copies of the `harmonic_series` binary in parallel to test how the system behaves at various load. See the example of results
[here](results/Intel_Platinum_8351N_CPU_2.40GHz_harmonic_series/)
(generated on [Intel Platinum 8351N CPU.](https://www.intel.com/content/www/us/en/products/sku/212288/intel-xeon-platinum-8351n-processor-54m-cache-2-40-ghz/specifications.html)

There are also programs to measure the performance of the basic operations like
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

It will run all binaries, including [harmonic_series.c](harmonic_series.c), and summarize the results in file `results.txt`. See the example of results
[here.](results/Intel_Platinum_8351N_CPU_2.40GHz/results.txt)

### Discussion of results
#### Intel Skylake & Icelake architecture
On Intel CPUs, the AVX-512 division (in terms of floating operations per second) shows no acceleration on Skylake (Intel Gold 6126) and only minor improvement on Icelake (Intel Platinum 8351N) compared to AVX-256. On the contrary, other tested basic packed double operations (like addition and multiplication) run two times faster with AVX-512 compared with AVX2.

The explanation is provided, for example, in [this discussion on stackoverflow.com:](https://stackoverflow.com/questions/4125033/floating-point-division-vs-floating-point-multiplication/45899202#45899202)

*The divide / sqrt unit is not fully pipelined, for reasons explained in @NathanWhitehead's answer. The worst ratios are for 256b vectors, because (unlike other execution units) the divide unit is usually not full-width, so wide vectors have to be done in two halves. A not-fully-pipelined execution unit is so unusual that Intel CPUs have an arith.divider_active hardware performance counter to help you find code that bottlenecks on divider throughput instead of the usual front-end or execution port bottlenecks. (Or more often, memory bottlenecks or long latency chains limiting instruction-level parallelism causing instruction throughput to be less than ~4 per clock).*

I have confirmed it with the [llvm-mca](https://man.archlinux.org/man/extra/llvm/llvm-mca.1.en) and [perf-stat](doc/linux-perf.pdf).
Compare the [llvm-mca results](analysis/Intel_Platinum_8351N_CPU_2.40GHz_llvm-mca) on Intel Platinum 8351N CPU for [div_avx256](analysis/Intel_Platinum_8351N_CPU_2.40GHz_llvm-mca/div_avx256f.log) and [div_avx512](analysis/Intel_Platinum_8351N_CPU_2.40GHz_llvm-mca/div_avx512f.log). Notice that Block RThroughput (Block Reciprocal Throughput) is 10.3 for div_avx256 and 16.0 for div_avx512. Since the avx512 variant makes 2 times more floating point operations, the resulting throughput is (using arbitrary units) 100/10.3=9.7 and 200/16=12.5, respectively. This is in line with `perf stat` [results](analysis/Intel_Platinum_8351N_CPU_2.40GHz) showing that both div_avx256 and div_avx512 are backed bound, with execution slots being in 89% of time in the `Instruction executed, waiting to be retired` state. 

#### AMD Zen4 architecture
Zen4 has implemented AVX-512 using two parallel AVX-256 units (double-pumping). The runtime of all simple operations improves by a factor of 4x between standard doubles and AVX2 (4 packed doubles) and 2x between AVX2 and AVX-512 (8 packed doubles). Here is an example for the division operation:

Program       Wall clock time (minute:seconds)
* div_plain   1:53
* div_avx256f 0:28
* div_avx512f 0:14

See [full results.](results/AMD_EPYC_9654_96-Core_Processor/results.txt)

The results for harmonic series summation are woth some futher investigation:

Program       Wall clock time (minute:seconds)
HarmonicSeriesPlain 			0:41.33
HarmonicSeriesAVX256 			0:10.32
HarmonicSeriesAVX512 			0:09.82

AVX-256 shows runtime improvement by 4x over the standard double FP arithmetics. However, moving to AVX-512 shows no performance improvement anymore. It's also interesting to notice that HarmonicSeriesAVX256 runtime is significantly shorter than  for div_avx256f (10 seconds versus 28 seconds), even though both programs are doing the same amount of the AVX-256 divisions. However, HarmonicSeriesAVX256 is doing divisions on different numbers, meanining that CPU can use instruction pipelining and utilize both AVX-256 units simultaneously. This is why the AVX-512 version does not bring any improvement - AVX-256 is already fully utilizitating the both units anyhow. Moving to AVX-512 cannot increase the utilization anymore. By comparing `perf stat` results for [HarmonicSeriesAVX256](analysis/AMD_EPYC_9654_96-Core_Processor/HarmonicSeriesAVX256.log) and [HarmonicSeriesAVX512.log](analysis/AMD_EPYC_9654_96-Core_Processor/HarmonicSeriesAVX512.log) we can see that

* fp_ret_sse_avx_ops.all (the number of retired SSE/AVX opearations) is the same in both cases
* fpu_pipe_assignment.total is 2x higher for AVX-256 than for AVX-512

We can conclude that the workload is backend bound, and runtime is dominated by division. AVX-256 version already fully utilizes the HW, moving to AVX-512 does not bring any runtime improvement. However, it havles the number of instructions and fpu_pipe_assignments. 

#### Comparing FP division performance of AMD Zen4 and Intel Icelake for packed doubles
AVX-256 and AVX-512 packed double division can be up to 2x faster on AMD Zen4 than on Intel Icelake. AMD Zen4 has in total [four 256-bit ALUs](https://chipsandcheese.com/2022/11/05/amds-zen-4-part-1-frontend-and-execution-engine/), and compared to the standard double division, the measured speed up for AVX division was up to 11x (113 compared to 10 seconds wallclock time) with 4 independent 256-bit packed double vectors or with two independent 512-bit packed double vectors. Intel Icelake AVX division shows maximum speed up compared to standard doubles by 6.9x (145 versus 21 seconds) for two independent 256-bit packed double vectors. It seems like the Intel CPU has just two 256-division units. 

```
                                     AMD Zen4 EPYC 9654    Intel Platinum 8351N
div_plain                            1:53.82               2:25.68
div_avx256f                          0:28.47               0:35.69
div_avx512f                          0:14.25               0:30.38
div_two_independent_vectors_avx256f  0:15.26               0:21.32
div_two_independent_vectors_avx512f  0:09.82               0:21.55
div_four_independent_vectors_avx256f 0:10.07               0:21.33
div_four_independent_vectors_avx512f 0:09.82               0:21.33
```

