#include"MatPatDisplay.h"


/*
* absdiff(src,dst,differenc_iamge); 灰度图图像相减
*/


/*
* 计算角度
    float angle = 0.0;
	angle = atan(ta_cos);
	printf("angle = %f  \n", angle * 180.0 / 3.1416);
*/
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
			//float Gray = R*0.299 + G*0.587 + B*0.114;
			if ((G - R)>7 && (B - G)>20 || (R>250 && B>250 && G>250 && R<254 && G<254 && B<254)) // R > num && G>num && B>num  Gray >  num  || ( R>200 && G>200 && B>200  && abs(R-B)<20 && abs(R - G)<20)
			{
				*(p + j * 3) = 255;
				*(p + j * 3 + 1) = 255;
				*(p + j * 3 + 2) = 255;
			}
			else {
				*(p + j * 3) = 0;
				*(p + j * 3 + 1) = 0;
				*(p + j * 3 + 2) = 0;
			}
		}
	}
	//imwrite("JustGreen.jpg", img);
	return img;
}


void whattodo()
{
	Mat imme = imread("E://photo_opencv/girl.jpg", 1);
	cout << "" << imme.channels() << endl;
	Mat img1 = imread("E://photo_opencv/girl.jpg", -1);
	cout << "" << img1.channels() << endl;
}


void MatAdd()
{
	Mat img = imread("E://photo_opencv/dota.jpg");
	Mat logo = imread("E://photo_opencv/logo.jpg");
	// Mat mask = imread("E://photo_opencv/logo.jpg", 0);
	Mat imageROI = img(Rect(0, 0, logo.cols, logo.rows));
	// logo.copyTo(imageROI, mask);
	addWeighted(imageROI, 0.5, logo, 0.3, 0., imageROI);
	imwrite("idonoyt.jpg", img);

}


void Maskadd()
{
	Mat img = imread("E://photo_opencv/dota.jpg");
	Mat logo = imread("E://photo_opencv/logo.jpg");
	Mat mask = imread("E://photo_opencv/logo.jpg", 0);
	Mat imageROI = img(Rect(0, 0, logo.cols, logo.rows));
	logo.copyTo(imageROI, mask);
	imwrite("maks.jpg", img);
}

void SplitGet()
{
	Mat srcImage;
	Mat logoImage;
	vector<Mat>channels;
	Mat  imageBlueChannel;

	logoImage = imread("E://photo_opencv/logo.jpg", 0);
	srcImage = imread("E://photo_opencv/dota.jpg");

	//【2】把一个3通道图像转换成3个单通道图像  
	split(srcImage, channels);//分离色彩通道  
	imwrite("Blue.jpg", channels[0]);
	imwrite("Green.jpg", channels[1]);
	imwrite("Red.jpg", channels[2]);

	imageBlueChannel = channels.at(0);
	//【4】将原图的蓝色通道的（500,250）坐标处右下方的一块区域和logo图进行加权操作，将得到的混合结果存到imageBlueChannel中  
	addWeighted(imageBlueChannel(Rect(0, 0, logoImage.cols, logoImage.rows)), 1.0,
		logoImage, 0.5, 0, imageBlueChannel(Rect(0, 0, logoImage.cols, logoImage.rows)));

	//【5】将三个单通道重新合并成一个三通道  
	merge(channels, srcImage);

	//【6】显示效果图  
	//namedWindow("<1>游戏原画+logo蓝色通道 by浅墨");
	// imshow("<1>游戏原画+logo蓝色通道 by浅墨", srcImage);
	imwrite("happen.jpg", srcImage);
}

void creatTrack()
{
	Mat imme = imread("E://photo_opencv/girl.jpg", 1);

}