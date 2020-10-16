#include "stdafx.h"

#include "HiKanAPI.h"


HikAPI::HikAPI()
{
	m_hDevHandle = nullptr;
	matData = nullptr;
	rect_x = rect_y = width = height = 0;
}

HikAPI::~HikAPI()
{
	if (m_hDevHandle)
	{
		MV_CC_DestroyHandle(m_hDevHandle);
		m_hDevHandle = nullptr;
	}
}


int HikAPI::EnumDevices(MV_CC_DEVICE_INFO_LIST* pstDevList)
{
	int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, pstDevList);
	if (MV_OK != nRet)
	{
		return nRet;
	}

	return MV_OK;
}


// ch:���豸 | en:Open Device
int     HikAPI::Open(MV_CC_DEVICE_INFO* pstDeviceInfo)
{
	if (NULL == pstDeviceInfo)
	{
		return MV_E_PARAMETER;
	}

	int nRet = MV_OK;
	if (m_hDevHandle == NULL)
	{
		nRet = MV_CC_CreateHandle(&m_hDevHandle, pstDeviceInfo);
		if (MV_OK != nRet)
		{
			return nRet;
		}
	}

	nRet = MV_CC_OpenDevice(m_hDevHandle);
	if (MV_OK != nRet)
	{
		MV_CC_DestroyHandle(m_hDevHandle);
		m_hDevHandle = NULL;

		return nRet;
	}

	MVCC_INTVALUE stParam;
	nRet = MV_CC_SetIntValue(m_hDevHandle, "OffsetX", 0);
	nRet = MV_CC_SetIntValue(m_hDevHandle, "OffsetY", 0);
	nRet = MV_CC_GetIntValue(m_hDevHandle, "Width", &stParam);
	nRet = MV_CC_SetIntValue(m_hDevHandle, "Width", stParam.nMax);
	nRet = MV_CC_GetIntValue(m_hDevHandle, "Height", &stParam);
	nRet = MV_CC_SetIntValue(m_hDevHandle, "Height", stParam.nMax);

	if (width)
		nRet = MV_CC_SetIntValue(m_hDevHandle, "Width", width);
	//TRACE("set Width:%d ret:%x\n", width, nRet);

	if (height)
		nRet = MV_CC_SetIntValue(m_hDevHandle, "Height", height);
	//TRACE("set Height:%d ret:%x\n", height, nRet);

	nRet = MV_CC_SetIntValue(m_hDevHandle, "OffsetX", rect_x);
	//TRACE("set OffsetX:%d ret:%x\n", rect_x, nRet);
	nRet = MV_CC_SetIntValue(m_hDevHandle, "OffsetY", rect_y);

	nRet = MV_CC_SetEnumValue(m_hDevHandle, "TriggerMode", 0);
	if (MV_OK != nRet) {
		//TRACE("Set TriggerMode failed!");
	}


	// Get payload size
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	nRet = MV_CC_GetIntValue(m_hDevHandle, "PayloadSize", &stParam);
	if (MV_OK != nRet)
	{
		//TRACE("Get PayloadSize fail! nRet [0x%x]\n", nRet);
		MV_CC_DestroyHandle(m_hDevHandle);
		m_hDevHandle = nullptr;
		return false;
	}

	g_nPayloadSize = stParam.nCurValue;
	matData = reinterpret_cast<unsigned char *>(malloc(sizeof(unsigned char) * (g_nPayloadSize)));
	if (matData == nullptr) {
		//TRACE("Allocate memory failed.\n");
		MV_CC_DestroyHandle(m_hDevHandle);
		m_hDevHandle = nullptr;
		return false;
	}

	return MV_OK;
}


// ch:�ر��豸 | en:Close Device
int  HikAPI::Close()
{
	int nRet = MV_OK;

	if (nullptr == m_hDevHandle)
	{
		return MV_E_PARAMETER;
	}

	MV_CC_CloseDevice(m_hDevHandle);
	nRet = MV_CC_DestroyHandle(m_hDevHandle);
	m_hDevHandle = nullptr;
	//TRACE("close device.\n");
	return nRet;
}


// ch:����ץͼ | en:Start Grabbing
int  HikAPI::StartGrabbing()
{
	return MV_CC_StartGrabbing(m_hDevHandle);
}


// ch:ֹͣץͼ | en:Stop Grabbing
int     HikAPI::StopGrabbing()
{
	return MV_CC_StopGrabbing(m_hDevHandle);
}


int     HikAPI::RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
	if (nullptr == pRgbData)
	{
		return MV_E_PARAMETER;
	}

	for (unsigned int j = 0; j < nHeight; j++)
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}

	return MV_OK;
}

