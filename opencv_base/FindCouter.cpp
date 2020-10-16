#include"FindCouterT.h"

// drawContours(resultImage, contours, -1, Scalar(255, 0, 255));

GetPointRect GetTheRectRangle()
{
	GetPointRect  getpointrect;

	//这里得到左上角的点
	if (corner_point[0].x > corner_point[1].x)
	{
		getpointrect.rect_x = corner_point[1].x;
	}
	else {
		getpointrect.rect_x = corner_point[0].x;
	}

	if (corner_point[1].y > corner_point[2].y)
	{
		getpointrect.rect_y = corner_point[2].y;
	}
	else {
		getpointrect.rect_y = corner_point[1].y;
	}

	//这里计算其长宽高

	if (corner_point[2].x > corner_point[3].x)
	{
		getpointrect.rect_weight = corner_point[2].x - getpointrect.rect_x;
	}
	else
	{
		getpointrect.rect_weight = corner_point[3].x - getpointrect.rect_x;
	}

	if (corner_point[0].y > corner_point[3].y)
	{
		getpointrect.rect_hight = corner_point[0].y - getpointrect.rect_y;
	}
	else {
		getpointrect.rect_hight = corner_point[3].y - getpointrect.rect_y;
	}

	return getpointrect;
}

Mat shihe_drawcouter(Mat src)
{
	Mat binner;
	Mat fghher;
	Mat getrch;
	Mat getrch2;
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	cvtColor(src, fghher, CV_RGB2GRAY);
	//Canny(fghher, binner, 50, 200,3);
	threshold(fghher, binner, 180, 255, CV_THRESH_BINARY);
	//imshow("binner", binner);
	morphologyEx(binner, getrch, 0, element);

	findContours(getrch, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));

		cvBoxPoints(rect, Corners);
		if (rect.size.width  * rect.size.height > 10000 &&
			rect.size.width > 100 &&
			rect.size.height > 100) {

			// 获取得到相应的中心坐标程序
			//	angle = rect.angle;
			//	center.x = rect.center.x;
			//	center.y = rect.center.y;

			line(src, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
			line(src, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
			line(src, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
			line(src, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
		}
	}
	imshow("src",src);
	return src;
}


//贴边画出检测到的矩形
void Findcouter(Mat img)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	bool inpicture = true;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));

		cvBoxPoints(rect, Corners);
		if (rect.size.width  * rect.size.height > 10000 &&
			rect.size.width > 100 &&
			rect.size.height > 100) {


			inpicture = true;
			// for (int n = 0; n < 4; n++) {
			// if (Corners[n].x<0 || Corners[n].x>img.cols || Corners[n].y<0 || Corners[n].y>img.rows) {
			//	 inpicture = false;
			//	 break;
			//	 }

			//	}

			if (inpicture == true) {
				cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
				cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
				for (int n = 0; n < 4; n++) {
					cout << "n =   " << n << "    x = " << Corners[n].x << "  y = " << Corners[n].y << endl;
				}

				line(img, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
				line(img, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
				line(img, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
				line(img, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);

			}
		}
	}

	imwrite("cvLine.jpg", img);
	//Mat Contours = Mat::zeros(img.size(), CV_8UC1);
	//	for (int i = 0; i < contours.size(); i++)
	//{
	//contours[i]代表的是第i个轮廓，contours[i].size()代表的是第i个轮廓上所有的像素点数  
	//	for (int j = 0; j < contours[i].size(); j++)
	//	{
	//绘制出contours向量内所有的像素点  
	//	Point P = Point(contours[i][j].x, contours[i][j].y);
	//	Contours.at<uchar>(P) = 255;
	//	}
	//for (int i = 0; i < contours.size();i++)
	//{
	//	Point2f rect_points[4];

	//}

	//	}
	//imwrite("Findcouter.jpg", Contours);

	//CvBox2D rect;
	//CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(pContour, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
}

float  Findcouter_center(Mat img, Point * point)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	float angle = 0.0;
	int pointnum = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));

		cvBoxPoints(rect, Corners);
		if (rect.size.width  * rect.size.height > 10000 &&
			rect.size.width > 100 &&
			rect.size.height > 100) {


			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			angle = rect.angle;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			point[pointnum].x = rect.center.x;
			point[pointnum].y = rect.center.y;
			pointnum++;
			//for (int n = 0; n < 4; n++) {
			//	cout << "n =   " << n << "    x = " << Corners[n].x << "  y = " << Corners[n].y << endl;
			//}

			line(img, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
			line(img, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
			line(img, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
			line(img, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);

		}
	}

	imwrite("cvLine.jpg", img);
	return angle;

}

Mat whatTHis(Mat img, Mat src, Point * center1)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat imme = img.clone();
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];

	CvPoint2D32f center;
	Point gtecross;
	gtecross.x = 0;
	gtecross.y = 0;

	vector<Rect> boundRect(contours.size());
	//	vector<Rect> getlast;
	//int getnum = 0;
	//	boundRect[i] = boundingRect(Mat(contours[i]));
	int num[5][5];
	int numlen = 0;
	int numxy = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);
		boundRect[i] = boundingRect(Mat(contours[i]));
		if (rect.size.height *rect.size.width > 15000 && rect.size.height > 100 && rect.size.width > 100)
		{
			//cout << "rect.angle  =   " << rect.angle << endl;
			//	cout << "rect.center.x =  " << rect.center.x << "      rect.center.y = " << rect.center.y << endl;
			//center.x = rect.center.x;
			//center.y = rect.center.y;
			gtecross = rect.center;
			for (int j = 0; j <= 3; j++)
			{
				line(img, Corners[j], Corners[(j + 1) % 4], Scalar(255), 2);
			}
			//画出外面正常框出的矩形
			rectangle(imme, Point(boundRect[i].x, boundRect[i].y), Point(boundRect[i].x + boundRect[i].width, boundRect[i].y + boundRect[i].height), Scalar(255), 2, 8);
			num[numlen][numxy] = i;
			num[numlen][numxy + 1] = boundRect[i].x;
			numlen++;
			numxy++;
			//	getlast[getnum].x = boundRect[i].x;
			//	getlast[getnum].y = boundRect[i].y;
			//getlast[getnum].width = boundRect[i].width;
			//	getlast[getnum].height = boundRect[i].height;
			//	getnum++;
		}
	}
	imwrite("getout.jpg", imme);
	int d = 0;
	if (num[0][1] > num[1][1])
	{
		d = num[0][0];
	}
	else {
		d = num[1][0];
	}
	Rect rect1(boundRect[d].x, boundRect[d].y, boundRect[d].width, boundRect[d].height);
	*center1 = gtecross;
	Mat dest = src(rect1);
	return dest;
	//rectangle(dstImg, Point(boundRect[i].x, boundRect[i].y), Point(boundRect[i].x + boundRect[i].width, boundRect[i].y + boundRect[i].height), Scalar(0, 255, 0), 2, 8);
}

