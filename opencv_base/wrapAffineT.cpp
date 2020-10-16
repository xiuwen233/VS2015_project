#include"warpAffineT.h"

Mat ImageChangle(Point center, float angle, Mat src)
{
	Mat dst(src.size(), src.type());
	Mat rotMat = getRotationMatrix2D(center, angle, 1);
	warpAffine(src, dst, rotMat, src.size());
	return src;
}