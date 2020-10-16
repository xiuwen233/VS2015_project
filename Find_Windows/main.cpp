#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

int main(int argc, char** argv)
{
	std::vector<RECT> vecScreenList;
	DISPLAY_DEVICE stDispDev = { 0 };
	BOOL bRet = FALSE;
	int nDevNum = 0;
	int nScreenCount = 0;
	printf("┌──┬─────────┬─────────────────┬──────┬────┬─────┬───────────────────────────────────────────────────┐\n");
	printf("│%-4s│ %-17s│ %-33s│ %s │ %-7s│ %-9s│ %-101s│\n",
		"Num", "DeviceName", "DeviceString", "StateFlags", "Active", "DeviceID", "DeviceKey");
	while (true)
	{
		memset(&stDispDev, 0, sizeof(DISPLAY_DEVICE));
		stDispDev.cb = sizeof(DISPLAY_DEVICE);
		bRet = EnumDisplayDevices(NULL, nDevNum, &stDispDev, EDD_GET_DEVICE_INTERFACE_NAME);
		if (bRet == FALSE)
		{
			break;
		}
		printf("├──┼─────────┼─────────────────┼──────┼────┼─────┼───────────────────────────────────────────────────┤\n");
		printf("│%-4d│ %-17s│ %-33s│ 0x%08x │ %-7s│ %-9s│ %-101s│\n",
			nDevNum, stDispDev.DeviceName, stDispDev.DeviceString, stDispDev.StateFlags,
			((DISPLAY_DEVICE_ACTIVE & stDispDev.StateFlags) ? "Yes" : "No"),
			stDispDev.DeviceID, stDispDev.DeviceKey);
		if (DISPLAY_DEVICE_ACTIVE & stDispDev.StateFlags)
		{
			DEVMODE stDevMode = { 0 };
			stDevMode.dmSize = sizeof(DEVMODE);
			bRet = EnumDisplaySettings(stDispDev.DeviceName, ENUM_CURRENT_SETTINGS, &stDevMode);
			if (bRet)
			{
				RECT rect = { 0 };
				rect.left = stDevMode.dmPosition.x;
				rect.top = stDevMode.dmPosition.y;
				rect.right = stDevMode.dmPosition.x + stDevMode.dmPelsWidth;
				rect.bottom = stDevMode.dmPosition.y + stDevMode.dmPelsHeight;
				vecScreenList.push_back(rect);
			}
		}
		nDevNum++;
	}
	printf("└──┴─────────┴─────────────────┴──────┴────┴─────┴───────────────────────────────────────────────────┘\n");
	auto lamda = [](RECT a, RECT b)
	{
		if (a.left < b.left)
		{
			return true;
		}
		else if (a.top < b.top)
		{
			return true;
		}
		else
		{
			return false;
		}
	};
	// 排序
	std::sort(vecScreenList.begin(), vecScreenList.end(), lamda);

	printf("\n\n┌────┬──────┬───────────┬───┐\n");
	printf("│%s│ %-11s│ %-20s │ %-4s │\n", "屏幕编号", "分辨率", "坐标", "主屏");
	nScreenCount = 0;
	for (auto& itr : vecScreenList)
	{
		printf("├────┼──────┼───────────┼───┤\n");
		printf("│%-8d│ %-4dx%-4d  │ %-4d,%-4d,%-4d,%-4d  │ %-4s │\n", ++nScreenCount,
			itr.right - itr.left, itr.bottom - itr.top, itr.left, itr.top, itr.right, itr.bottom,
			(((itr.left == 0) && (itr.top == 0)) ? "是" : "否"));
	}
	printf("└────┴──────┴───────────┴───┘\n");
	getchar();
	return 0;
}
