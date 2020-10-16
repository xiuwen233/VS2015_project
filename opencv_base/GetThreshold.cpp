#include"GetThreshold.h"

// 该算法把信息论中熵的概念引入到图像中，通过计算阈值分割后两部分熵的和来判断阈值是否为最佳阈值。
int Get1DMaxEntropyThreshold(int HistGram[256])
{
	int  X, Y, Amount = 0;
	double HistGramD[256];
	double SumIntegral, EntropyBack, EntropyFore, MaxEntropy;
	int MinValue = 255, MaxValue = 0;
	int Threshold = 0;

	for (MinValue = 0; MinValue < 256 && HistGram[MinValue] == 0; MinValue++);
	for (MaxValue = 255; MaxValue > MinValue && HistGram[MinValue] == 0; MaxValue--);
	if (MaxValue == MinValue) return MaxValue;          // 图像中只有一个颜色             
	if (MinValue + 1 == MaxValue) return MinValue;      // 图像中只有二个颜色

	for (Y = MinValue; Y <= MaxValue; Y++) Amount += HistGram[Y];        //  像素总数

	for (Y = MinValue; Y <= MaxValue; Y++)   HistGramD[Y] = (double)HistGram[Y] / Amount + 1e-17;

	MaxEntropy = MinValue; ;
	for (Y = MinValue + 1; Y < MaxValue; Y++)
	{
		SumIntegral = 0;
		for (X = MinValue; X <= Y; X++) SumIntegral += HistGramD[X];
		EntropyBack = 0;
		for (X = MinValue; X <= Y; X++) EntropyBack += (-HistGramD[X] / SumIntegral * log(HistGramD[X] / SumIntegral));
		EntropyFore = 0;
		for (X = Y + 1; X <= MaxValue; X++) EntropyFore += (-HistGramD[X] / (1 - SumIntegral) * log(HistGramD[X] / (1 - SumIntegral)));
		if (MaxEntropy < EntropyBack + EntropyFore)
		{
			Threshold = Y;
			MaxEntropy = EntropyBack + EntropyFore;
		}
	}
	return Threshold;
}

//  该算法是1979年由日本大津提出的，主要是思想是取某个阈值，使得前景和背景两类的类间方差最大，matlab中的graythresh即是以该算法为原理执行的
int GetOSTUThreshold(int HistGram[256])
{
	int X, Y, Amount = 0;
	int PixelBack = 0, PixelFore = 0, PixelIntegralBack = 0, PixelIntegralFore = 0, PixelIntegral = 0;
	double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma;              // 类间方差;
	int MinValue, MaxValue;
	int Threshold = 0;

	for (MinValue = 0; MinValue < 256 && HistGram[MinValue] == 0; MinValue++);
	for (MaxValue = 255; MaxValue > MinValue && HistGram[MinValue] == 0; MaxValue--);
	if (MaxValue == MinValue) return MaxValue;          // 图像中只有一个颜色             
	if (MinValue + 1 == MaxValue) return MinValue;      // 图像中只有二个颜色

	for (Y = MinValue; Y <= MaxValue; Y++) Amount += HistGram[Y];        //  像素总数

	PixelIntegral = 0;
	for (Y = MinValue; Y <= MaxValue; Y++) PixelIntegral += HistGram[Y] * Y;
	SigmaB = -1;
	for (Y = MinValue; Y < MaxValue; Y++)
	{
		PixelBack = PixelBack + HistGram[Y];
		PixelFore = Amount - PixelBack;
		OmegaBack = (double)PixelBack / Amount;
		OmegaFore = (double)PixelFore / Amount;
		PixelIntegralBack += HistGram[Y] * Y;
		PixelIntegralFore = PixelIntegral - PixelIntegralBack;
		MicroBack = (double)PixelIntegralBack / PixelBack;
		MicroFore = (double)PixelIntegralFore / PixelFore;
		Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);
		if (Sigma > SigmaB)
		{
			SigmaB = Sigma;
			Threshold = Y;
		}
	}
	return Threshold;
}

int GetHuangFuzzyThreshold(int HistGram[256])
{
	int X, Y;
	int First, Last;
	int Threshold = -1;
	double BestEntropy, Entropy;
	BestEntropy = DBL_MAX;
	//cout << "BestEntropy =  " << BestEntropy << endl;
	//   找到第一个和最后一个非0的色阶值
	for (First = 0; First < 256 && HistGram[First] == 0; First++);
	for (Last = 256 - 1; Last > First && HistGram[Last] == 0; Last--);
	if (First == Last) return First;                // 图像中只有一个颜色
	if (First + 1 == Last) return First;            // 图像中只有二个颜色
	//cout << "for over" << "  get First " << First << endl;
	//cout << "get Last " << Last << endl;
	// 计算累计直方图以及对应的带权重的累计直方图
	int  S[256];
	long  W[256];             // 对于特大图，此数组的保存数据可能会超出int的表示范围，可以考虑用long类型来代替
	S[0] = HistGram[0];
	for (Y = First > 1 ? First : 1; Y <= Last; Y++)
	{
		S[Y] = S[Y - 1] + HistGram[Y];
		W[Y] = W[Y - 1] + Y * HistGram[Y];
	}
	//cout << "arry over" << endl;
	// 建立公式（4）及（6）所用的查找表
	double Smu[256];
	for (Y = 1; Y < (Last + 1 - First); Y++)
	{
		double mu = 1 / (1 + (double)Y / (Last - First));               // 公式（4）
		Smu[Y] = -mu * log(mu) - (1 - mu) * log(1 - mu);      // 公式（6）
	}
	//cout << "Smu over" << endl;
	// 迭代计算最佳阈值
	for (Y = First; Y <= Last; Y++)
	{
		Entropy = 0;
		int mu = (int)round((double)W[Y] / S[Y]);             // 公式17
		for (X = First; X <= Y; X++)
			Entropy += Smu[abs(X - mu)] * HistGram[X];
		mu = (int)round((double)(W[Last] - W[Y]) / (S[Last] - S[Y]));  // 公式18
		for (X = Y + 1; X <= Last; X++)
			Entropy += Smu[abs(X - mu)] * HistGram[X];       // 公式8
		if (BestEntropy > Entropy)
		{
			BestEntropy = Entropy;      // 取最小熵处为最佳阈值
			Threshold = Y;
		}
	}
	return Threshold;
}