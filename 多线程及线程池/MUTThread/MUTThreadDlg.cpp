
// MUTThreadDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MUTThread.h"
#include "MUTThreadDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CMUTThreadDlg �Ի���

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


// CMUTThreadDlg ��Ϣ�������

BOOL CMUTThreadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	if (FALSE == m_ThreadPool.Create(POOL_SIZE, 20, 5, 1))
	{
		m_InfoDis = L"�̳߳ش���ʧ��";
	}
	else
	{
		m_InfoDis = L"�̳߳ش����ɹ�";
	}
	UpdateData(FALSE);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMUTThreadDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMUTThreadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMUTThreadDlg::OnBnClickedButtonThreadPoolCreat()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
// 	if (FALSE == m_ThreadPool.Create(POOL_SIZE, 20, 5, 1))
// 	{
// 		m_InfoDis = L"�̳߳ش���ʧ��";
// 	}
// 	else
// 	{
// 		m_InfoDis = L"�̳߳ش����ɹ�";
// 	}
// 	UpdateData(FALSE);
}

void CMUTThreadDlg::OnBnClickedButtonThreadPoolClose()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	m_ThreadPool.Destroy();
	m_InfoDis = L"�ر��̳߳�";
	UpdateData(FALSE);
}

void CMUTThreadDlg::OnBnClickedButtonNewTask()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
// 	pWorkerThread = new CWorkerThread;
// 	//pWorkerThread->SetParam(12,13);
// 	pWorkerThread->SetAutoDelete(TRUE);
// 	paramEE param = setPareamEE(1, 0, 0, true);
// 	pWorkerThread->SetParam(param);
// 	if (FALSE == m_ThreadPool.Run(pWorkerThread))
// 	{	
// 		m_InfoDis = L"��������ʧ��";
// 		UpdateData(FALSE);
// 	}
	//pWorkerThread->GetResult();
}


void CMUTThreadDlg::OnBnClickedButtonBreak()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//m_bIshold = FALSE;
	//pWorkerThread->setParam_Hold(false);
}


void CMUTThreadDlg::OnBnClickedButtonNewTask2()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
// 	CWorkerThread* pWorkerThread1;
// 	pWorkerThread1 = new CWorkerThread;
// 	//pWorkerThread->SetParam(12,13);
// 	pWorkerThread1->SetAutoDelete(TRUE);
// 	paramEE param = setPareamEE(2, 3, 0, true);
// 	pWorkerThread1->SetParam(param);
// 	if (FALSE == m_ThreadPool.Run(pWorkerThread1))
// 	{
// 		m_InfoDis = L"��������ʧ��";
// 	}	
}

void CMUTThreadDlg::OnBnClickedButtonNewTask3()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}
