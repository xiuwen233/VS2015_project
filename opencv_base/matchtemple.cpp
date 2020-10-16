#include"matchtemple.h"

/*
*  TM_SQDIFF   TM_SQDIFF_NORMED  最小的值 越小越好
*  其他的都是大的好
*/
Mat  MatchTempleImage(Mat src, Mat temple)
{
	Mat result;
	int result_rows = src.rows - temple.rows + 1;
	int result_cols = src.cols - temple.cols + 1;
	result.create(result_cols, result_rows, CV_32FC1);
	matchTemplate(src, temple, result, CV_TM_SQDIFF_NORMED);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	double minVal = -1;
	double maxVal;
	Point minLoc;
	Point maxLoc;
	Point matchLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	if (CV_TM_SQDIFF_NORMED == TM_SQDIFF || CV_TM_SQDIFF_NORMED == TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
	}
	else
	{
		matchLoc = maxLoc;
	}
	rectangle(src, matchLoc, Point(matchLoc.x + temple.cols, matchLoc.y + temple.rows), Scalar(0, 255, 0), 2, 8, 0);
	imshow("src", src);
	return src;

}