Point Findcouterthis(Mat img, float &angle)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];

	CvPoint2D32f center;
	Point gtecross;
	gtecross.x = 0;
	gtecross.y = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);

		if (rect.size.height *rect.size.width > 10000 && rect.size.height > 100 && rect.size.width > 100)
		{
			cout << "rect.angle  =   " << rect.angle << endl;
			angle = rect.angle;
			cout << "rect.center.x =  " << rect.center.x << "      rect.center.y = " << rect.center.y << endl;
			//center.x = rect.center.x;
			//center.y = rect.center.y;
			gtecross = rect.center;
			for (int j = 0; j <= 3; j++)
			{
				line(img, Corners[j], Corners[(j + 1) % 4], Scalar(255), 2);
			}
		}
	}

	imwrite("out.jpg", img);
	return gtecross;
	//Mat dst(img.size(), img.type());
	//Mat rotMat = getRotationMatrix2D(center, 90+angle, 1);
	//warpAffine(img, dst, rotMat, img.size());
	//imwrite("xuanzhuang.jpg",dst);
}

Point Findcouter_foroutside(Mat img, float &angle)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];

	CvPoint2D32f center;
	Point gtecross;
	gtecross.x = 0;
	gtecross.y = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);

		if (rect.size.height *rect.size.width > 10000 && rect.size.height > 100 && rect.size.width > 100)
		{
			cout << "rect.angle  =   " << rect.angle << endl;
			angle = rect.angle;
			cout << "rect.center.x =  " << rect.center.x << "      rect.center.y = " << rect.center.y << endl;

			gtecross = rect.center;
			for (int j = 0; j <= 3; j++)
			{
				line(img, Corners[j], Corners[(j + 1) % 4], Scalar(255), 2);
			}
		}
	}
	imwrite("foroutside.jpg", img);
	return gtecross;

}

