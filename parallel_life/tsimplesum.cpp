#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <random>
#include <chrono>
using namespace std;

#define SIZE 1e8
#define MAX_THREAD 20

// #define double int

std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(0.0,1.0);


void work(int l, int r, double* sum, vector <double>* data, shared_ptr<mutex> mtp) {
    double part = 0;
    for (int i = l; i< r; ++i) {
        part += (*data)[i];
    }

    lock_guard<mutex> g(*mtp);
//    cout<<std::this_thread::get_id() <<endl;
    *sum += part;
}


double getThreadSum(vector <double>& data, int numbThr) {
   // shared_ptr<mutex> ptrMutex(new mutex);
    shared_ptr<mutex> mtp(new mutex);
    vector <thread> vt;
    int size = data.size();
    int d = size/numbThr;
    double ans = 0;
    for(int i = 0;i < numbThr; ++i) {
        vt.push_back(thread(work, i * d, (i == numbThr - 1 ? size : (i+1)*d) , &ans, &data, mtp));
    }


    for(auto& t : vt) {
        t.join();
    }
    return ans;
}

int main() {
    vector <double> data(SIZE);
    double realSum = 0;
    for(int i =  0;i < SIZE; ++i) {
        data[i] = distribution(generator);
    }

    for(int i = 0; i < SIZE; ++i) {
        realSum += data[i];
    }

    cout<<fixed;
    cout.precision(10);
    cout << "realSum: " << realSum << endl;

    for(int i = 1; i <= MAX_THREAD; ++i) {

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        double threadSum = getThreadSum(data, i);
        end = std::chrono::system_clock::now();
        int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>
                                 (end-start).count();
       
        cout << "threads: " << i << " time: " << elapsed_milliseconds << " sum: " << threadSum << " delta: " << abs(threadSum - realSum) << endl;
    cout << thread::hardware_concurrency() << endl;
}

//-std=c++11
//g++ -std=c++11 tsimplesum.cpp -o tsimplesum

