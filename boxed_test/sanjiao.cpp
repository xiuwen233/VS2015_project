#include <iostream>
using namespace std;
struct Point
{
	int x;
	int y;
};
int type(Point  A, Point B,Point M) {
	Point MA, MB;
	
	MA.x = A.x - M.x;
	MA.y = A.y - M.y;
	MB.x = B.x - M.x;
	MB.y = B.y - M.y;
	if ((MA.x*MB.y - MB.x) >= 0)
	{
		return 1;
	}
	else {
		return -1;
	}
}
bool sameSide(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
{

	double x = (x3 - x2)*(y0 - y2) - (x0 - x2)*(y3 - y2);
	double y = (x3 - x2)*(y1 - y2) - (x1 - x2)*(y3 - y2);

	return x*y >= 0;

}
bool pointInTriangle(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
{
	// 如果点（x0，y0）在三个半平面（三条线）的同一侧，则它位于（由这三条线组成）三角形中
	bool same_1 = sameSide(x0, y0, x1, y1, x2, y2, x3, y3);
	bool same_2 = sameSide(x0, y0, x2, y2, x1, y1, x3, y3);
	bool same_3 = sameSide(x0, y0, x3, y3, x1, y1, x2, y2);

	return same_1 && same_2 && same_3;

}
int main() {
	Point  A, B, C;
	A.x = 2;
	A.y = 2;
	B.x = 12;
	B.y = 2;
	C.x = 7;
	C.y =8;

	Point M;
	M.x = 1000;
	M.y =10;

	int m1, m2, m3;
	m1 = type(A, B, M);
	m2 = type(B, C, M);
	m3 = type(C, A, M);
	cout << m1 << "  " << m2 << "  " << m3 << endl;
	if ((m1 == m2) && (m1 == m3)) {
		cout <<" YES"<< endl;
	}
	else {
		cout << " NO" << endl;
	}

	system("pause");
	return 0;
}