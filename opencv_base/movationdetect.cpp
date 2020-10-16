
#include"movationdetect.h"

// this is for two �˶�����������Բ��ϼ�����  ȡ��һ֡Ϊ������̫��
static Mat MoveDetect(Mat frame1, Mat frame2)
{
	Mat result = frame2.clone();
	Mat gray1, gray2;
	cvtColor(frame1, gray1, CV_BGR2GRAY);
	cvtColor(frame2, gray2, CV_BGR2GRAY);

	Mat diff;
	absdiff(gray1, gray2, diff);
	// imshow("absdiss", diff);
	threshold(diff, diff, 45, 255, CV_THRESH_BINARY);
	// imshow("threshold", diff);

	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat element2 = getStructuringElement(MORPH_RECT, Size(25, 25));
	erode(diff, diff, element);
	//  imshow("erode", diff);

	dilate(diff, diff, element2);
	//  imshow("dilate", diff);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarcy;
	//����Բ������
	findContours(diff, contours, hierarcy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	cout << "num=" << contours.size() << endl;
	vector<RotatedRect> box(contours.size());
	for (int i = 0; i<contours.size(); i++)
	{
		box[i] = fitEllipse(Mat(contours[i]));
		ellipse(result, box[i], Scalar(0, 255, 0), 2, 8);
		circle(result, box[i].center, 3, Scalar(0, 0, 255), -1, 8);
	}
	return result;
}

void MotionTest()
{
	VideoCapture cap(0);
	if (!cap.isOpened()) //�����Ƿ�ɹ�
		return;
	Mat frame;
	Mat result;
	Mat background;
	int count = 0;
	while (1)
	{
		cap >> frame;
		if (frame.empty())
			break;
		else {
			count++;
			if (count == 1)
				background = frame.clone(); //��ȡ��һ֡Ϊ����֡

			imshow("video", frame);
			result = MoveDetect(background, frame);
			imshow("result", result);
			if (waitKey(50) == 27)
				break;
		}
	}
	//cap.release();
}


//�˲�����ȥ������    ��һ֡Ϊ������̫��

string intToString(int number)
{
	stringstream ss;
	ss << number;
	return ss.str();
}

Mat MoveDetect_fliter(Mat background, Mat img)
{
	Mat result = img.clone();
	Mat gray1, gray2;
	cvtColor(background, gray1, CV_BGR2GRAY);
	cvtColor(img, gray2, CV_BGR2GRAY);

	Mat diff;
	absdiff(gray1, gray2, diff);
	threshold(diff, diff, 20, 255, CV_THRESH_BINARY);
	// imshow("threshold", diff);
	blur(diff, diff, Size(10, 10));//��ֵ�˲�
								   //imshow("blur", diff);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarcy;
	findContours(diff, contours, hierarcy, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE); //��������
	vector<Rect> boundRect(contours.size()); //������Ӿ��μ���
											 //drawContours(img2, contours, -1, Scalar(0, 0, 255), 1, 8);  //��������
	int x0 = 0, y0 = 0, w0 = 0, h0 = 0;
	for (int i = 0; i<contours.size(); i++)
	{
		boundRect[i] = boundingRect((Mat)contours[i]); //����ÿ����������Ӿ���

		x0 = boundRect[i].x;  //��õ�i����Ӿ��ε����Ͻǵ�x����
		y0 = boundRect[i].y; //��õ�i����Ӿ��ε����Ͻǵ�y����
		w0 = boundRect[i].width; //��õ�i����Ӿ��εĿ��
		h0 = boundRect[i].height; //��õ�i����Ӿ��εĸ߶�
								  //rectangle(result, Point(x0, y0), Point(x0+w0, y0+h0), Scalar(0, 255, 0), 2, 8); //���Ƶ�i����Ӿ���
		circle(result, Point(x0 + w0 / 2, y0 + h0 / 2), 15, Scalar(0, 255, 0), 2, 8);
		line(result, Point(x0 + w0 / 2 - 15, y0 + h0 / 2), Point(x0 + w0 / 2 + 15, y0 + h0 / 2), Scalar(0, 255, 0), 2, 8);
		line(result, Point(x0 + w0 / 2, y0 + h0 / 2 - 10), Point(x0 + w0 / 2, y0 + h0 / 2 + 15), Scalar(0, 255, 0), 2, 8);
		putText(result, "(" + intToString(x0 + w0 / 2) + "," + intToString(y0 + h0 / 2) + ")", Point(x0 + w0 / 2 + 15, y0 + h0 / 2), 1, 1, Scalar(255, 0, 0), 2);
	}
	return result;
}

void MotionDector_Fliter()  //���Ч���ܲ� ��֪���ڸ�ʲô
{
	VideoCapture cap(0);
	if (!cap.isOpened()) //�����Ƿ�ɹ�
		return;
	Mat frame;
	Mat result;
	Mat background;
	int count = 0;
	while (1)
	{
		cap >> frame;
		if (frame.empty())
			break;
		else {
			count++;
			if (count == 1)
				background = frame.clone(); //��ȡ��һ֡Ϊ����֡
			imshow("video", frame);
			result = MoveDetect_fliter(background, frame);
			imshow("result", result);
			if (waitKey(50) == 27)
				break;
		}
	}
	//cap.release();
}


//  �˶��켣���� 

Point center;
Point fre_center;//�洢ǰһ֡��������
int num = 0;
vector<Point> points;

//Mat MoveDetect(Mat background, Mat img)
//{
// Mat result = img.clone();
// Mat gray1, gray2;
// cvtColor(background, gray1, CV_BGR2GRAY);
// cvtColor(img, gray2, CV_BGR2GRAY);

// Mat diff;
// absdiff(gray1, gray2, diff);
// imshow("absdiss", diff);
// threshold(diff, diff, 45, 255, CV_THRESH_BINARY);
// imshow("threshold", diff);

// Mat element = getStructuringElement(MORPH_RECT, Size(1, 1));
// Mat element2 = getStructuringElement(MORPH_RECT, Size(9, 9));
// erode(diff, diff, element);
// imshow("erode", diff);
// dilate(diff, diff, element2);
// imshow("dilate", diff);

// vector<vector<Point>> contours;
// vector<Vec4i> hierarcy;
// findContours(diff, contours, hierarcy, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE); //��������
// vector<Rect> boundRect(contours.size()); //������Ӿ��μ���
//										  //drawContours(img2, contours, -1, Scalar(0, 0, 255), 1, 8);  //��������
// vector<RotatedRect> box(contours.size());
// int x0 = 0, y0 = 0, w0 = 0, h0 = 0;
// for (int i = 0; i<contours.size(); i++)
// {
//	 boundRect[i] = boundingRect((Mat)contours[i]); //����ÿ����������Ӿ���

//	 x0 = boundRect[i].x;  //��õ�i����Ӿ��ε����Ͻǵ�x����
//	 y0 = boundRect[i].y; //��õ�i����Ӿ��ε����Ͻǵ�y����
//	 w0 = boundRect[i].width; //��õ�i����Ӿ��εĿ��
//	 h0 = boundRect[i].height; //��õ�i����Ӿ��εĸ߶�
//	 if (w0>30 && h0>30)//ɸѡ�������30������
//	 {
//		 num++;
//		 //rectangle(result, Point(x0, y0), Point(x0+w0, y0+h0), Scalar(0, 255, 0), 2, 8); //���Ƶ�i����Ӿ���
//		 box[i] = fitEllipse(Mat(contours[i]));
//		 ellipse(result, box[i], Scalar(255, 0, 0), 2, 8);            //��Բ����
//		 circle(result, box[i].center, 3, Scalar(0, 0, 255), -1, 8); //������
//		 center = box[i].center;//��ǰ֡����������
//		 points.push_back(center);//��������points������
//		 if (num != 1)
//		 {
//			 //line(result, fre_center, center, Scalar(255, 0, 0), 2, 8);
//			 for (int j = 0; j<points.size() - 1; j++)
//				 line(result, points[j], points[j + 1], Scalar(0, 255, 0), 2, 8);
//		 }
//		 //fre_center = center;
//	 }
// }
// return result;
//}

void main12()
{
	VideoCapture cap("E://man.avi");
	if (!cap.isOpened()) //�����Ƿ�ɹ�
		return;
	Mat frame;
	Mat background;
	Mat result;
	int count = 0;
	while (1)
	{
		cap >> frame;
		if (!frame.empty())
		{
			count++;
			if (count == 1)
				background = frame.clone(); //��ȡ��һ֡Ϊ����֡
			imshow("video", frame);
			result = MoveDetect(background, frame);
			imshow("result", result);
			if (waitKey(30) == 27)
				break;
		}
		else
			break;
	}
	cap.release();
}