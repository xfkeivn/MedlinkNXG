
// MedlinkNXGDlg.h: 头文件
//

#pragma once
#include <memory>
using namespace std;
class VideoSourceMgr;
// CMedlinkNXGDlg 对话框
class CMedlinkNXGDlg : public CDialogEx
{
// 构造
public:
	CMedlinkNXGDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MEDLINKNXG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;
	VideoSourceMgr *pWndmgr;
	
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT id, int w, int h);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
};
