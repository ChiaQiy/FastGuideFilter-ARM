#include <iostream>
#include <arm_neon.h>
#include <unistd.h>

#include "fastguidefilter.hpp"


void FastGuideFilter::filter_0(float* src, float* dst, int r) {
    for (int h=0; h<height; h++) {
        for (int w=0; w<width; w++) {
            //border: 00000
            int start_h = std::max(0, h - r);
            int end_h = std::min(height - 1, h + r);
            int start_w = std::max(0, w - r);
            int end_w = std::min(width - 1, w + r);

            float tmp = 0;
            for (int sh=start_h; sh<=end_h; sh++) {
                for (int sw=start_w; sw<=end_w; sw++) {
                    tmp += src[sh * width + sw];
                }
            }
            dst[h * width + w] = tmp;
        }
    }

}

void FastGuideFilter::filter_1(float* src, float* dst, int r) {
    //horizon
    float* cachePtr = &(cache[0]);
    for (int h = 0; h < height; h++) {
        int shift = h * width;
        for (int w = 0; w < width; w++) {
            int start_w = std::max(0, w - r);
            int end_w = std::min(width - 1, w + r);

            float tmp = 0;
            for (int sw = start_w; sw <= end_w; sw++) {
                tmp += src[shift + sw];
            }

            cachePtr[shift + w] = tmp;
            dst[shift + w] = 0;
        }
    }
    //vertical
    for (int h = 0; h < height; h++) {
        int start_h = std::max(0, h - r);
        int end_h = std::min(height - 1, h + r);
        
        for (int sh = start_h; sh <= end_h; sh++) {
            int shift = sh * width;
            for (int w = 0; w < width; w++) {
                dst[shift + w] += cachePtr[shift + w];
            }
        }
    }
}

void FastGuideFilter::filter_2(float* src, float* dst, int r) {
    //horizon
    float* cachePtr = &(cache[0]);
    for (int h = 0; h < height; h++) {
        int shift = h * width;
        float tmp = 0;

        //1st half
        for (int w = 0; w < r; w++) {
            tmp += src[shift + w];
        }
        for (int w = 0; w <= r; w++) {
            tmp += src[shift + w + r];
            cachePtr[shift + w] = tmp;
        }
        //middle
        int start_w = r + 1;
        int end_w = width - r - 1;
        for (int w = start_w; w <= end_w; w++) {
            tmp += src[shift + w + r];
            tmp -= src[shift + w - r - 1];
            cachePtr[shift + w] = tmp;            
        }
        //last half
        start_w = width - r;
        for (int w = start_w; w < width; w++) {
            tmp -= src[shift + w - r - 1];
            cachePtr[shift + w] = tmp;
        }
    }

    float* colSumPtr = &(colSum[0]);
    for (int i = 0; i < width; i++) {
        colSumPtr[i] = 0;
    }

    //vertical
    for (int h = 0; h < r; h++) {
        for (int w = 0; w < width; w++) {
            colSumPtr[w] += cachePtr[h * width + w];
        }    
    }
    //1st half
    for (int h = 0; h <= r; h++) {
        int shift = h * width;
        for (int w = 0; w < width; w++) {
            colSumPtr[w] += cachePtr[shift + (r * width) + w];
            dst[shift + w] = colSumPtr[w];
        }
    }
    //middle
    int start_h = r + 1;
    int end_h = height - r - 1;
    for (int h = start_h; h <= end_h; h++) {
        int shift = h * width;
        for (int w = 0; w < width; w++) {
            colSumPtr[w] += cachePtr[shift + (r * width) + w];
            colSumPtr[w] -= cachePtr[shift - ((r + 1) * width) + w];
            dst[shift + w] = colSumPtr[w];
        }
    }
    //last half
    start_h = height - r;
    for (int h = start_h; h < height; h++) {
        int shift = h * width;
        for (int w = 0; w < width; w++) {
            colSumPtr[w] -= cachePtr[shift - ((r + 1) * width) + w];
            dst[shift + w] = colSumPtr[w];
        }
    }
}

