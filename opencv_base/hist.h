#pragma once
#include"opecvinclude.h"

#include<iostream>
using namespace std;
/*
* �����ͳ��ֱ��ͼ��ʾ�� ͼƬͳ��
*��������ֱ��ͼͳ�ơ��ͱȽϻ���ʲô���⻯֮���
*/

void HistTest(Mat image);
void Getkill(Mat img);
void JustKill(Mat img);
int  MyTEST(Mat img2);

// compareHist(hist_image1, hist_iamge2, CV_COMP_CHISQR); ���ڱȽ�ֱ��ͼͳ�Ƶ�
MatND GetMatNDfill(Mat img); 