// convert data stream in Mat format
int HikAPI::Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData, cv::Mat &src)
{
	cv::Mat srcImage;
	if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
	{
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
	}
	else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
	{
		RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
	}
	else
	{
		printf("unsupported pixel format\n");
		return -1;
	}

	if (nullptr == srcImage.data)
	{
		return -1;
	}

	//save converted image in a local file
	src = srcImage.clone();

	srcImage.release();
	return MV_OK;
}


int  HikAPI::GetOneFrameTimeout(cv::Mat &frame, unsigned int nMsec)
{
	if (nullptr == matData)
	{
		return MV_E_PARAMETER;
	}

	int nRet = MV_OK;
	MV_FRAME_OUT_INFO_EX pFrameInfo = { 0 };
	nRet = MV_CC_GetOneFrameTimeout(m_hDevHandle, matData, g_nPayloadSize, &pFrameInfo, nMsec);
	if (MV_OK != nRet)
	{
		return nRet;
	}

	//    *pnDataLen = pFrameInfo->nFrameLen;
	nRet = Convert2Mat(&pFrameInfo, matData, frame);

	return nRet;
}


// ch:������ʾ���ھ�� | en:Set Display Window Handle
int     HikAPI::Display(void* hWnd)
{
	return MV_CC_Display(m_hDevHandle, hWnd);
}


int HikAPI::SaveImage(MV_SAVE_IMAGE_PARAM_EX* pstParam)
{
	if (nullptr == pstParam)
	{
		return MV_E_PARAMETER;
	}

	return MV_CC_SaveImageEx2(m_hDevHandle, pstParam);
}

// ch:ע��ͼ�����ݻص� | en:Register Image Data CallBack
int HikAPI::RegisterImageCallBack(void(__stdcall* cbOutput)(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo,
	void* pUser), void* pUser)
{
	return MV_CC_RegisterImageCallBackEx(m_hDevHandle, cbOutput, pUser);
}


// ch:ע����Ϣ�쳣�ص� | en:Register Message Exception CallBack
int     HikAPI::RegisterExceptionCallBack(void(__stdcall* cbException)(unsigned int nMsgType, void* pUser), void* pUser)
{
	return MV_CC_RegisterExceptionCallBack(m_hDevHandle, cbException, pUser);
}


// ch:��ȡInt�Ͳ������� Width��Height����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Int type parameters, such as Width and Height, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::GetIntValue(IN const char* strKey, OUT unsigned int *pnValue)
{
	if (nullptr == strKey || nullptr == pnValue)
	{
		return MV_E_PARAMETER;
	}

	MVCC_INTVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	int nRet = MV_CC_GetIntValue(m_hDevHandle, strKey, &stParam);
	if (MV_OK != nRet)
	{
		return nRet;
	}

	*pnValue = stParam.nCurValue;

	return MV_OK;
}


// ch:����Int�Ͳ������� Width��Height����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Set Int type parameters, such as Width and Height, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::SetIntValue(IN const char* strKey, IN unsigned int nValue)
{
	if (nullptr == strKey)
	{
		return MV_E_PARAMETER;
	}

	return MV_CC_SetIntValue(m_hDevHandle, strKey, nValue);
}


// ch:��ȡFloat�Ͳ������� ExposureTime��Gain����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Float type parameters, such as ExposureTime and Gain, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::GetFloatValue(IN const char* strKey, OUT float *pfValue)
{
	if (nullptr == strKey || nullptr == pfValue)
	{
		return MV_E_PARAMETER;
	}

	MVCC_FLOATVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_FLOATVALUE));
	int nRet = MV_CC_GetFloatValue(m_hDevHandle, strKey, &stParam);
	if (MV_OK != nRet)
	{
		return nRet;
	}

	*pfValue = stParam.fCurValue;

	return MV_OK;
}


// ch:����Float�Ͳ������� ExposureTime��Gain����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Set Float type parameters, such as ExposureTime and Gain, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::SetFloatValue(IN const char* strKey, IN float fValue)
{
	if (nullptr == strKey)
	{
		return MV_E_PARAMETER;
	}

	return MV_CC_SetFloatValue(m_hDevHandle, strKey, fValue);
}


// ch:��ȡEnum�Ͳ������� PixelFormat����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Enum type parameters, such as PixelFormat, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::GetEnumValue(IN const char* strKey, OUT unsigned int *pnValue)
{
	if (nullptr == strKey || nullptr == pnValue)
	{
		return MV_E_PARAMETER;
	}

	MVCC_ENUMVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_ENUMVALUE));
	int nRet = MV_CC_GetEnumValue(m_hDevHandle, strKey, &stParam);
	if (MV_OK != nRet)
	{
		return nRet;
	}

	*pnValue = stParam.nCurValue;

	return MV_OK;
}