void FastGuideFilter::filter_3(float* src, float* dst, int r) {
    //horizon
    float* cachePtr = &(cache[0]);
    for (int h = 0; h < height; h++) {
        int shift = h * width;
        float tmp = 0;

        //1st half
        for (int w = 0; w < r; w++) {
            tmp += src[shift + w];
        }
        for (int w = 0; w <= r; w++) {
            tmp += src[shift + w + r];
            cachePtr[shift + w] = tmp;
        }
        //middle
        int start_w = r + 1;
        int end_w = width - r - 1;
        for (int w = start_w; w <= end_w; w++) {
            tmp += src[shift + w + r];
            tmp -= src[shift + w - r - 1];
            cachePtr[shift + w] = tmp;            
        }
        //last half
        start_w = width - r;
        for (int w = start_w; w < width; w++) {
            tmp -= src[shift + w - r - 1];
            cachePtr[shift + w] = tmp;
        }
    }

    float* colSumPtr = &(colSum[0]);
    for (int i = 0; i < width; i++) {
        colSumPtr[i] = 0;
    }

    //vertical
    for (int h = 0; h < r; h++) {
        for (int w = 0; w < width; w++) {
            colSumPtr[w] += cachePtr[h * width + w];
        }    
    }
    //1st half
    for (int h = 0; h <= r; h++) {
        int shift = h * width;
        for (int w = 0; w < width; w++) {
            colSumPtr[w] += cachePtr[shift + (r * width) + w];
            dst[shift + w] = colSumPtr[w];
        }
    }
    //middle
    int start_h = r + 1;
    int end_h = height - r - 1;
    for (int h = start_h; h <= end_h; h++) {
        int shift = h * width;
        //arm neon optimize
        int n = width >> 2;
        int remain = width - (n << 2);
        float* addPtr = cachePtr + shift + (r * width);
        float* subPtr = cachePtr + shift - ((r + 1) * width);
        float* tmpColSumPtr = colSumPtr;
        float* tmpDstPtr = dst + shift;
        for (; n > 0; n--) {
            float32x4_t _add = vld1q_f32(addPtr);
            float32x4_t _sub = vld1q_f32(subPtr);
            float32x4_t _colSum = vld1q_f32(tmpColSumPtr);

            float32x4_t _tmp = vaddq_f32(_colSum, _add);
            _tmp = vsubq_f32(_tmp, _sub);

            vst1q_f32(tmpColSumPtr, _tmp);
            vst1q_f32(tmpDstPtr, _tmp);

            addPtr += 4;
            subPtr += 4;
            tmpColSumPtr += 4;
            tmpDstPtr += 4;
        }
        for (; remain > 0; remain--) {
            *tmpColSumPtr += *addPtr;
            *tmpColSumPtr -= *subPtr;
            *tmpDstPtr = *tmpColSumPtr;
            addPtr++;
            subPtr++;
            tmpColSumPtr++;
            tmpDstPtr++;
        }
    }
    //last half
    start_h = height - r;
    for (int h = start_h; h < height; h++) {
        int shift = h * width;
        for (int w = 0; w < width; w++) {
            colSumPtr[w] -= cachePtr[shift - ((r + 1) * width) + w];
            dst[shift + w] = colSumPtr[w];
        }
    }
}

void FastGuideFilter::resizeDown4x_0(float *src, float *dst)
{
    //interp: nearest
    const float* ptr = src;
    float* outptr = dst;
    int out_height = height >> 2;
    int out_width = width >> 2;
    for (int h = 0; h < out_height; h++) {
        for (int w = 0; w < out_width; w++) {
            outptr[h * out_width + w] = ptr[(h << 2) * width + (w << 2)];
        }
    }
}

void FastGuideFilter::resizeUp4x_0(float *src, float *dst)
{
    //interp: linear

}