# priority-queue-benchmark
Creating benchmark cases to quantify the performance of different implementations of priority queues

## Setup
- export BOOST_PATH={PATH_TO_BOOST_LIB}
- install pandas and pytest in Python
- GCC compiler

## Test Run
```pytest testrun.py::test_single -s``` running single trial test  
```pytest testrun.py::test_simple_I -s``` running simple cases in TEST I  
```pytest testrun.py::test_simple_II -s``` running simple cases in TEST II  
```pytest testrun.py::test_I -s``` slower, running all cases in TEST I  
```pytest testrun.py::test_II -s``` slower, running all cases in TEST II  
```pytest testrun.py::test_all -s``` slower, running all cases available  

test result csv is generated to the results folder

## TEST Candidates
### A) std::priority_queue
### B) boost::pairing_heap
### C) pairing_heap_priqueue
### D) skiplist_priqueue
### E) arch_aware_heap (type I only, int payload)

## TEST I: add random elements one by one
### TESTID:
#### i) 1k elements
#### ii) 10k elements
#### iii) 100k elements
#### iv) 1M elements
#### v) 10M elements
#### vi) 100M elements


## TEST II: find the shortest path in graph
### TESTID:
#### i) 1k elements
#### ii) 10k elements
#### iii) 100k elements
#### iv) 1M elements
#### v) 10M elements
#### vi) 100M elements

## TEST Results
### hardware: 16 x 11th Gen Intel(R) Core(TM) i7-11700F @ 2.50GHz CPU Cores

#### test_I_result.csv

| cand | type | id  | size      | original  | original per-op | my time  | my per-op |
|------|------|-----|-----------|-----------|-----------------|----------|-----------|
| A    | I    | i   | 1000      | 55us      | 55ns            | 37us     | 37ns      |
| A    | I    | ii  | 10000     | 2.362ms   | 236.2ns         | 0.47ms   | 47.10ns   |
| A    | I    | iii | 100000    | 12.765ms  | 127.65ns        | 5.52ms   | 55.24ns   |
| A    | I    | iv  | 1000000   | 118.329ms | 118.33ns        | 55.89ms  | 55.89ns   |
| A    | I    | v   | 10000000  | 1.8307s   | 183.07ns        | 692.34ms | 69.23ns   |
| A    | I    | vi  | 100000000 | 26.9805s  | 269.81ns        | 7.92s    | 79.16ns   |
| B    | I    | i   | 1000      | 167us     | 167ns           | 196us    | 196ns     |
| B    | I    | ii  | 10000     | 2.491ms   | 249.1ns         | 3.22ms   | 322.10ns  |
| B    | I    | iii | 100000    | 32.491ms  | 324.91ns        | 43.39ms  | 433.91ns  |
| B    | I    | iv  | 1000000   | 1.0923s   | 1.09us          | 402.79ms | 402.79ns  |
| B    | I    | v   | 10000000  | 25.3795s  | 2.54us          | 5.52s    | 551.62ns  |
| B    | I    | vi  | 100000000 | 563.0792s | 5.63us          | 58.59s   | 585.92ns  |
| C    | I    | i   | 1000      | 92us      | 92ns            | 121us    | 121ns     |
| C    | I    | ii  | 10000     | 1.304ms   | 130.4ns         | 1.64ms   | 163.70ns  |
| C    | I    | iii | 100000    | 17.832ms  | 178.32ns        | 25.59ms  | 255.93ns  |
| C    | I    | iv  | 1000000   | 958.796ms | 958.80ns        | 216.59ms | 216.59ns  |
| C    | I    | v   | 10000000  | 19.5441s  | 1.95us          | 2.95s    | 294.54ns  |
| C    | I    | vi  | 100000000 | 367.4421s | 3.67us          | 32.57s   | 325.71ns  |
| D    | I    | i   | 1000      | 163us     | 163ns           | 249us    | 249ns     |
| D    | I    | ii  | 10000     | 5.546ms   | 554.6ns         | 3.13ms   | 313.00ns  |
| D    | I    | iii | 100000    | 49.966ms  | 499.66ns        | 47.69ms  | 476.88ns  |
| D    | I    | iv  | 1000000   | 1.4865s   | 1.49us          | 692.32ms | 692.32ns  |
| D    | I    | v   | 10000000  | 29.5455s  | 2.95us          | 7.46s    | 745.89ns  |
| D    | I    | vi  | 100000000 | 695.3429s | 6.95us          | 76.36s   | 763.64ns  |
| E    | I    | i   | 1000      | n/a       | n/a             | 28us     | 28ns      |
| E    | I    | ii  | 10000     | n/a       | n/a             | 398us    | 39.80ns   |
| E    | I    | iii | 100000    | n/a       | n/a             | 5.305ms  | 53.05ns   |
| E    | I    | iv  | 1000000   | n/a       | n/a             | 56.990ms | 56.99ns   |
| E    | I    | v   | 10000000  | n/a       | n/a             | 727.086ms| 72.71ns   |
| E    | I    | vi  | 100000000 | n/a       | n/a             | 7.717618s| 77.18ns   |


#### test_II_result.csv

| cand | type | id  | size     | original  | original per-op | my time  | my per-op |
|------|------|-----|----------|-----------|-----------------|----------|-----------|
| A    | II   | i   | 1000     | 3.815ms   | 3.82us          | 1.83ms   | 1.83us    |
| A    | II   | ii  | 10000    | 21.626ms  | 2.16us          | 25.65ms  | 2.57us    |
| A    | II   | iii | 100000   | 648.841ms | 6.49us          | 589.10ms | 5.89us    |
| A    | II   | iv  | 1000000  | 11.4406s  | 11.44us         | 9.89s    | 9.89us    |
| A    | II   | v   | 10000000 | 197.4666s | 19.75us         | 156.91s  | 15.69us   |
| B    | II   | i   | 1000     | 1.548ms   | 1.55us          | 1.98ms   | 1.98us    |
| B    | II   | ii  | 10000    | 41.349ms  | 4.13us          | 36.61ms  | 3.66us    |
| B    | II   | iii | 100000   | 636.757ms | 6.37us          | 586.71ms | 5.87us    |
| B    | II   | iv  | 1000000  | 11.4765s  | 11.48us         | 9.52s    | 9.52us    |
| B    | II   | v   | 10000000 | 217.4507s | 21.75us         | 149.18s  | 14.92us   |
| C    | II   | i   | 1000     | 3.041ms   | 3.04us          | 1.89ms   | 1.89us    |
| C    | II   | ii  | 10000    | 28.246ms  | 2.82us          | 26.76ms  | 2.68us    |
| C    | II   | iii | 100000   | 696.461ms | 6.96us          | 597.52ms | 5.98us    |
| C    | II   | iv  | 1000000  | 10.9391s  | 10.94us         | 9.41s    | 9.41us    |
| C    | II   | v   | 10000000 | 206.0349s | 20.60us         | 153.10s  | 15.31us   |
| D    | II   | i   | 1000     | 5.445ms   | 5.45us          | 2.12ms   | 2.12us    |
| D    | II   | ii  | 10000    | 23.252ms  | 2.33us          | 35.96ms  | 3.60us    |
| D    | II   | iii | 100000   | 663.265ms | 6.63us          | 611.70ms | 6.12us    |
| D    | II   | iv  | 1000000  | 12.1386s  | 12.14us         | 9.64s    | 9.64us    |
| D    | II   | v   | 10000000 | 233.5522s | 23.36us         | 146.35s  | 14.64us   |
