#ifndef __FAST_GUIDE_FILTER_H__
#define __FAST_GUIDE_FILTER_H__

#include <vector>

class BoxFilter {
public:
    BoxFilter(const int height, const int width) : height(height), width(width){
        cache.resize(height * width);
        colSum.resize(width);
    };
    ~BoxFilter(){
        std::vector<float>().swap(cache);
        std::vector<float>().swap(colSum);
    };

    void filter_0(float* src, float* dst, int r);
    void filter_1(float *src, float *dst, int r);
    void filter_2(float *src, float *dst, int r);
    void filter_3(float *src, float *dst, int r);

private:
    int height;
    int width;
    std::vector<float> cache;
    std::vector<float> colSum;
};

#endif