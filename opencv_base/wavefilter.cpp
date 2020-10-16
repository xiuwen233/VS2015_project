#include"wavefilter.h"

void wavefilter(Mat img)
{
	Mat out;
	boxFilter(img, img, -1, Size(5, 5));  //方框滤波
	blur(img, img, Size(5, 5)); //均值滤波
	GaussianBlur(img, out, Size(3, 3), 0, 0); //高斯滤波

	medianBlur(img, out, 7);//中值滤波 第三个参数表示孔径的线性尺寸，它的值必须是大于1的奇数 非线性
	bilateralFilter(img, out, 25, 25 * 2, 25 / 2);  //双边滤波
}