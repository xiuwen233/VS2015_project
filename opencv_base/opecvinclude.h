#pragma once
#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>

using namespace cv;

struct Myline {//ÿ����
	Point start;//���
	Point end;//�յ�
	int linelen;  // �ߵĳ���
};

typedef struct GetPointRect
{
	int rect_x;
	int rect_y;
	int rect_weight;
	int rect_hight;
}GetPointRect;
