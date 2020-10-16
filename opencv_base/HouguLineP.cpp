#include"HouguLinePu.h"

Mat HoufuLineP(Mat src)
{ 
	Myline L[1024];
	Mat clolorimg, cannyimg, getrch, binner;
	vector<Vec4i> lines;
	cvtColor(src, clolorimg, CV_RGB2GRAY);
	threshold(clolorimg, binner, 180, 255, CV_THRESH_BINARY);
	imshow("binner", binner);
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	morphologyEx(binner, getrch, 0, element);
	imshow("getrch", getrch);
	Canny(getrch, cannyimg, 50, 200, 3);
	imshow("cannyimg", cannyimg);
	HoughLinesP(cannyimg, lines, 1, CV_PI / 360, 100, 60, 10);
	for (int i = 0; i < lines.size() && i<32; i++)
	{
		L[i].start.x = lines[i][0];
		L[i].start.y = lines[i][1];
		L[i].end.x = lines[i][2];
		L[i].end.y = lines[i][3];
		line(src, L[i].start, L[i].end, Scalar(255, 0, 255), 2, 8, 0);
	}
	imshow("src",src);
	return src;

}

Point getCross(Myline line1, Myline line2)
{
	Point P;
	//y = a * x + b;

	float k1, b1; //直线line1的斜率, 截距  依据传入的循序，line1是竖线，有可能是 直线 x = n
	float k2, b2; //直线line1的斜率, 截距
	if ((line1.start.x - line1.end.x) != 0) {
		k1 = ((float)line1.start.y - line1.end.y) / (line1.start.x - line1.end.x);
		b1 = (float)line1.start.y - k1 * (line1.start.x);
		k2 = ((float)line2.start.y - line2.end.y) / (line2.start.x - line2.end.x);
		b2 = (float)line2.start.y - k2 * (line2.start.x);

		P.x = (b1 - b2) / (k2 - k1);
		P.y = k2 * (b1 - b2) / (k2 - k1) + b2;
	}
	else {
		k2 = ((float)line2.start.y - line2.end.y) / (line2.start.x - line2.end.x);
		b2 = (float)line2.start.y - k2 * (line2.start.x);

		P.x = line1.start.x;
		P.y = k2* line1.start.x + b2;
	}
	return P;
}