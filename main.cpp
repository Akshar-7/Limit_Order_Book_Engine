#include <iostream>

#include "benchmark.h"

int main(){
std::cout << "Starting benchmark...\n\n" << std::endl;

    int num = 10;
    for(int i=1; i<=7; i++, num*=10){
        run_benchmark(num);
    }

    std::cout << "Benchmark complete.\n" << std::endl;
    return 0;
}