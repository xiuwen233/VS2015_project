#pragma once
#include"opecvinclude.h"

//ʮ���ּ��������ֵ���㷨  ��Ȼ�ӽ������˵����һ��ֵ �������HIST����
int Get1DMaxEntropyThreshold(int HistGram[256]);
int GetHuangFuzzyThreshold(int HistGram[256]);
int GetOSTUThreshold(int HistGram[256]);