float Findcouter_inside_small(Mat img)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];

	CvPoint2D32f center;
	Point gtecross;
	gtecross.x = 0;
	gtecross.y = 0;
	cout << "img.rows  =   " << img.rows << "      img.cols  =  " << img.cols << endl;
	int n = 0;
	float angle = 0.0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);

		if (rect.size.height *rect.size.width > 2500 && rect.size.height > 50 && rect.size.width > 50
			&& rect.size.height < 150 && rect.size.width < 150)
		{
			if (rect.center.x >= 0 && rect.center.x <= img.cols &&rect.center.y >= 0 && rect.center.y <= img.rows)
			{
				cout << "rect.angle  =   " << rect.angle << endl;
				angle = rect.angle;
				cout << "rect.center.x =  " << rect.center.x << "      rect.center.y = " << rect.center.y << endl;
				//center.x = rect.center.x;
				//center.y = rect.center.y;
				gtecross = rect.center;
				for (int j = 0; j <= 3; j++)
				{
					line(img, Corners[j], Corners[(j + 1) % 4], Scalar(255), 2);
				}
				n++;
			}
		}
	}

	if (n > 1) {
		return 0;
	}
	if (gtecross.x > img.cols / 2)
	{
		if (gtecross.y > img.rows / 2)
		{
			angle = 90 + angle;
			cout << "右上角" << endl;
		}
		else {
			angle = angle;
			cout << "左上角 " << endl;
		}
	}
	else {
		if (gtecross.y > img.rows / 2) {
			angle = 180 + angle;
			cout << "右下角 " << endl;
		}
		else {
			angle = angle - 90;
			cout << "左下角 " << endl;
		}

	}
	imwrite("wherehththttTTTTT.jpg", img);
	return angle;
	//Mat dst(img.size(), img.type());
	//Mat rotMat = getRotationMatrix2D(center, 90+angle, 1);
	//warpAffine(img, dst, rotMat, img.size());
	//imwrite("xuanzhuang.jpg",dst);
}

Point Findcouter_Positive(Mat img, float * angle)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	Point  Twocengter[2];

	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));

		cvBoxPoints(rect, Corners);
		if (rect.size.width  * rect.size.height > 2500 &&
			rect.size.width > 50 &&
			rect.size.height > 50) {

			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			*angle = rect.angle;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			for (int n = 0; n < 4; n++) {
				line(img, Point((int)Corners[n].x, (int)Corners[n].y), Point((int)Corners[(n + 1) % 4].x, (int)Corners[(n + 1) % 4].y), Scalar(255, 0, 255), 2, 8, 0);
			}
			if (rect.size.width > 150 && rect.size.height > 150)
			{
				Twocengter[0].x = rect.center.x;
				Twocengter[0].y = rect.center.y;
			}
			else {
				Twocengter[1].x = rect.center.x;
				Twocengter[1].y = rect.center.y;
			}
		}

	}

	if (Twocengter[1].x >Twocengter[0].x)
	{
		if (Twocengter[1].y >Twocengter[0].y)
		{
			*angle = *angle;
			cout << "左上角" << endl;
		}
		else {
			*angle = *angle - 90;
			cout << "左下角" << endl;
		}
	}
	else {
		if (Twocengter[1].y >Twocengter[0].y)
		{
			*angle = 90 + *angle;
			cout << "右上角" << endl;
		}
		else {
			*angle = 180 + *angle;
			cout << "右下角" << endl;
		}
	}


	imwrite("cvLine.jpg", img);
	return Twocengter[0];

}