// ch:����Enum�Ͳ������� PixelFormat����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Set Enum type parameters, such as PixelFormat, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::SetEnumValue(IN const char* strKey, IN unsigned int nValue)
{
	if (nullptr == strKey)
	{
		return MV_E_PARAMETER;
	}

	return MV_CC_SetEnumValue(m_hDevHandle, strKey, nValue);
}


// ch:��ȡBool�Ͳ������� ReverseX����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Bool type parameters, such as ReverseX, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::GetBoolValue(IN const char* strKey, OUT bool *pbValue)
{
	if (nullptr == strKey || nullptr == pbValue)
	{
		return MV_E_PARAMETER;
	}

	return MV_CC_GetBoolValue(m_hDevHandle, strKey, pbValue);
}


// ch:����Bool�Ͳ������� ReverseX����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Set Bool type parameters, such as ReverseX, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::SetBoolValue(IN const char* strKey, IN bool bValue)
{
	if (NULL == strKey)
	{
		return MV_E_PARAMETER;
	}

	return MV_CC_SetBoolValue(m_hDevHandle, strKey, bValue);
}


// ch:��ȡString�Ͳ������� DeviceUserID����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�UserSetSave
// en:Get String type parameters, such as DeviceUserID, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::GetStringValue(IN const char* strKey, IN OUT char* strValue, IN unsigned int nSize)
{
	if (nullptr == strKey || nullptr == strValue)
	{
		return MV_E_PARAMETER;
	}

	MVCC_STRINGVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_STRINGVALUE));
	int nRet = MV_CC_GetStringValue(m_hDevHandle, strKey, &stParam);
	if (MV_OK != nRet)
	{
		return nRet;
	}

	strcpy_s(strValue, nSize, stParam.chCurValue);

	return MV_OK;
}


// ch:����String�Ͳ������� DeviceUserID����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�UserSetSave
// en:Set String type parameters, such as DeviceUserID, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::SetStringValue(IN const char* strKey, IN const char* strValue)
{
	if (nullptr == strKey)
	{
		return MV_E_PARAMETER;
	}

	return MV_CC_SetStringValue(m_hDevHandle, strKey, strValue);
}


// ch:ִ��һ��Command������� UserSetSave����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Execute Command once, such as UserSetSave, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     HikAPI::CommandExecute(IN const char* strKey)
{
	if (nullptr == strKey)
	{
		return MV_E_PARAMETER;
	}

	return MV_CC_SetCommandValue(m_hDevHandle, strKey);
}

int HikAPI::GetOptimalPacketSize()
{
	return MV_CC_GetOptimalPacketSize(m_hDevHandle);
}


int     HikAPI::GetAllMatchInfo(OUT unsigned int *nLostFrame, OUT unsigned int *nFrameCount)
{
	MV_CC_DEVICE_INFO stDevInfo = { 0 };
	int nRet = MV_CC_GetDeviceInfo(m_hDevHandle, &stDevInfo); //�ýӿ�u3�ݲ�֧�֣��ô˷�ʽ���ж�u3����GigE
	if (MV_E_SUPPORT == nRet)
	{
		stDevInfo.nTLayerType = MV_USB_DEVICE;
	}

	if (MV_GIGE_DEVICE == stDevInfo.nTLayerType)
	{
		MV_ALL_MATCH_INFO struMatchInfo = { 0 };
		MV_MATCH_INFO_NET_DETECT stMatchInfoNetDetect;
		struMatchInfo.pInfo = &stMatchInfoNetDetect;

		struMatchInfo.nType = MV_MATCH_TYPE_NET_DETECT; // ch:���������Ͷ�����Ϣ | en:Net flow and lsot packet information
		memset(struMatchInfo.pInfo, 0, sizeof(MV_MATCH_INFO_NET_DETECT));
		struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_NET_DETECT);

		nRet = MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
		if (MV_OK != nRet)
		{
			return nRet;
		}

		MV_MATCH_INFO_NET_DETECT *pInfo = (MV_MATCH_INFO_NET_DETECT*)struMatchInfo.pInfo;
		*nFrameCount = pInfo->nNetRecvFrameCount;
		*nLostFrame = stMatchInfoNetDetect.nLostFrameCount;
	}
	else if (MV_USB_DEVICE == stDevInfo.nTLayerType)
	{
		MV_ALL_MATCH_INFO struMatchInfo = { 0 };
		MV_MATCH_INFO_USB_DETECT stMatchInfoNetDetect;
		struMatchInfo.pInfo = &stMatchInfoNetDetect;

		struMatchInfo.nType = MV_MATCH_TYPE_USB_DETECT; // ch:���������Ͷ�����Ϣ | en:Net flow and lsot packet information
		memset(struMatchInfo.pInfo, 0, sizeof(MV_MATCH_INFO_USB_DETECT));
		struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_USB_DETECT);

		nRet = MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
		if (MV_OK != nRet)
		{
			return nRet;
		}

		MV_MATCH_INFO_NET_DETECT *pInfo = (MV_MATCH_INFO_NET_DETECT*)struMatchInfo.pInfo;
		*nFrameCount = pInfo->nNetRecvFrameCount;
		*nLostFrame = stMatchInfoNetDetect.nErrorFrameCount;
	}

	return MV_OK;
}


