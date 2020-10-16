#include"hist.h"


//直方图进行滤波
//for (int k = 0; k < 15; k++)
//{
//	realValue[0] = (realValue[0] + realValue[0] + realValue[1]) / 3;
//
//	for (int i = 1; i < 255; i++)
//	{
//		realValue_tmp1 = realValue[i - 1];
//		realValue_tmp2 = realValue[i];
//		realValue_tmp3 = realValue[i + 1];
//		realValue[i] = (realValue_tmp1 + realValue_tmp2 + realValue_tmp3) / 3;
//	}
//	realValue[255] = (realValue[254] + realValue[255] + realValue[255]) / 3;
//}


void HistTest(Mat image)
{
	MatND hist;
	int bins = 256;
	int hist_size[] = { bins };
	float range[] = { 0, 256 };
	const float* ranges[] = { range };
	int channels[] = { 0 };
	calcHist(&image, 1, channels, Mat(), // do not use mask      
		hist, 1, hist_size, ranges,
		true, // the histogram is uniform      
		false);

	//计算出出现的频率  
	int sum = image.cols*image.rows;
	float p[256];
	for (int i = 0; i<256; i++) {
		p[i] = hist.at<float>(i) / (1.0*sum);
		//cout<<p[i]<<" ";    
	}
	double max_val;  //直方图的最大值   
	minMaxLoc(hist, 0, &max_val, 0, 0); //计算直方图最大值  
										//画出直方图  
	int maxheight = 256;
	Mat image2 = Mat::zeros(256, 2 * 256, CV_8UC3);
	for (int i = 0; i<256; i++) {
		//      计算高度  
		double height = (maxheight*hist.at<float>(i)) / (1.0*max_val);
		//画出对应的高度图  
		//坐标体系中的零点坐标定义为图片的左上角，X轴为图像矩形的上面那条水平线，从左往右；Y轴为图像矩形左边的那条垂直线，从上往下。在Point(x,y)和Rect(x,y)中，
		//第一个参数x代表的是元素所在图像的列数，第二个参数y代表的是元素所在图像的行数，而在at(x,y)中是相反的。  
		rectangle(image2, Point(i * 2, 255),
			Point((i + 1) * 2 - 1, 255 - height),
			CV_RGB(255, 255, 255));
	}
	imwrite("help.jpg", image2);
	//cout<<endl;  
}

MatND GetMatNDfill(Mat img)
{
	MatND hist;       // 在cv中用CvHistogram *hist = cvCreateHist  
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };   //这里需要为const类型  
	int size = 256;
	int channels = 0;
	//计算图像的直方图  
	calcHist(&img, 1, &channels, Mat(), hist, dims, &size, ranges);    //cv中是cvCalcHist  
	int hpt = saturate_cast<int>(0.9 * size);
	double minVal = 0;
	double maxVal = 0;
	int scale = 1;
	Mat imageShow(size * scale, size, CV_8U, Scalar(0));

	minMaxLoc(hist, &minVal, &maxVal, 0, 0);    //cv中用的是cvGetMinMaxHistValue  
	for (int i = 0; i < 256; i++)
	{
		float value = hist.at<float>(i);           // 注意hist中是float类型    cv中用cvQueryHistValue_1D  
		int realValue = saturate_cast<int>(value * hpt / maxVal);
		rectangle(imageShow, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255));
	}

	//imshow("looksee.jpg", imageShow);
	return hist;
}

void Getkill(Mat img)
{
	MatND hist;       // 在cv中用CvHistogram *hist = cvCreateHist  
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };   //这里需要为const类型  
	int size = 256;
	int channels = 0;
	//计算图像的直方图  
	calcHist(&img, 1, &channels, Mat(), hist, dims, &size, ranges);    //cv中是cvCalcHist  
	int hpt = saturate_cast<int>(0.9 * size);
	double minVal = 0;
	double maxVal = 0;
	int scale = 1;
	Mat imageShow(size * scale, size, CV_8U, Scalar(0));

	minMaxLoc(hist, &minVal, &maxVal, 0, 0);    //cv中用的是cvGetMinMaxHistValue  
	for (int i = 0; i < 256; i++)
	{
		float value = hist.at<float>(i);           // 注意hist中是float类型    cv中用cvQueryHistValue_1D  
		int realValue = saturate_cast<int>(value * hpt / maxVal);
		rectangle(imageShow, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255));
	}

	//imshow(WINDOWS_NAMES, imageShow);
	imshow("looksee.jpg", imageShow);
}


