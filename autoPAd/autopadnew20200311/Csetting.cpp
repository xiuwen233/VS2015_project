// Csetting.cpp : 实现文件
//

#include "stdafx.h"
#include "NewAutoPat.h"
#include "Csetting.h"
#include "afxdialogex.h"


// Csetting 对话框

IMPLEMENT_DYNAMIC(Csetting, CDialogEx)

Csetting::Csetting(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

Csetting::~Csetting()
{
}

void Csetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Csetting, CDialogEx)
END_MESSAGE_MAP()


// Csetting 消息处理程序
