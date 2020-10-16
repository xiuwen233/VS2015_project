#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

struct Myline {//每条线
	Point start;//起点
	Point end;//终点
	int linelen;  // 线的长度
};

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

Point HoughLi(Mat img, Mat ourage)
{
	vector<Vec4i> lines;
	HoughLinesP(img, lines, 1, CV_PI / 360, 100, 50, 18);
	Myline L[1024];
	Myline  vertical[100], across[100]; //用来存放
	int numver = 0, numacr = 0;
	cout << "lines.size()" << lines.size() << endl;
	for (int i = 0; i < lines.size(); i++)
	{
		//line(img, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]),Scalar(255),2 ,8,0);
		L[i].start.x = lines[i][0];
		L[i].start.y = lines[i][1];
		L[i].end.x = lines[i][2];
		L[i].end.y = lines[i][3];
		L[i].linelen = (lines[i][0] - lines[i][2])* (lines[i][0] - lines[i][2]) + (lines[i][1] - lines[i][3])*(lines[i][1] - lines[i][3]);

		float k;
		//计算斜率 以及斜率不存在的情况 
		//由于是矩形不考虑畸变只分横和
		if ((L[i].start.x - L[i].end.x) != 0) //计算斜率 以及斜率不存在的情况  
		{
			k = ((float)L[i].start.y - L[i].end.y) / (L[i].start.x - L[i].end.x);
		}
		else {
			k = -3;
		}

		//直线k<0  0是左 2 上 1 右 3下
		//直线K>=0 0是下，1上，2左 3右
		if (k<0)	//判断直线横或竖 45度角为分界线       
		{
			vertical[numver++] = L[i];
		}
		else {
			across[numacr++] = L[i];
		}
	}
	//cout << "numver =  " << numver << "    numacr =  " << numacr << endl;
	int maxmidx = 0;
	int maxmidy = 0;
	int maxy = 0;
	int maxx = 0;

	for (int i = 0; i <(numver - 1); i++)
	{
		for (int j = i, k = i; j < (numver - 1); j++)
		{
			int mid = abs(((vertical[k].start.x + vertical[k].end.x) / 2 - (vertical[j + 1].start.x + vertical[j + 1].end.x) / 2) / 2);
			if (mid > maxmidx) {
				maxmidx = mid;
			}

			if ((vertical[k].start.x > vertical[j + 1].start.x))
			{
				if (vertical[k].start.x > maxx)
				{
					maxx = vertical[k].start.x;
				}
			}
			else
			{
				if (vertical[j + 1].start.x > maxx)
				{
					maxx = vertical[j + 1].start.x;
				}
			}

		}
	}

	for (int i = 0; i < (numacr - 1); i++)
	{
		for (int j = i, k = i; j < (numacr - 1); j++)
		{
			int mid = abs(((across[k].start.y + across[k].end.y) / 2 - (across[j + 1].start.y + across[j + 1].end.y) / 2) / 2);
			if (mid > maxmidy) {
				maxmidy = mid;
			}
			if (across[k].start.y > across[j + 1].start.y)
			{
				if (across[k].start.y > maxy)
				{
					maxy = across[k].start.y;
				}
			}
			else
			{
				if (across[j + 1].start.y > maxy)
				{
					maxy = across[j + 1].start.y;
				}
			}

		}
	}
	//进一步分成4条边的数组
	Myline  verticalRight[50], verticalLeft[50];
	Myline  acrossUp[50], acrossDown[50];

	int vRi = 0, vLi = 0;
	for (int i = 0; i < numver; i++)
	{
		if ((vertical[i].start.x + vertical[i].end.x) / 2 >(maxx - maxmidx))
		{
			verticalRight[vRi++] = vertical[i];
		}
		else {
			verticalLeft[vLi++] = vertical[i];
		}
	}

	int aRi = 0, aLi = 0;
	for (int i = 0; i < numacr; i++)
	{
		if ((across[i].start.y + across[i].end.y) / 2 >(maxy - maxmidy))
		{
			acrossDown[aRi++] = across[i];
		}
		else {
			acrossUp[aLi++] = across[i];
		}
	}
	Myline releseout[4];   //用于储存最终获得4条线
						   //从4条边数组分别取出最长的一条边  理论上越长越直
	if (vRi == 1) {
		releseout[1] = verticalRight[0];
	}
	else {
		for (int i = 0; i < vRi - 1; i++)
		{
			if (verticalRight[i].linelen > verticalRight[i + 1].linelen)
			{
				releseout[1] = verticalRight[i];
			}
			else {
				releseout[1] = verticalRight[i + 1];
			}
		}
	}

	if (vLi == 1) {
		releseout[0] = verticalLeft[0];
	}
	else {
		for (int i = 0; i < vLi - 1; i++)
		{
			if (verticalLeft[i].linelen > verticalLeft[i + 1].linelen)
			{
				releseout[0] = verticalLeft[i];
			}
			else {
				releseout[0] = verticalLeft[i + 1];
			}
		}
	}

	if (aRi == 1) {
		releseout[3] = acrossDown[0];
	}
	else {
		for (int i = 0; i < (aRi - 1); i++)
		{
			if (acrossDown[i].linelen > acrossDown[i + 1].linelen)
			{
				releseout[3] = acrossDown[i];
			}
			else {
				releseout[3] = acrossDown[i + 1];
			}
		}
	}

	if (aLi == 1) {
		releseout[2] = acrossUp[0];
	}
	else {
		for (int i = 0; i < (aLi - 1); i++)
		{
			if (acrossUp[i].linelen > acrossUp[i + 1].linelen)
			{
				releseout[2] = acrossUp[i];
			}
			else {
				releseout[2] = acrossUp[i + 1];
			}
		}
	}

	for (int n = 0; n < 4; n++)
	{
		line(ourage, releseout[n].start, releseout[n].end, Scalar(0, 255, 0), 2, 8, 0);
	}
	//imwrite("cloloLine.jpg", ourage);
	Myline forcenter1, forcenter2;
	Point outp;
	outp = getCross(releseout[0], releseout[2]);
	//cout << " outp.x  =  " << outp.x << "    outp.y =   " << outp.y << endl;
	forcenter1.start = outp;
	outp = getCross(releseout[1], releseout[2]);
	forcenter2.end = outp;
	outp = getCross(releseout[0], releseout[3]);
	forcenter2.start = outp;
	outp = getCross(releseout[1], releseout[3]);
	forcenter1.end = outp;

	Point centerSize;
	centerSize = getCross(forcenter1, forcenter2);

	return centerSize;
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

		if (rect.size.height *rect.size.width > 15000 && rect.size.height > 100 && rect.size.width > 100)
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

	return gtecross;
}