void JustKill(Mat img)
{
	MatND hist[3];       // 在cv中用CvHistogram *hist = cvCreateHist  
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };   //这里需要为const类型  
	int size = 256;
	int channels = 0;
	//计算图像的直方图  
	vector<Mat> Channels;
	split(img, Channels);

	imwrite("Rn.jpg", Channels[0]);
	imwrite("Gn.jpg", Channels[1]);
	imwrite("Bn.jpg", Channels[2]);
	calcHist(&Channels[0], 1, &channels, Mat(), hist[0], dims, &size, ranges);    //cv中是cvCalcHist  
	calcHist(&Channels[1], 1, &channels, Mat(), hist[1], dims, &size, ranges);
	calcHist(&Channels[2], 1, &channels, Mat(), hist[2], dims, &size, ranges);


	int hpt = saturate_cast<int>(0.9 * size);
	double minVal = 0;
	double maxVal[3];
	int scale = 3;
	Mat imageShow(size, size * 3, CV_8UC3, Scalar(0, 0, 0)); //

	minMaxLoc(hist[0], &minVal, &maxVal[0], 0, 0);    //cv中用的是cvGetMinMaxHistValue  
	minMaxLoc(hist[1], &minVal, &maxVal[1], 0, 0);
	minMaxLoc(hist[2], &minVal, &maxVal[2], 0, 0);
	for (int i = 0; i < 256; i++) {
		//float value1 = hist[0].at<float>(i);           // 注意hist中是float类型    cv中用cvQueryHistValue_1D  
		//float value2 = hist[1].at<float>(i);
		//float value3 = hist[2].at<float>(i);
		int realValue1 = saturate_cast<int>(hist[0].at<float>(i)* hpt / maxVal[0]);
		int realValue2 = saturate_cast<int>(hist[1].at<float>(i) * hpt / maxVal[1]);
		int realValue3 = saturate_cast<int>(hist[2].at<float>(i) * hpt / maxVal[2]);
		rectangle(imageShow, Point(i, size - 1), Point((i + 1) - 1, size - realValue1), Scalar(0, 0, 255));
		rectangle(imageShow, Point(i + 256, size - 1), Point(i + 256, size - realValue2), Scalar(0, 255, 0));
		rectangle(imageShow, Point(i + 256 * 2, size - 1), Point(i + 256 * 2, size - realValue3), Scalar(255, 0, 0));
	}
	imwrite("get_three.jpg", imageShow);
	//for (int i = 0; i < 256; i++)
	//{
	//float value = hist[0].at<float>(i);           // 注意hist中是float类型    cv中用cvQueryHistValue_1D  
	//	int realValue = saturate_cast<int>(value * hpt / maxVal);
	//rectangle(imageShow, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255));
	//}

	//imshow(WINDOWS_NAMES, imageShow);
	//imwrite("what("+ to_string(num)+").jpg", imageShow);
}

//void threshold(Mat img)
//{
//	MatND hist;       // 在cv中用CvHistogram *hist = cvCreateHist  
//	int dims = 1;
//	float hranges[] = { 0, 255 };
//	const float *ranges[] = { hranges };   //这里需要为const类型  
//	int size = 256;
//	int channels = 0;
//	//计算图像的直方图  
//	calcHist(&img, 1, &channels, Mat(), hist, dims, &size, ranges);    //cv中是cvCalcHist  
//	int hpt = saturate_cast<int>(0.9 * size);
//	double minVal = 0;
//	double maxVal = 0;
//	int scale = 1;
//	Mat imageShow(size * scale, size, CV_8U, Scalar(0));
//
//	minMaxLoc(hist, &minVal, &maxVal, 0, 0);    //cv中用的是cvGetMinMaxHistValue  
//	for (int i = 0; i < 256; i++)
//	{
//		float value = hist.at<float>(i);           // 注意hist中是float类型    cv中用cvQueryHistValue_1D  
//		int realValue = saturate_cast<int>(value * hpt / maxVal);
//		rectangle(imageShow, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255));
//	}
//
//	//imshow(WINDOWS_NAMES, imageShow);
//	imwrite("what(04).jpg", imageShow);
//
//	int realValue[256];
//	int realValue_tmp1;
//	int realValue_tmp2;
//	int realValue_tmp3;
//
//	for (int i = 0; i < 256; i++)
//	{
//		realValue[i] = saturate_cast<int>(hist.at<float>(i) * hpt / maxVal);
//		//rectangle(dstImage, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue[0]), Scalar(255), 1, 8, 0);
//	}
//
//	for (int k = 0; k < 10; k++)
//	{
//		realValue[0] = (realValue[0] + realValue[0] + realValue[1]) / 3;
//
//		for (int i = 1; i < 255; i++)
//		{
//			realValue_tmp1 = realValue[i - 1];
//			realValue_tmp2 = realValue[i];
//			realValue_tmp3 = realValue[i + 1];
//			realValue[i] = (realValue_tmp1 + realValue_tmp2 + realValue_tmp3) / 3;
//		}
//		realValue[255] = (realValue[254] + realValue[255] + realValue[255]) / 3;
//	}
//
//	int num = Get1DMaxEntropyThreshold(realValue);
//	cout << "  Get1DMaxEntropyThreshold  num = " << num << endl;
//	//	num = GetHuangFuzzyThreshold(realValue);
//	//cout << "  GetHuangFuzzyThreshold  num =  " << num << endl;
//	num = GetOSTUThreshold(realValue);
//	cout << "  GetOSTUThreshold  num =  " << num << endl;
//
//	Mat afterImage(size * scale, size, CV_8U, Scalar(0));
//	for (int i = 0; i < 256; i++)
//	{
//		rectangle(afterImage, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue[i]), Scalar(255));
//	}
//	imwrite("afterimg.jpg", afterImage);
//}

