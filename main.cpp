// g++ (GCC) 14.2.0

#include <iostream>
#include <random>
#include <numeric>
#include <vector>
#include <execution>
#include <thread>
#include <algorithm>
#include "measuring_time.h"

#define VEC_ITER vector<int>::iterator
#define UNARY_OP [](int x){return x * 2;} // can be any unary operation

using namespace std;

vector<int> generate_vector(const int& size){
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(INT_MIN, INT_MAX);

    vector<int> vec(size);

    for(int& it : vec){
        it = dis(gen);
    }

    return vec;
}

int parallel_transform_reduce(VEC_ITER begin, VEC_ITER end, int init, const function<int(int,int)>& binary_op, const function<int(int)>& unary_op, const int& K){
    const int length = (int)distance(begin, end);

    if (!distance(begin, end)){
        return init;
    }

    const int hardware_threads = (int)thread::hardware_concurrency();
    const int num_threads = min(hardware_threads != 0 ? hardware_threads : 2, K);
    const int block_size = length / num_threads;

    vector<int> result(num_threads, 0);
    vector<thread> threads(num_threads);

    for(int i = 0; i < num_threads; i++) {
        threads[i] = thread([=, &result]() {
            auto begin_block = begin + i * block_size;
            auto end_block = ((i == num_threads - 1) ? end : (begin_block + block_size));

            result[i] = transform_reduce(begin_block, end_block, 0, binary_op, unary_op);
        });
    }

    for(auto& t : threads){
        t.join();
    }

    return reduce(result.begin(), result.end(), init, binary_op);
}

int main(int argc, char* argv[]){
    vector<int> v = generate_vector(10e4); // vector size can be changed
    int result = 0;

    // (1) - no policy
    {
        cout << "No policy                   | Time: " <<
        measure_time([&](){
            result = transform_reduce(v.begin(), v.end(), 0, plus<>(),UNARY_OP);
        }) << " microseconds | Result = " << result << endl;
    };

    // (2) - with policy
    {
        cout << "Sequenced policy            | Time: " <<
        measure_time([&](){
            result = transform_reduce(execution::seq,v.begin(),v.end(),0,plus<>(),UNARY_OP);
        }) << " microseconds | Result = " << result << endl;

        cout << "Parallel policy             | Time: " <<
        measure_time([&](){
            result = transform_reduce(execution::par,v.begin(),v.end(),0,plus<>(),UNARY_OP);
        }) << " microseconds | Result = " << result << endl;

        cout << "Parallel unsequenced policy | Time: " <<
        measure_time([&](){
            result = transform_reduce(execution::par_unseq,v.begin(),v.end(),0,plus<>(),UNARY_OP);
        }) << " microseconds | Result = " << result << endl;

        cout << "Unsequenced policy          | Time: " <<
        measure_time([&](){
            result = transform_reduce(execution::unseq, v.begin(),v.end(),0,plus<>(),UNARY_OP);
        }) << " microseconds | Result = " << result << endl;
    }

    // (3) - own parallel algorithm
    {
        cout << "\nOwn parallel algorithm" << endl;

        pair<int, int> best_time_at_K = {INT_MAX, 0};

        for(int i = 1; i <= thread::hardware_concurrency(); i++){
            int time = (int)measure_time([&](){
                result = parallel_transform_reduce(v.begin(), v.end(), 0, plus<>(),UNARY_OP, i);});
            cout << "K = " << i << "  | Time: " << time << " microseconds | Result = " << result << endl;

            if (time < best_time_at_K.first){
                best_time_at_K = {time, i};
            }
        }

        cout << "Best time(" << best_time_at_K.first << " microseconds) at K = " << best_time_at_K.second << ". MAX_K = " << thread::hardware_concurrency() << endl;
    }

    return 0;
}
