# Benchmarks

An optional and highly experimental benchmarking rig for VanillaRTB, requires google-benchmark
package (else is excluded from the build).

Both Google Benchmark and VanillaRTB should be built in the release mode (`cmake -B Release ...`)
and CPU scaling should be disabled when running the benchmarks (`cpupower frequency-set -g performance`)
if realistic results are required.

## Buidling benchmark library (optional step)
If Google Benchmark library is not available in the target environment, it can be manually built.
Commands depend on the target environment but are usually similar to the ones below.

    $ cd $work_dir
    $ git clone 'https://github.com/google/benchmark.git'
    $ mkdir benchmark-build
    $ cd benchmark-build
    $ chrt -b 0 cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${HOME}/pkg/tp/benchmark-$(cd ../benchmark && git rev-parse --short HEAD) -G Ninja ../benchmark
    $ chrt -b 0 ninja -j$(nproc) -l$(nproc) install

## Building VanillaRTB
    $ cd $work_adir
    $ git clone 'https://github.com/venediktov/vanilla-rtb'
    $ mkdir vanilla-rtb-build
    $ cd vanilla-rtb-build
    $ chrt -b 0 cmake -DCMAKE_BUILD_TYPE=Release -Dbenchmark_DIR=/path/to/benchmark/installation -G Ninja ../vanilla-rtb
    $ chrt -b 0 ninja -j$(nproc) -l$(nproc)

## Running Benchmarks

Benchmarks can be run either as a build target

    $ cmake --build . --target benchmark

or directly (specifically, if command line customization or debugging is required)

    $ cd $work_dir
    $ cd vanilla-rtb-build
    $ benchmarks/vanilla-rtb-benchmarks

The expected out is something like

    $ ninja benchmark
    [0/1] cd .../vanilla-rtb-build/benchmarks && .../vanilla-rtb-build/benchmarks/vanilla-rtb-benchmarks
    Run on (4 X 2997 MHz CPU s)
    2017-04-16 15:23:40
    --------------------------------------------------------------------------------------------------------
    Benchmark                                                                 Time           CPU Iterations
    --------------------------------------------------------------------------------------------------------
    GenericDslBenchmarkFixture/generic_dsl_extract_request_benchmark     343695 ns     343692 ns       2036   2.39187MB/s


## Editing the README.md
The [MarkDown Preview Plus Chrome Plugin](https://www.google.ch/?q=markdown+preview+plus+chrome+plugin)
in the GitHub mode was used to validate the markup syntax. Once installed the plugin should be permissioned
to access file URLs on the Chrome plugin configuration page.
