#include<iostream>
#include"hist.h"

int getmenthod = 0;
int threshold_num = 160;
int element_number = 0;
int element_type = 0;

//上面的，先是用来检测芯片的
Point  corner_point[4];
//得到的矩形图


static void ContrastAndBright(int, void *)
{
	Mat m = Mat(800, 600, 0);
	imshow("【控制面板】", m);
}

void InitControlBoard()
{
	namedWindow("【控制面板】", 0);
	createTrackbar("模板匹配算法：", "【控制面板】", &getmenthod, 8, ContrastAndBright);
	createTrackbar("二值化阈值：", "【控制面板】", &threshold_num, 256, ContrastAndBright);
	createTrackbar("morgh", "【控制面板】", &element_number, 8, ContrastAndBright);
	createTrackbar("opengl", "【控制面板】", &element_type, 8, ContrastAndBright);
}

//检测文件是否存在
//if (_access("447_3.jpg", 0) != 0)
//{
//	printf("file is not exits \n");
//}
//else
//{
//	printf("file is exits \n");
//}


//void  test2()
//{
//	Mat out_img = imread("567.jpg");
//	ImageScanner scanner;
//	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
//	Mat imageGray;
//	cvtColor(out_img, imageGray, CV_RGB2GRAY);
//	int width = imageGray.cols;
//	int height = imageGray.rows;
//	uchar *raw = (uchar *)imageGray.data;
//	Image imageZbar(width, height, "Y800", raw, width * height);
//	scanner.scan(imageZbar); //扫描条码      
//	Image::SymbolIterator symbol = imageZbar.symbol_begin();
//
//	for (; symbol != imageZbar.symbol_end(); ++symbol)
//	{
//		cout << symbol->get_data() << endl;
//
//	}
//	imageZbar.set_data(NULL, 0);
//}



//   进行仿射变换  

int main()
{
  //  icotl()
	getchar();
	return 0;
}
