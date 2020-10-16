#pragma once
#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>

using namespace cv;

struct Myline {//每条线
	Point start;//起点
	Point end;//终点
	int linelen;  // 线的长度
};

typedef struct GetPointRect
{
	int rect_x;
	int rect_y;
	int rect_weight;
	int rect_hight;
}GetPointRect;
