#include"differentFilter.h"

void Sobellok()
{
	Mat imme = imread("E://photo_opencv/temp_2317.jpg", 1);
	Mat img, um;
	Sobel(imme, img, imme.depth(), 1, 0, 3, 1);
	imwrite("Sobel_X.jpg", img);
	Sobel(imme, um, imme.depth(), 0, 1, 3, 1);
	imwrite("Sobel_Y.jpg", um);
}

void Lapulasi()
{
	Mat imme = imread("E://photo_opencv/temp_2317.jpg", 1);
	Mat dst, out;
	Laplacian(imme, dst, CV_16S, 3, 1);
	imwrite("Lapulasi.jpg", dst);
}

void UpOrDown()
{
	Mat imme = imread("E://photo_opencv/temp_2317.jpg", 1);
	Mat big, small;
	pyrUp(imme, big, Size(imme.cols * 2, imme.rows * 2));
	imwrite("pyrUp.jpg", big);
	pyrDown(imme, small, Size(imme.cols / 2, imme.rows / 2));
	imwrite("pyrDown.jpg", small);
}

void HoufuHUAxian()  //经典的画出来太不好了
{
	Mat imme = imread("E://photo_opencv/temp_2317.jpg", 1);
	Mat imgcvt, canimg;
	cvtColor(imme, imgcvt, CV_RGB2GRAY);
	Canny(imgcvt, canimg, 50, 100, 3);
	vector<Vec2f> lines;//定义一个矢量结构lines用于存放得到的线段矢量集合  
	HoughLines(canimg, lines, 1, CV_PI / 180, 150, 50, 10);
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(imgcvt, pt1, pt2, Scalar(55, 100, 195), 1, CV_AA);
	}
	imwrite("imgcvt.jpg", imgcvt);
}

void FloodFillImg()
{
	Mat imme = imread("E://photo_opencv/temp_2317.jpg", 1);
	Rect ccomp;
	floodFill(imme, Point(50, 300), Scalar(155, 255, 55), &ccomp, Scalar(20, 20, 20), Scalar(20, 20, 20));
	cout << "" << ccomp.size() << endl;
	cout << "" << ccomp.area() << endl;
	imwrite("FloodFill.jpg", imme);
}