int  MyTEST(Mat img2)
{
	MatND hist;       // 在cv中用CvHistogram *hist = cvCreateHist  
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };   //这里需要为const类型  
	int size = 256;
	int channels = 0;
	//计算图像的直方图  
	calcHist(&img2, 1, &channels, Mat(), hist, dims, &size, ranges);    //cv中是cvCalcHist  
	int hpt = saturate_cast<int>(0.9 * size);
	double minVal = 0;
	double maxVal = 0;
	int scale = 1;
	Mat imageShow(size * scale, size, CV_8U, Scalar(0));

	minMaxLoc(hist, &minVal, &maxVal, 0, 0);    //cv中用的是cvGetMinMaxHistValue  
	for (int i = 0; i < 256; i++)
	{
		float value = hist.at<float>(i);           // 注意hist中是float类型    cv中用cvQueryHistValue_1D  
		int realValue = saturate_cast<int>(value * hpt / maxVal);
		rectangle(imageShow, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255));
	}

	//imshow(WINDOWS_NAMES, imageShow);
	imwrite("what(01).jpg", imageShow);

	int realValue[256];
	int realValue_tmp1;
	int realValue_tmp2;
	int realValue_tmp3;

	for (int i = 0; i < 256; i++)
	{
		realValue[i] = saturate_cast<int>(hist.at<float>(i) * hpt / maxVal);
		//rectangle(dstImage, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue[0]), Scalar(255), 1, 8, 0);
	}

	for (int k = 0; k < 15; k++)
	{
		realValue[0] = (realValue[0] + realValue[0] + realValue[1]) / 3;

		for (int i = 1; i < 255; i++)
		{
			realValue_tmp1 = realValue[i - 1];
			realValue_tmp2 = realValue[i];
			realValue_tmp3 = realValue[i + 1];
			realValue[i] = (realValue_tmp1 + realValue_tmp2 + realValue_tmp3) / 3;
		}
		realValue[255] = (realValue[254] + realValue[255] + realValue[255]) / 3;
	}

	Mat afterImage(size * scale, size, CV_8U, Scalar(0));
	for (int i = 0; i < 256; i++)
	{
		rectangle(afterImage, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue[i]), Scalar(255));
	}
	imwrite("afterimg.jpg", afterImage);

	int firstpeak = 0; //从右往左第一个波峰
	for (int i = 255; i >= 9; i--)
	{
		if (realValue[i - 1] < realValue[i])
		{
			if (realValue[i - 2] < realValue[i - 1])
			{
				if (realValue[i - 3] < realValue[i - 2])
				{
					if (realValue[i - 4] < realValue[i - 3])
					{
						if (realValue[i - 5] < realValue[i - 4])
						{
							if (realValue[i - 6] <= realValue[i - 5])
							{
								if (realValue[i - 7] <= realValue[i - 6])
								{
									if (realValue[i - 8] <= realValue[i - 7])
									{
										if (realValue[i - 9] <= realValue[i - 8])
										{
											firstpeak = i;
											break;
										}
									}

								}

							}
						}

					}
				}
			}
		}
	}

	int  firstturn = 0;
	for (int j = firstpeak; j >= 8; j--)
	{
		if (realValue[j - 1] >= realValue[j])
		{
			if (realValue[j - 2] >= realValue[j - 1])
			{
				if (realValue[j - 3] >= realValue[j - 2])
				{
					if (realValue[j - 4] >= realValue[j - 3])
					{
						if (realValue[j - 5] >= realValue[j - 4])
						{
							if (realValue[j - 6] >= realValue[j - 5])
							{
								if (realValue[j - 7] >= realValue[j - 6])
								{
									if (realValue[j - 8] >= realValue[j - 7])
									{
										firstturn = j;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	int firtsttrough = 0;
	for (int j = firstpeak; j >= 8; j--)
	{
		if (realValue[j - 1] > realValue[j])
		{
			if (realValue[j - 2] > realValue[j - 1])
			{
				if (realValue[j - 3] > realValue[j - 2])
				{
					if (realValue[j - 4] > realValue[j - 3])
					{
						if (realValue[j - 5] > realValue[j - 4])
						{
							if (realValue[j - 6] >= realValue[j - 5])
							{
								if (realValue[j - 7] >= realValue[j - 6])
								{
									if (realValue[j - 8] >= realValue[j - 7])
									{
										firtsttrough = j;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//cout << "firstpeak =   " << firstpeak << endl;
	//cout << "firtsttrough =  " << firtsttrough << endl;
	//cout << "firstturn =  " << firstturn << endl;

	int hh = firstturn - firtsttrough;
	int mid = (firstturn + firtsttrough) / 2;
	int num = firtsttrough + hh / 6;
	return num;
}