#pragma once
#include"opecvinclude.h"

//���������Ƚ�����ͼ�������Ե� ͼƬ����Ҫ��Դ��

Mat calPHashCode(Mat image);
Mat calHashCode(Mat image);
int calHammingDistance(Mat modelHashCode, Mat testHashCode);

//��ֵ�����   �����ȽϷŴ����С֮��Ĳ�� һ�����岻��
double getPSNR(const Mat& I1, const Mat& I2);