Mat justforGrenn(Mat img)
{
	for (int i = 0; i < img.rows; i++)
	{
		uchar* p = img.ptr(i);//获取Mat某一行的首地址  

		for (int j = 0; j < img.cols; j++)
		{
			//顺序是BGR,不是RGB
			int B = *(p + j * 3);
			int G = *(p + j * 3 + 1);
			int R = *(p + j * 3 + 2);
			if ((G - R)>5 && (G - B)>5) // R > num && G>num && B>num  Gray >  num
			{
				*(p + j * 3) = 255;
				*(p + j * 3 + 1) = 255;
				*(p + j * 3 + 2) = 255;
			}
			else {
				//*(p + j * 3) = 255;
				//*(p + j * 3 + 1) = 255;
				//*(p + j * 3 + 2) = 255;
			}
		}
	}
	imwrite("JustGreen.jpg", img);
	return img;
}

void OneTest()
{

	Mat img, img2, img3, out;
	String beaforeclasspath = "E://photo_opencv/boxTest/what/what(";
	String afterclasspath = ").jpg";
	for (int i = 90; i < 94; i++) {
		String classpath = beaforeclasspath + to_string(i) + afterclasspath;
		img = imread(classpath);
		Mat source = img.clone();
		Mat immg = justforGrenn(img);
		medianBlur(immg, out, 7);
		cvtColor(out, out, CV_RGB2GRAY);
		Canny(out, out, 50, 100, 3);

		Point sidecenter;
		float angle;
		Mat fingimg = out.clone();
		sidecenter = Findcouterthis(fingimg, angle);
		Point srccenter;
		Mat houghimg = out.clone();
		Mat houimg = img.clone();
		srccenter = HoughLi(houghimg, houimg);

		cout << "sidecenter.x  =  " << sidecenter.x << "    sidecenter.y  =" << sidecenter.y << endl;
		cout << "srccenter.x  =  " << srccenter.x << "    srccenter.y  =" << srccenter.y << endl;

		circle(source, sidecenter, 3, Scalar(255, 0, 255), -1); //第五个参数我设为-1，表明这是个实点
		circle(source, srccenter, 3, Scalar(0, 255, 255), -1);
		imwrite("source" + to_string(i) + ".jpg", source);
		if (sidecenter.x > srccenter.x)
		{
			if (sidecenter.y > srccenter.y) {
				angle = 180 + angle;
				cout << "右上角 " << endl;
			}
			else {
				angle = 90 + angle;
				cout << "右上角 " << endl;
			}
		}
		else {
			if (sidecenter.y > srccenter.y) {
				angle = angle - 90;
				cout << "左下角 " << endl;
			}
			else {
				angle = angle;
				cout << "左上角 " << endl;
			}
		}
		cout << " final angle === " << angle;
		Mat dst(img.size(), img.type());
		Mat rotMat = getRotationMatrix2D(srccenter, angle, 1);
		warpAffine(img, dst, rotMat, img.size());
		imwrite("xuanzhuang" + to_string(i) + ".jpg", dst);
	}
	
	getchar();
}