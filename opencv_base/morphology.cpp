#include"morphology.h"

void morphology(Mat img)
{
	/*
	* MORPH_GRADIENT 形态学梯度运算 就是膨胀图与俯视图之差，用于保留物体的边缘轮廓。
	* MORPH_OPEN开运算 先腐蚀再膨胀
	* MORPH_CLOSE闭运算 先膨胀再腐蚀
	* MORPH_TOPHAT顶帽 原图像与开运算图之差，用于分离比邻近点亮一些的斑块。
	* MORPH_BLACKHAT黑帽 闭运算与原图像之差，用于分离比邻近点暗一些的斑块。
	* MORPH_ERODE 腐蚀
	* MORPH_DILATE 膨胀
	*/
	Mat out;
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	//矩形: MORPH_RECT
	//交叉形 : MORPH_CROSS
	//椭圆形 : MORPH_ELLIPSE
	morphologyEx(img, out, MORPH_GRADIENT, element);
	imwrite("GRADIENT.jpg", out);
	morphologyEx(img, out, MORPH_CLOSE, element);
	imwrite("CLOSE.jpg", out);
	morphologyEx(img, out, MORPH_TOPHAT, element);
	imwrite("TOPHAT.jpg", out);
	morphologyEx(img, out, MORPH_BLACKHAT, element);
	imwrite("BLACKHAT.jpg", out);
	morphologyEx(img, out, MORPH_ERODE, element);
	imwrite("ERODE.jpg", out);
	morphologyEx(img, out, MORPH_DILATE, element);
	imwrite("DILATE.jpg", out);
}