#include"wavefilter.h"

void wavefilter(Mat img)
{
	Mat out;
	boxFilter(img, img, -1, Size(5, 5));  //�����˲�
	blur(img, img, Size(5, 5)); //��ֵ�˲�
	GaussianBlur(img, out, Size(3, 3), 0, 0); //��˹�˲�

	medianBlur(img, out, 7);//��ֵ�˲� ������������ʾ�׾������Գߴ磬����ֵ�����Ǵ���1������ ������
	bilateralFilter(img, out, 25, 25 * 2, 25 / 2);  //˫���˲�
}