Mat  Findcouter_three(Mat img, Mat src, float * angle, Point * Center)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	bool inpicture = true;
	cout << "we have so many couters  =  " << contours.size() << endl;
	vector<Rect> boundRect(contours.size());
	Mat imme = img.clone();
	int get = 0;
	int numwid = 0;
	int numhei = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);
		boundRect[i] = boundingRect(Mat(contours[i]));
		if (rect.size.width > 100 && rect.size.height >100)
		{
			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			*angle = rect.angle;
			Point gett;
			gett = rect.center;
			(*Center).x = gett.x;
			(*Center).y = gett.y;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			for (int n = 0; n < 4; n++) {
				line(img, Point((int)Corners[n].x, (int)Corners[n].y), Point((int)Corners[(n + 1) % 4].x, (int)Corners[(n + 1) % 4].y), Scalar(255), 2, 8, 0);
			}
			cout << "boundRect[i].x =    " << boundRect[i].x << "        boundRect[i].x =    " << boundRect[i].y << endl;
			numwid = (boundRect[i].width * 0.2) / 2;
			numhei = (boundRect[i].height * 0.2) / 2;
			get = i;
			rectangle(imme, Point(boundRect[i].x + numwid, boundRect[i].y + numhei),
				Point(boundRect[i].x + boundRect[i].width*0.8 + numwid, boundRect[i].y + boundRect[i].height*0.8 + numhei), Scalar(255), 2, 8);
		}
	}

	Rect rect1(boundRect[get].x + numwid, boundRect[get].y + numhei, boundRect[get].width*0.9, boundRect[get].height*0.9);
	Mat dest = src(rect1);
	//imwrite("cvLine.jpg", img);
	imwrite("cvLine_2.jpg", imme);
	imwrite("ofcourse.jpg", img);
	return dest;
}

int  Findcouter_TWO_inside(Mat img)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	bool inpicture = true;
	cout << "we have so many couters  =  " << contours.size() << endl;
	Point getTWO[2];
	int numpoint = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);

		if (rect.size.width > 100 && rect.size.height >100)
		{
			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			if (numpoint >= 2) {
				cout << " error numpoint is too big " << endl;
			}
			else {
				getTWO[numpoint].x = rect.center.x;
				getTWO[numpoint].y = rect.center.y;
				numpoint++;
			}
			for (int n = 0; n < 4; n++) {
				line(img, Point((int)Corners[n].x, (int)Corners[n].y), Point((int)Corners[(n + 1) % 4].x, (int)Corners[(n + 1) % 4].y), Scalar(255, 0, 255), 2, 8, 0);
			}
		}
	}

	imwrite("cvLine.jpg", img);
	float k;
	//计算斜率 以及斜率不存在的情况 
	//由于是矩形不考虑畸变只分横和
	if ((getTWO[1].x - getTWO[0].x) != 0) //计算斜率 以及斜率不存在的情况  
	{
		k = ((float)getTWO[1].y - getTWO[0].y) / (getTWO[1].x - getTWO[0].x);
		if (k >= 0)
		{
			return  1;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}

}

Point Findcouter_Slash(Mat img, float * angle)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	Point center;
	bool inpicture = true;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));

		cvBoxPoints(rect, Corners);
		if (rect.size.width  * rect.size.height > 10000 &&
			rect.size.width > 100 &&
			rect.size.height > 100) {

			inpicture = true;

			*angle = rect.angle;
			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			center.x = (int)rect.center.x;
			center.y = (int)rect.center.y;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			//	for (int n = 0; n < 4; n++) {
			//	line(img, Point((int)Corners[n].x, (int)Corners[n].y), Point((int)Corners[(n+1)%4].x, (int)Corners[(n + 1) % 4].y), Scalar(255, 0, 255), 2, 8, 0);
			//}
		}
	}

	imwrite("cvLine.jpg", img);
	return center;
}