 // ����ֱ��ͼ���⻯

#include"THreadname.h"


void stretch(Mat* src, Mat* dst, int nMin, int nMax)
{
	//int low_value = nMin;    //��������ص���Сֵ   
	//int high_value = nMax;   //��������ص����ֵ  

	//float rate = 0;          //ͼ���������  

	//float stretch_p[256], stretch_p1[256], stretch_num[256];
	////�����������,��ʼ���������Ԫ��Ϊ0  
	//memset(stretch_p, 0, sizeof(stretch_p));
	//memset(stretch_p1, 0, sizeof(stretch_p1));
	//memset(stretch_num, 0, sizeof(stretch_num));
	////ͳ��ͼ������Ҷȼ����ֵĴ���  
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
	////ͳ�Ƹ����Ҷȼ����ֵĸ���  
	//for (i = 0; i<256; i++)
	//{
	//	stretch_p[i] = stretch_num[i] / (nHeight*nWidth);
	//}
	////ͳ�Ƹ����Ҷȼ��ĸ��ʺ�  
	//for (i = 0; i<256; i++)
	//{
	//	for (j = 0; j <= i; j++)
	//	{
	//		stretch_p1[i] += stretch_p[j];
	//	}
	//}
	////����������ֵ���ֵ  
	//for (i = 0; i<256; i++)
	//{
	//	if (stretch_p1[i]<0.1)     //low_valueȡֵ�ӽ���10%�������صĻҶ�ֵ  
	//	{
	//		low_value = i;
	//	}
	//	if (stretch_p1[i]>0.9)     //high_valueȡֵ�ӽ���90%�������صĻҶ�ֵ  
	//	{
	//		high_value = i;
	//		break;
	//	}
	//}
	//rate = (float)255 / (high_value - low_value + 1);
	////���лҶ�����  
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
			std::cout << "��ͼƬʧ��,����" << std::endl;
			break; 
		}
		//imshow("ԭͼ��", image);
		//---------------------------------------��ɫͼ���ֱ��ͼ���⻯ʵ�֣�
	///*	Mat imageRGB[3];
	//	split(image, imageRGB);
	//	for (int i = 0; i < 3; i++)
	//	{
	//		equalizeHist(imageRGB[i], imageRGB[i]);
	//	}
	//	merge(imageRGB, 3, image);*/

		////-------------------------------------������˹���ӿ�����ǿ�ֲ���ͼ��Աȶȣ�
		//Mat imageEnhance = image.clone();
		//Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
		//filter2D(image, imageEnhance, CV_8UC3, kernel);
		////imshow("������˹����ͼ����ǿЧ��", imageEnhance);


		////-------------------------------------------------���ڶ���Log�任��ͼ����ǿ
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
		////��һ����0~255    
		//normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
		////ת����8bitͼ����ʾ    
		//convertScaleAbs(imageLog, imageLog);


		//--------------------------------------------------------------٤��任��ͼ����ǿ
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
		////��һ����0~255    
		//normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
		////ת����8bitͼ����ʾ    
		//convertScaleAbs(imageGamma, imageGamma);


		//------------------------------------��ֱ��ͼ���⻯
		Mat gray;
		cvtColor(image, gray, CV_RGB2GRAY);
		Mat result;
		equalizeHist(gray, result);
		String filename_w = "��ֻ��ͼ_" + to_string(i) + ".jpg";
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



