#pragma once
#include"opecvinclude.h"

//十三种计算最佳阈值的算法  当然从结果上来说还是一个值 输入的是HIST数组
int Get1DMaxEntropyThreshold(int HistGram[256]);
int GetHuangFuzzyThreshold(int HistGram[256]);
int GetOSTUThreshold(int HistGram[256]);