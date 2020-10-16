#pragma once
#include"opecvinclude.h"
#include<iostream>

using namespace std;

//霍夫检测直线
Mat HoufuLineP(Mat src);

//计算两条直线的夹角
Point getCross(Myline line1, Myline line2);