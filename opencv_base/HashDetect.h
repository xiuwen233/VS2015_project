#pragma once
#include"opecvinclude.h"

//这是用来比较两副图的相似性的 图片输入要是源码

Mat calPHashCode(Mat image);
Mat calHashCode(Mat image);
int calHammingDistance(Mat modelHashCode, Mat testHashCode);

//峰值信噪比   用来比较放大和缩小之后的差距 一般意义不大
double getPSNR(const Mat& I1, const Mat& I2);