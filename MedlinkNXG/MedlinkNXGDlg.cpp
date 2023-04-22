#include "pch.h"
#include "framework.h"
#include "MedlinkNXG.h"
#include "MedlinkNXGDlg.h"
#include "afxdialogex.h"
#include "VideoSourceMgr.h"
#include "spdlog/sinks/rotating_file_sink.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

auto max_size = 1048576 * 20;
auto max_files = 20;
auto logger = spdlog::rotating_logger_mt("medlinknxg", "medlinknxg.log", max_size, max_files);
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()



CMedlinkNXGDlg::CMedlinkNXGDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MEDLINKNXG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMedlinkNXGDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMedlinkNXGDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


BOOL CMedlinkNXGDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		

	logger->set_level(spdlog::level::err);
	logger->info("#Start the application");
	pWndmgr =  new VideoSourceMgr(this);
	pWndmgr->init_custom_capture_video_source();

	
	return TRUE;  
}

void CMedlinkNXGDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}



void CMedlinkNXGDlg::OnPaint()
{







	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);


	}
	else
	{
		CPaintDC dc(this);
		pWndmgr->draw_main_rect(dc);

	}

	CDialogEx::OnPaint();

}


HCURSOR CMedlinkNXGDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMedlinkNXGDlg::OnClose()
{
	
	delete pWndmgr;
	CDialogEx::OnClose();
}

void CMedlinkNXGDlg::OnSize(UINT id, int w, int h)
{
	CDialogEx::OnSize(id,w,h);
	if (pWndmgr!=nullptr)
	pWndmgr->onParentWndSize();
	Invalidate();
}


