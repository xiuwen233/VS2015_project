#pragma once


// Csetting �Ի���

class Csetting : public CDialogEx
{
	DECLARE_DYNAMIC(Csetting)

public:
	Csetting(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~Csetting();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
