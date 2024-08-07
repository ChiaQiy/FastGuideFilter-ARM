#include <iostream>
#include <chrono>
#include <unistd.h>

#include "fastguidefilter.hpp"


using namespace std::chrono;

//test on imx6ull, Cortex-A7
//32KB I/D L1 cache, 128KB L2 cache
int main(int argc, char** argv)
{
    std::cout << "------------------------hello------------------------" << std::endl;


    const int height = 480;
    const int width = 640;
    FastGuideFilter fgFilter(height, width);

    float* data_src = new float[width*height];
    float* data_dst = new float[width*height];
    for (int i=0; i<width*height; i++) {
        data_src[i] = 1.0;
        data_dst[i] = 0.0;
    }

    auto start = std::chrono::steady_clock::now();

    // fgFilter.filter_0(data_src, data_dst, 1);      //226207us
    // fgFilter.filter_1(data_src, data_dst, 1);      //152601us
    // fgFilter.filter_2(data_src, data_dst, 1);      //74233us
    // fgFilter.filter_3(data_src, data_dst, 1);      //51874us
    fgFilter.resizeDown4x_0(data_src, data_dst);    //1422us

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function elapsed: " << duration.count() << "us\n";

    std::cout << std::endl;
    for (int i=0; i<5; i++) {
        for (int j=0; j<10; j++) {
            std::cout << data_dst[i * width + j];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    

    return 0;
}