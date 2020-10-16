#pragma once
#include"opecvinclude.h"

#include<iostream>
using namespace std;
/*
* 这个是统计直方图显示的 图片统计
*可以用于直方图统计·和比较还有什么均衡化之类的
*/

void HistTest(Mat image);
void Getkill(Mat img);
void JustKill(Mat img);
int  MyTEST(Mat img2);

// compareHist(hist_image1, hist_iamge2, CV_COMP_CHISQR); 用于比较直方图统计的
MatND GetMatNDfill(Mat img); 