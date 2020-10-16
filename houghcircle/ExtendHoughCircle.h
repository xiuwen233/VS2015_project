#pragma once

#include <cv.h>
#include <windows.h>

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;

struct circle_found
{
	float score;	//ƥ�����
	cv::Vec3f circle;	//�ҵ���Բ
};

namespace ExtendCV
{
	//_image���������ͼ�񣬱���Ϊ8λ��ͨ����_circles�����ҵ���Բ��dp����cv::houghcircles�е�dp��min_dist����cv::houghcircles�е�minDist��Բ��С����
	//low_threshold������_imageԤ������ȡ������canny����ֵ��high_threshold������_imageԤ������ȡ������canny����ֵ
	//acc_threshold����cv::houghcircles�е�param2�ۼ���ֵ��minRadius����Բ����С�뾶��maxRadius����Բ�����뾶
	//minScore�����ҳ���Բ�����е��������غ��ʣ���Ϊ����
	//_contour_image������ѡ����������ͼ���������ǿգ���low_threshold��high_threshold���ԣ���������ͼ�Ľ�һ��Ԥ����
	void FindCircles(cv::InputArray _image, vector<circle_found>& _circles, float dp, int min_dist,
		int low_threshold, int high_threshold, int acc_threshold, int minRadius, int maxRadius,
		float minScore, cv::InputArray _contour_image = cv::Mat());

}