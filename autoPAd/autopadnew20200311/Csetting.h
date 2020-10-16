#pragma once


// Csetting 对话框

class Csetting : public CDialogEx
{
	DECLARE_DYNAMIC(Csetting)

public:
	Csetting(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~Csetting();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
