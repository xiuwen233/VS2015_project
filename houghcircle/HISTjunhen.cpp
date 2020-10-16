 // 测试直方图均衡化

#include"THreadname.h"


void stretch(Mat* src, Mat* dst, int nMin, int nMax)
{
	//int low_value = nMin;    //拉伸后像素的最小值   
	//int high_value = nMax;   //拉伸后像素的最大值  

	//float rate = 0;          //图像的拉伸率  

	//float stretch_p[256], stretch_p1[256], stretch_num[256];
	////清空三个数组,初始化填充数组元素为0  
	//memset(stretch_p, 0, sizeof(stretch_p));
	//memset(stretch_p1, 0, sizeof(stretch_p1));
	//memset(stretch_num, 0, sizeof(stretch_num));
	////统计图像各个灰度级出现的次数  
	//uchar* srcData = (uchar*)src->imageData;
	//uchar* dstData = (uchar*)dst->imageData;
	//int nHeight = src->height;
	//int nWidth = src->width;
	//int i, j;
	//uchar nVal = 0;
	//for (i = 0; i<nHeight; i++)
	//{
	//	for (j = 0; j<nWidth; j++)
	//	{
	//		nVal = srcData[i*nWidth + j];
	//		stretch_num[nVal]++;
	//	}
	//}
	////统计各个灰度级出现的概率  
	//for (i = 0; i<256; i++)
	//{
	//	stretch_p[i] = stretch_num[i] / (nHeight*nWidth);
	//}
	////统计各个灰度级的概率和  
	//for (i = 0; i<256; i++)
	//{
	//	for (j = 0; j <= i; j++)
	//	{
	//		stretch_p1[i] += stretch_p[j];
	//	}
	//}
	////计算两个阈值点的值  
	//for (i = 0; i<256; i++)
	//{
	//	if (stretch_p1[i]<0.1)     //low_value取值接近于10%的总像素的灰度值  
	//	{
	//		low_value = i;
	//	}
	//	if (stretch_p1[i]>0.9)     //high_value取值接近于90%的总像素的灰度值  
	//	{
	//		high_value = i;
	//		break;
	//	}
	//}
	//rate = (float)255 / (high_value - low_value + 1);
	////进行灰度拉伸  
	//for (i = 0; i<nHeight; i++)
	//{
	//	for (j = 0; j<nWidth; j++)
	//	{
	//		nVal = srcData[i*nWidth + j];
	//		if (nVal<low_value)
	//		{
	//			dstData[i*nWidth + j] = 0;
	//		}
	//		else if (nVal>high_value)
	//		{
	//			dstData[i*nWidth + j] = 255;
	//		}
	//		else
	//		{
	//			dstData[i*nWidth + j] = (uchar)((nVal - low_value)*rate + 0.5);
	//			if (dstData[i*nWidth + j]>255)
	//			{
	//				dstData[i*nWidth + j] = 255;
	//			}
	//		}
	//	}
	//}
}


int main34()
{   
	String beaforeclasspath = "E://photo_opencv/GetBox/circle(";
	String afterclasspath = ").jpg";
	for (int i = 11; i <= 19;i++)
	{  
		String filename = beaforeclasspath + to_string(i) + afterclasspath;
		Mat image = imread(filename, 1);
		if (image.empty())
		{
			std::cout << "打开图片失败,请检查" << std::endl;
			break; 
		}
		//imshow("原图像", image);
		//---------------------------------------彩色图像的直方图均衡化实现：
	///*	Mat imageRGB[3];
	//	split(image, imageRGB);
	//	for (int i = 0; i < 3; i++)
	//	{
	//		equalizeHist(imageRGB[i], imageRGB[i]);
	//	}
	//	merge(imageRGB, 3, image);*/

		////-------------------------------------拉普拉斯算子可以增强局部的图像对比度：
		//Mat imageEnhance = image.clone();
		//Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
		//filter2D(image, imageEnhance, CV_8UC3, kernel);
		////imshow("拉普拉斯算子图像增强效果", imageEnhance);


		////-------------------------------------------------基于对数Log变换的图像增强
		//Mat imageLog(image.size(), CV_32FC3);
		//for (int i = 0; i < image.rows; i++)
		//{
		//	for (int j = 0; j < image.cols; j++)
		//	{
		//		imageLog.at<Vec3f>(i, j)[0] = log(1 + image.at<Vec3b>(i, j)[0]);
		//		imageLog.at<Vec3f>(i, j)[1] = log(1 + image.at<Vec3b>(i, j)[1]);
		//		imageLog.at<Vec3f>(i, j)[2] = log(1 + image.at<Vec3b>(i, j)[2]);
		//	}
		//}
		////归一化到0~255    
		//normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
		////转换成8bit图像显示    
		//convertScaleAbs(imageLog, imageLog);


		//--------------------------------------------------------------伽马变换的图像增强
		//Mat imageGamma(image.size(), CV_32FC3);
		//for (int i = 0; i < image.rows; i++)
		//{
		//	for (int j = 0; j < image.cols; j++)
		//	{
		//		imageGamma.at<Vec3f>(i, j)[0] = (image.at<Vec3b>(i, j)[0])*(image.at<Vec3b>(i, j)[0])*(image.at<Vec3b>(i, j)[0]);
		//		imageGamma.at<Vec3f>(i, j)[1] = (image.at<Vec3b>(i, j)[1])*(image.at<Vec3b>(i, j)[1])*(image.at<Vec3b>(i, j)[1]);
		//		imageGamma.at<Vec3f>(i, j)[2] = (image.at<Vec3b>(i, j)[2])*(image.at<Vec3b>(i, j)[2])*(image.at<Vec3b>(i, j)[2]);
		//	}
		//}
		////归一化到0~255    
		//normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
		////转换成8bit图像显示    
		//convertScaleAbs(imageGamma, imageGamma);


		//------------------------------------单直方图均衡化
		Mat gray;
		cvtColor(image, gray, CV_RGB2GRAY);
		Mat result;
		equalizeHist(gray, result);
		String filename_w = "单只方图_" + to_string(i) + ".jpg";
		imwrite(filename_w, result);
	}
	getchar();
	return 0;
}


void main3()
{
	Mat imme = imread("E://photo_opencv/GetBox/circle(11).jpg");
	Mat image;
	stretch(&imme, &image,0,255);
	HoughCircles
}



