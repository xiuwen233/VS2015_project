#include"Equalize.h"

void Testapk(Mat img)
{
	Mat dest;
	equalizeHist(img, dest);
	imwrite("junheihua.jpg", dest);
}

void RGBtest(Mat img)
{
	Mat imageRGB[3];
	Mat image = img;
	split(image, imageRGB);
	for (int i = 0; i < 3; i++)
	{
		equalizeHist(imageRGB[i], imageRGB[i]);
	}
	merge(imageRGB, 3, image);
	imwrite("Rgb.jpg", image);
}

// ������˹���ӿ�����ǿ�ֲ���ͼ��Աȶȣ�
void LapuLas(Mat img)
{
	Mat imageEnhance;
	Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
	filter2D(img, imageEnhance, CV_8UC3, kernel);
	imwrite("LapuLas.jpg", imageEnhance);
}


// �����任���Խ�ͼ��ĵͻҶ�ֵ������չ����ʾ���ͻҶȲ��ָ����ϸ�ڣ�����߻Ҷ�ֵ����ѹ�������ٸ߻Ҷ�ֵ���ֵ�ϸ�ڣ��Ӷ��ﵽǿ��ͼ��ͻҶȲ��ֵ�Ŀ�ġ�
void LogTurn(Mat img)
{
	Mat image = img.clone();
	Mat imageLog(image.size(), CV_32FC3);
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			imageLog.at<Vec3f>(i, j)[0] = log(1 + image.at<Vec3b>(i, j)[0]);
			imageLog.at<Vec3f>(i, j)[1] = log(1 + image.at<Vec3b>(i, j)[1]);
			imageLog.at<Vec3f>(i, j)[2] = log(1 + image.at<Vec3b>(i, j)[2]);
		}
	}
	//��һ����0~255    
	normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
	//ת����8bitͼ����ʾ    
	convertScaleAbs(imageLog, imageLog);
	imwrite("Log.jpg", imageLog);
}


//٤��任����ͼ��Աȶ�ƫ�ͣ�������������ֵƫ�ߣ�������������أ�����µ�ͼ����ǿЧ�����ԡ�
void Gammaimg(Mat img)
{
	Mat image = img.clone();
	Mat imageGamma(image.size(), CV_32FC3);
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			imageGamma.at<Vec3f>(i, j)[0] = (image.at<Vec3b>(i, j)[0])*(image.at<Vec3b>(i, j)[0])*(image.at<Vec3b>(i, j)[0]);
			imageGamma.at<Vec3f>(i, j)[1] = (image.at<Vec3b>(i, j)[1])*(image.at<Vec3b>(i, j)[1])*(image.at<Vec3b>(i, j)[1]);
			imageGamma.at<Vec3f>(i, j)[2] = (image.at<Vec3b>(i, j)[2])*(image.at<Vec3b>(i, j)[2])*(image.at<Vec3b>(i, j)[2]);
		}
	}
	//��һ����0~255    
	normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
	//ת����8bitͼ����ʾ    
	convertScaleAbs(imageGamma, imageGamma);
	imwrite("Gammaimg.jpg", imageGamma);
}