int HikAPI::SetCameraArea(unsigned int offx, unsigned int offy, unsigned int Width, unsigned int Height)
{
	rect_x = offx;
	rect_y = offy;
	width = Width;
	height = Height;
	return 0;
}


int HikAPI::ChangeCameraArea(unsigned int offx, unsigned int offy, unsigned int Width, unsigned int Height)
{
	int nRet;
	MVCC_INTVALUE stParam;
	rect_x = offx;
	rect_y = offy;
	width = Width;
	height = Height;

	nRet = MV_CC_SetIntValue(m_hDevHandle, "OffsetX", 0);
	nRet = MV_CC_SetIntValue(m_hDevHandle, "OffsetY", 0);
	nRet = MV_CC_GetIntValue(m_hDevHandle, "Width", &stParam);
	nRet = MV_CC_SetIntValue(m_hDevHandle, "Width", stParam.nMax);
	nRet = MV_CC_GetIntValue(m_hDevHandle, "Height", &stParam);
	nRet = MV_CC_SetIntValue(m_hDevHandle, "Height", stParam.nMax);

	if (width)
		nRet = MV_CC_SetIntValue(m_hDevHandle, "Width", width);
	//TRACE("set Width:%d ret:%x\n", width, nRet);

	if (height)
		nRet = MV_CC_SetIntValue(m_hDevHandle, "Height", height);
	//TRACE("set Height:%d ret:%x\n", height, nRet);

	nRet = MV_CC_SetIntValue(m_hDevHandle, "OffsetX", rect_x);
	//TRACE("set OffsetX:%d ret:%x\n", rect_x, nRet);
	nRet = MV_CC_SetIntValue(m_hDevHandle, "OffsetY", rect_y);
	//TRACE("set OffsetY:%d ret:%x\n", rect_y, nRet);


	// ch:���ô���ģʽΪoff || en:set trigger mode as off
	nRet = MV_CC_SetEnumValue(m_hDevHandle, "TriggerMode", 0);
	if (MV_OK != nRet) {
		//TRACE("Set TriggerMode failed!");
	}

	// Get payload size
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	nRet = MV_CC_GetIntValue(m_hDevHandle, "PayloadSize", &stParam);
	if (MV_OK != nRet)
	{
		//TRACE("Get PayloadSize fail! nRet [0x%x]\n", nRet);
		MV_CC_DestroyHandle(m_hDevHandle);
		m_hDevHandle = nullptr;
		return false;
	}

	g_nPayloadSize = stParam.nCurValue;
	if (matData != NULL)
	{
		free(matData);
	}
	matData = reinterpret_cast<unsigned char *>(malloc(sizeof(unsigned char) * (g_nPayloadSize)));
	if (matData == nullptr) {
		//TRACE("Allocate memory failed.\n");
		MV_CC_DestroyHandle(m_hDevHandle);
		m_hDevHandle = nullptr;
		return false;
	}
	return 0;
}


//int HikAPI::OpenMatdisplay()
//{
//	cv::Mat frame, out;
//	int ret;
//	int number;
//	int length = 0;
//	while (1)
//	{
//		ret = GetOneFrameTimeout(frame, 200);
//		if (ret || frame.empty())
//		{
//			AfxMessageBox("get mat failed what should i do ");
//			continue;
//		}
//
//		cv::imshow("source", frame);
//		number = cv::waitKey(10);
//		if (number == 's' || number == 'S')
//		{
//			cv::imwrite("number.jpg", frame);
//		}
//		else if (number == 'q' || number == 'Q')
//		{
//			break;
//		}
//	}
//
//	cv::destroyAllWindows();
//}


//int   HikAPI::opencvshow()
//{
//	thread t(OpenMatdisplay);
//	t.detach();
//	return 0;
//}