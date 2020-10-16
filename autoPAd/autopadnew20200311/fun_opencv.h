#pragma once

#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>

using namespace cv;

void ContrastAndBright(int, void *);
void GetBrightness(Mat InputImg, float& cast, float& f_dark_quantity);
//void InitControlBoard();
void  make_image(int circle_r, Mat& temple);
Point MatchCircleTemplate(Mat bitwise_out, Mat templ, int circle_d);