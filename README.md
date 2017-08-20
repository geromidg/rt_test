RT Test
=======

# Overview
RT Test is a latency detection program used to evaluate the real-time performance of an embedded device.<br>
It collects a specific amount of samples in a user-defined time period and provides statistics about the overall latency,<br>
It is similar to [cyclictest](https://wiki.linuxfoundation.org/realtime/documentation/howto/tools/cyclictest), although more limited.

To execute the program, run:<br>
`$ make`<br>
`$ sudo ./rt_test [cycle_time] [number_of_cycles]`

# Real-Time Linux
The normal Linux distribution does not offer **hard** real-time capabilities.<br>
In order to achieve those, a real-time preemption patch (*RT-PREEMPT*) is applied to the Linux kernel.

#### Performance
The overall performance of the OS is degraded, but **determinism** (predictability) is provided.<br>
In contrast to a normal OS that uses soft real-time, the average error (throughput) is increased, but the maximum error (predictability) is decreased.<br>
This means that *throughput is not the top priority* of the OS, and will be sacrificed if needed.

#### RT-PREEMPT Patch
For resources about building and configuring the real-time kernel, see [this](https://wiki.linuxfoundation.org/realtime/documentation/howto/applications/preemptrt_setup) page.<br>
For information on how to build a real-time application, see [this](https://wiki.linuxfoundation.org/realtime/documentation/howto/applications/application_base) page.

#### Platform
This program is aimed at devices running real-time Linux.<br>
For experimental purposes, it was run on a **BCM2837** board (Raspberry Pi 3).<br>
The board is comprised of a quad-core ARM Cortex-A53 (ARMv8 architecture), running on 1.2 GHz, with 1GB RAM (power consumption is around 800 mA).<br>
The hosting OS is a Linux flavor, specific to RPi, called Raspbian.<br>
The tests run on a 4.4.34 Linux kernel.

# Implementation
In order to collect all timestamps and provide latency statistics, a **cyclic executive task** is created on a separate thread that runs on a user-defined cycle time.<br>
At the start of every iteration of the sampling process, the cycle time is added to the task's timer. After the sampling is done, the task sleeps for the remaining of the time before starting a new iteration.

A scheduler **statistics module** was created in order to monitor the performance of the system during the sampling. It monitors the average, minimum and maximum latency errors during a test run.<br>
Each time a sample is collected, these statistics are updated with regard to the previous sample.

# Tests & Results
In order to evaluate the magnitude of the latency, two experiments were carried out on the embedded system.<br>
Below are the plots of the latency of every sample (with respect to its previous sample) for every experiment.<br>
The statistics of every run are also provided.

1. Cycle time of 10 ms.<br>
Average Error: 06.12 us<br>
Min Error: 00.02 us<br>
Max Error: 72.58 us

![Experiment 1](docs/latency_10.png)

2. Cycle time of 100 ms.<br>
Average Error: 14.82 us<br>
Min Error: 00.02 us<br>
Max Error: 66.02 us
 
![Experiment 2](docs/latency_100.png)

# Problems & Improvements
Since the user can define the number of samples to be collected by the test program with an argument, the required **memory is allocated on runtime**.<br>
A way to avoid this would be for the user to define the desired number of samples at compile-time.<br><br>
Static allocation should be, in general, preferred in an embedded system.<br>
If the application is safety-critical, dynamic allocation should be avoided at all costs, since an allocation failure could be catastrophic.<br>
<br><br><br><br>

*Dimitrios Panagiotis G. Geromichalos*<br>
*August, 2017*