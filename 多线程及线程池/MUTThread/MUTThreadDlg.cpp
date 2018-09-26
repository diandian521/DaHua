
// MUTThreadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MUTThread.h"
#include "MUTThreadDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMUTThreadDlg 对话框

CMUTThreadDlg::CMUTThreadDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMUTThreadDlg::IDD, pParent)
	, m_InfoDis(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//m_bIshold = TRUE;
}

void CMUTThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_INFO_DIS, m_InfoDis);
}

BEGIN_MESSAGE_MAP(CMUTThreadDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_THREAD_POOL_CREAT, &CMUTThreadDlg::OnBnClickedButtonThreadPoolCreat)
	ON_BN_CLICKED(IDC_BUTTON_THREAD_POOL_CLOSE, &CMUTThreadDlg::OnBnClickedButtonThreadPoolClose)
	ON_BN_CLICKED(IDC_BUTTON_NEW_TASK, &CMUTThreadDlg::OnBnClickedButtonNewTask)
	ON_BN_CLICKED(IDC_BUTTON_BREAK, &CMUTThreadDlg::OnBnClickedButtonBreak)
	ON_BN_CLICKED(IDC_BUTTON_NEW_TASK2, &CMUTThreadDlg::OnBnClickedButtonNewTask2)
	ON_BN_CLICKED(IDC_BUTTON_NEW_TASK3, &CMUTThreadDlg::OnBnClickedButtonNewTask3)
END_MESSAGE_MAP()


// CMUTThreadDlg 消息处理程序

BOOL CMUTThreadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	if (FALSE == m_ThreadPool.Create(POOL_SIZE, 20, 5, 1))
	{
		m_InfoDis = L"线程池创建失败";
	}
	else
	{
		m_InfoDis = L"线程池创建成功";
	}
	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMUTThreadDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMUTThreadDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMUTThreadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMUTThreadDlg::OnBnClickedButtonThreadPoolCreat()
{
	// TODO:  在此添加控件通知处理程序代码
// 	if (FALSE == m_ThreadPool.Create(POOL_SIZE, 20, 5, 1))
// 	{
// 		m_InfoDis = L"线程池创建失败";
// 	}
// 	else
// 	{
// 		m_InfoDis = L"线程池创建成功";
// 	}
// 	UpdateData(FALSE);
}

void CMUTThreadDlg::OnBnClickedButtonThreadPoolClose()
{
	// TODO:  在此添加控件通知处理程序代码
	m_ThreadPool.Destroy();
	m_InfoDis = L"关闭线程池";
	UpdateData(FALSE);
}

void CMUTThreadDlg::OnBnClickedButtonNewTask()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
// 	pWorkerThread = new CWorkerThread;
// 	//pWorkerThread->SetParam(12,13);
// 	pWorkerThread->SetAutoDelete(TRUE);
// 	paramEE param = setPareamEE(1, 0, 0, true);
// 	pWorkerThread->SetParam(param);
// 	if (FALSE == m_ThreadPool.Run(pWorkerThread))
// 	{	
// 		m_InfoDis = L"创建任务失败";
// 		UpdateData(FALSE);
// 	}
	//pWorkerThread->GetResult();
}


void CMUTThreadDlg::OnBnClickedButtonBreak()
{
	// TODO:  在此添加控件通知处理程序代码
	//m_bIshold = FALSE;
	//pWorkerThread->setParam_Hold(false);
}


void CMUTThreadDlg::OnBnClickedButtonNewTask2()
{
	// TODO:  在此添加控件通知处理程序代码
// 	CWorkerThread* pWorkerThread1;
// 	pWorkerThread1 = new CWorkerThread;
// 	//pWorkerThread->SetParam(12,13);
// 	pWorkerThread1->SetAutoDelete(TRUE);
// 	paramEE param = setPareamEE(2, 3, 0, true);
// 	pWorkerThread1->SetParam(param);
// 	if (FALSE == m_ThreadPool.Run(pWorkerThread1))
// 	{
// 		m_InfoDis = L"创建任务失败";
// 	}	
}

void CMUTThreadDlg::OnBnClickedButtonNewTask3()
{
	// TODO:  在此添加控件通知处理程序代码
}
