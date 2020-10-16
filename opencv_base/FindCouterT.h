#pragma once
#include"opecvinclude.h"
#include<iostream>

using namespace std;
// 比较适用的方法
Mat shihe_drawcouter(Mat src);

Point  corner_point[4];

GetPointRect GetTheRectRangle();

float  Findcouter_center(Mat img, Point * point);
void  Findcouter(Mat img);
Mat whatTHis(Mat img, Mat src, Point * center);
Point Findcouterthis(Mat img, float &angle);
Point Findcouter_foroutside(Mat img, float &angle);
float Findcouter_inside_small(Mat img);
Point Findcouter_Positive(Mat img, float * angle);
Mat  Findcouter_three(Mat img, Mat src, float * angle, Point * Center);
int  Findcouter_TWO_inside(Mat img);
Point Findcouter_Slash(Mat img, float * angle);