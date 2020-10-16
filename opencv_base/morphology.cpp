#include"morphology.h"

void morphology(Mat img)
{
	/*
	* MORPH_GRADIENT ��̬ѧ�ݶ����� ��������ͼ�븩��ͼ֮����ڱ�������ı�Ե������
	* MORPH_OPEN������ �ȸ�ʴ������
	* MORPH_CLOSE������ �������ٸ�ʴ
	* MORPH_TOPHAT��ñ ԭͼ���뿪����ͼ֮����ڷ�����ڽ�����һЩ�İ߿顣
	* MORPH_BLACKHAT��ñ ��������ԭͼ��֮����ڷ�����ڽ��㰵һЩ�İ߿顣
	* MORPH_ERODE ��ʴ
	* MORPH_DILATE ����
	*/
	Mat out;
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	//����: MORPH_RECT
	//������ : MORPH_CROSS
	//��Բ�� : MORPH_ELLIPSE
	morphologyEx(img, out, MORPH_GRADIENT, element);
	imwrite("GRADIENT.jpg", out);
	morphologyEx(img, out, MORPH_CLOSE, element);
	imwrite("CLOSE.jpg", out);
	morphologyEx(img, out, MORPH_TOPHAT, element);
	imwrite("TOPHAT.jpg", out);
	morphologyEx(img, out, MORPH_BLACKHAT, element);
	imwrite("BLACKHAT.jpg", out);
	morphologyEx(img, out, MORPH_ERODE, element);
	imwrite("ERODE.jpg", out);
	morphologyEx(img, out, MORPH_DILATE, element);
	imwrite("DILATE.jpg", out);
}