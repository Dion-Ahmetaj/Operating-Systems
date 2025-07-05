# Operating-Systems
**Pizza Order System**   Simulates restaurant workflow (orders→baking→delivery) using pthreads. Synchronizes resources (operators/cooks/ovens/drivers) via mutexes &amp; condition variables. 

This repository contains a POSIX threads-based simulation of a pizza restaurant's order fulfillment workflow. The system models telephone operators taking orders, cooks preparing pizzas, ovens baking concurrently, and drivers handling deliveries. Implemented in C, it uses mutexes and condition variables for resource synchronization (limited operators/cooks/ovens/drivers). Customers arrive at random intervals, order random pizza quantities (Margherita/Pepperoni/Special), with payment success probability checks. The program outputs order lifecycle events and final statistics including revenue, service times, and cooling durations. Developed for AUEB's Operating Systems course.

Key Features
Order lifecycle: Payment processing → Pizza preparation → Parallel baking → Delivery

Resource management: Synchronized access to operators/cooks/ovens/drivers

Statistical output: Revenue per pizza type, service time analytics, failure rates

Configurable parameters: All constants defined in pizza.h

Thread-safe design: Mutex-protected statistics and output

Usage
Compile: gcc -pthread -o pizza pizza.c

Run: ./pizza <customer_count> <seed>

Test: Execute ./test-res.sh for automated test (100 customers)

Files: pizza.h (declarations), pizza.c (main logic), test-res.sh (test script), os_project.pdf (the description of the project in Greek documentation).
