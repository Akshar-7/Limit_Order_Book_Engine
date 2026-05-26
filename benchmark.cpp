#include "memory_pool.h"
#include "MPSC_ring_buffer.h"
#include "limit_order_book.h"

#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <iomanip>

using namespace std;

using OrderBuffer = MPSC_ring_buffer<Order*, 1<<20>; 

mt19937 rng(1337);

pair<int,int> price = {10000, 10050};
pair<int,int> quantity = {10, 1000};
pair<int,int> side = {0, 1};
pair<int,int> type = {1, 100};

uniform_int_distribution<uint32_t> price_dist(price.first, price.second);
uniform_int_distribution<uint32_t> quantity_dist(quantity.first, quantity.second);
uniform_int_distribution<uint32_t> side_dist(side.first, side.second);
uniform_int_distribution<uint32_t> type_dist(type.first, type.second);

void producer_logic(int total_orders, OrderBuffer& buffer, MemoryPool<Order>& pool){
    for(int i=0; i<total_orders; i++){
        Order* order = pool.remove();
        order->id = i;
        order->price = price_dist(rng);
        order->quantity = quantity_dist(rng);
        order->side = static_cast<OrderSide> (side_dist(rng));
        order->type = static_cast<OrderType> (type_dist(rng) > 90);

        buffer.push(order);
    }
}

void consumer_logic(int total_orders, OrderBuffer& buffer, LOB& lob){
    for(int i=0; i < total_orders; i++){
        Order* order = buffer.pop();
        lob.process(order);
    }
}

void run_benchmark(const int total_orders){
    cout << "--- Booting Benchmark for " << total_orders << " Orders ---\n";

    auto pool_A = new MemoryPool<Order>(total_orders/2);
    auto pool_B = new MemoryPool<Order>(total_orders/2);
    auto lob = new LOB();
    auto buffer = new OrderBuffer();

    auto start_time = chrono::high_resolution_clock::now();

    thread producer_A(producer_logic, total_orders/2, std::ref(*buffer), std::ref(*pool_A));
    thread producer_B(producer_logic, total_orders/2, std::ref(*buffer), std::ref(*pool_B));

    consumer_logic(total_orders, *buffer, *lob);

    producer_A.join();
    producer_B.join();

    // Clean Up
    delete buffer;
    delete lob;
    delete pool_A;
    delete pool_B;

    auto end_time = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> elapsed_ms = end_time - start_time;
    double total_ms = elapsed_ms.count();
    double total_seconds = total_ms / 1000.0;
    double throughput = total_orders / total_seconds;

    cout << fixed << setprecision(3);
    
    cout << "Execution Time: " << total_ms << " ms\n";
    cout << "Throughput:     " << throughput << " orders per second\n\n";
}