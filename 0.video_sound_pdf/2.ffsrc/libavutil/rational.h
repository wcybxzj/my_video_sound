#ifndef RATIONAL_H
#define RATIONAL_H

//两整数精确表示分数。
//常规的可以用一个float 或double 型数来表示分数，
//但不是精确表示，在需要相对比较精确计算的时候，为避免非精确表示带来的计算误差，采用两整数来精确表示
typedef struct AVRational
{
    int num; // numerator   // 分子
    int den; // denominator // 分母
} AVRational;

static inline double av_q2d(AVRational a)
{
    return a.num / (double)a.den;
}

#endif
