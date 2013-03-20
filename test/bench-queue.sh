#!/bin/bash

#queue for several benchmarks

test/bench.sh cpu "1 2 4 8 16 32 64 128 512 1024 2048" argon_108.inp bench_cpu_108.out
test/bench.sh gpu "1 2 4 8 16 32 64 128 512 1024 2048" argon_2916.inp bench_gpu_2916.out

test/bench.sh cpu "1 2 4 8 16 32 64 128 512 1024 2048" argon_2916.inp bench_cpu_2916.out
test/bench.sh gpu "1 2 4 8 16 32 64 128 512 1024 2048" argon_2916.inp bench_gpu_2916.out

test/bench.sh gpu "1 2 4 8 16 32 64 128 512 1024 2048" argon_78732.inp bench_gpu_78732.out
test/bench.sh gpu "1 2 4 8 16 32 64 128 512 1024 2048" argon_78732.inp bench_gpu_78732.out
