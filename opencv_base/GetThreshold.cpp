#include"GetThreshold.h"

// ���㷨����Ϣ�����صĸ������뵽ͼ���У�ͨ��������ֵ�ָ���������صĺ����ж���ֵ�Ƿ�Ϊ�����ֵ��
int Get1DMaxEntropyThreshold(int HistGram[256])
{
	int  X, Y, Amount = 0;
	double HistGramD[256];
	double SumIntegral, EntropyBack, EntropyFore, MaxEntropy;
	int MinValue = 255, MaxValue = 0;
	int Threshold = 0;

	for (MinValue = 0; MinValue < 256 && HistGram[MinValue] == 0; MinValue++);
	for (MaxValue = 255; MaxValue > MinValue && HistGram[MinValue] == 0; MaxValue--);
	if (MaxValue == MinValue) return MaxValue;          // ͼ����ֻ��һ����ɫ             
	if (MinValue + 1 == MaxValue) return MinValue;      // ͼ����ֻ�ж�����ɫ

	for (Y = MinValue; Y <= MaxValue; Y++) Amount += HistGram[Y];        //  ��������

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

//  ���㷨��1979�����ձ��������ģ���Ҫ��˼����ȡĳ����ֵ��ʹ��ǰ���ͱ����������䷽�����matlab�е�graythresh�����Ը��㷨Ϊԭ��ִ�е�
int GetOSTUThreshold(int HistGram[256])
{
	int X, Y, Amount = 0;
	int PixelBack = 0, PixelFore = 0, PixelIntegralBack = 0, PixelIntegralFore = 0, PixelIntegral = 0;
	double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma;              // ��䷽��;
	int MinValue, MaxValue;
	int Threshold = 0;

	for (MinValue = 0; MinValue < 256 && HistGram[MinValue] == 0; MinValue++);
	for (MaxValue = 255; MaxValue > MinValue && HistGram[MinValue] == 0; MaxValue--);
	if (MaxValue == MinValue) return MaxValue;          // ͼ����ֻ��һ����ɫ             
	if (MinValue + 1 == MaxValue) return MinValue;      // ͼ����ֻ�ж�����ɫ

	for (Y = MinValue; Y <= MaxValue; Y++) Amount += HistGram[Y];        //  ��������

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
	//   �ҵ���һ�������һ����0��ɫ��ֵ
	for (First = 0; First < 256 && HistGram[First] == 0; First++);
	for (Last = 256 - 1; Last > First && HistGram[Last] == 0; Last--);
	if (First == Last) return First;                // ͼ����ֻ��һ����ɫ
	if (First + 1 == Last) return First;            // ͼ����ֻ�ж�����ɫ
	//cout << "for over" << "  get First " << First << endl;
	//cout << "get Last " << Last << endl;
	// �����ۼ�ֱ��ͼ�Լ���Ӧ�Ĵ�Ȩ�ص��ۼ�ֱ��ͼ
	int  S[256];
	long  W[256];             // �����ش�ͼ��������ı������ݿ��ܻᳬ��int�ı�ʾ��Χ�����Կ�����long����������
	S[0] = HistGram[0];
	for (Y = First > 1 ? First : 1; Y <= Last; Y++)
	{
		S[Y] = S[Y - 1] + HistGram[Y];
		W[Y] = W[Y - 1] + Y * HistGram[Y];
	}
	//cout << "arry over" << endl;
	// ������ʽ��4������6�����õĲ��ұ�
	double Smu[256];
	for (Y = 1; Y < (Last + 1 - First); Y++)
	{
		double mu = 1 / (1 + (double)Y / (Last - First));               // ��ʽ��4��
		Smu[Y] = -mu * log(mu) - (1 - mu) * log(1 - mu);      // ��ʽ��6��
	}
	//cout << "Smu over" << endl;
	// �������������ֵ
	for (Y = First; Y <= Last; Y++)
	{
		Entropy = 0;
		int mu = (int)round((double)W[Y] / S[Y]);             // ��ʽ17
		for (X = First; X <= Y; X++)
			Entropy += Smu[abs(X - mu)] * HistGram[X];
		mu = (int)round((double)(W[Last] - W[Y]) / (S[Last] - S[Y]));  // ��ʽ18
		for (X = Y + 1; X <= Last; X++)
			Entropy += Smu[abs(X - mu)] * HistGram[X];       // ��ʽ8
		if (BestEntropy > Entropy)
		{
			BestEntropy = Entropy;      // ȡ��С�ش�Ϊ�����ֵ
			Threshold = Y;
		}
	}
	return Threshold;
}