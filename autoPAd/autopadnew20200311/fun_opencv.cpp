#include"fun_opencv.h"



void GetBrightness(Mat InputImg, float& cast, float& f_dark_quantity)
{
	Mat GRAYimg = InputImg;
	float a = 0;
	int Hist[256];
	for (int i = 0; i<256; i++)
		Hist[i] = 0;
	for (int i = 0; i<GRAYimg.rows; i++)
	{
		for (int j = 0; j<GRAYimg.cols; j++)
		{
			a += float(GRAYimg.at<uchar>(i, j) - 128);//在计算过程中，考虑128为亮度均值点  
			int x = GRAYimg.at<uchar>(i, j);
			Hist[x]++;
		}
	}
	f_dark_quantity = a / float(GRAYimg.rows*InputImg.cols);
	float D = abs(f_dark_quantity);
	float Ma = 0;
	for (int i = 0; i<256; i++)
	{
		Ma += abs(i - 128 - f_dark_quantity)*Hist[i];
	}
	Ma /= float((GRAYimg.rows*GRAYimg.cols));
	float M = abs(Ma);
	float K = D / M;
	cast = K;
	return;
}


Point MatchCircleTemplate(Mat bitwise_out, Mat templ, int circle_d)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);
	matchTemplate(bitwise_out, templ, g_resultImage, 4);
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;
	minMaxLoc(g_resultImage, &minValue, &maxValue, &minLocation, &maxLocation);
	if (4 == TM_SQDIFF || 4 == CV_TM_SQDIFF_NORMED)
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}

	Point p_re;
	p_re.x = matchLocation.x + circle_d / 2;
	p_re.y = matchLocation.y + circle_d / 2;
	return p_re;
}


 void ContrastAndBright(int, void *)
{
	Mat m = Mat(600, 600, 0);
	imshow("【控制面板】", m);
}


void  make_image(int circle_r, Mat& temple)
{
	Mat mask = Mat(Size(circle_r + 2, circle_r + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(circle_r / 2 + 1, circle_r / 2 + 1), circle_r / 2, Scalar(255, 255, 255), 1);
	temple = mask.clone();
}
