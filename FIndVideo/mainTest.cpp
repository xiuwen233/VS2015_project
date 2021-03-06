
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "windows.h"
#include "dshow.h"
#include <iostream>

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")

using namespace cv;
using namespace std;

int listDevices(vector<string>& list) {

	//COM Library Initialization
	//comInit();

	//if (!silent) DebugPrintOut("\nVIDEOINPUT SPY MODE!\n\n");


	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;
	CoInitialize(NULL);

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));


	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK) {

			printf("SETUP: Looking For Capture Devices\n");
			IMoniker *pMoniker = NULL;

			while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {

				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)) {
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}


				// Find the description or friendly name.
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"Description", &varName, 0);

				if (FAILED(hr)) hr = pPropBag->Read(L"FriendlyName", &varName, 0);

				if (SUCCEEDED(hr))
				{

					hr = pPropBag->Read(L"FriendlyName", &varName, 0);

					int count = 0;
					char tmp[255] = { 0 };
					//int maxLen = sizeof(deviceNames[0]) / sizeof(deviceNames[0][0]) - 2;
					while (varName.bstrVal[count] != 0x00 && count < 255)
					{
						tmp[count] = (char)varName.bstrVal[count];
						count++;
					}
					list.push_back(tmp);
					//deviceNames[deviceCounter][count] = 0;

					//if (!silent) DebugPrintOut("SETUP: %i) %s\n", deviceCounter, deviceNames[deviceCounter]);
				}

				pPropBag->Release();
				pPropBag = NULL;

				pMoniker->Release();
				pMoniker = NULL;

				deviceCounter++;
			}

			pDevEnum->Release();
			pDevEnum = NULL;

			pEnum->Release();
			pEnum = NULL;
		}

		//if (!silent) DebugPrintOut("SETUP: %i Device(s) found\n\n", deviceCounter);
	}

	//comUnInit();

	return deviceCounter;
}

int main()
{
	vector<string> list;
	listDevices(list);
	int capid0 = 0, capid1 = 0;
	cout << "dev_size =      " << list.size() << endl;
	for (int i = 0; i<list.size(); i++)
	{
		if (list[i] == "3D Camera")
			capid1 = i;
		if (list[i] == "USB2.0 HD UVC WebCam")
			capid0 = i;
		cout << "device lists:  " << list[i] <<"     i =   "<<i<< endl;
	}
	getchar();
	return 0;
}
