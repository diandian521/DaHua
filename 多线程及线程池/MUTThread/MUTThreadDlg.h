
// MUTThreadDlg.h : ͷ�ļ�
//

#pragma once
#include "resource.h"
#include "../DICThreadPool/DICTaskObject.h"
#include "../DICThreadPool/DICThreadPool.h"	
#include "../WorkerThread/TrackerMonitor.h"
#include "../WorkerThread/TrackerTask.h"
#include "afxwin.h"

#include <opencv2/highgui.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include<highgui.h>
#include<cv.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/core/mat.hpp>
#include <opencv.hpp>
#include <opencv/cxcore.hpp>

#include "vector"
#include "map"

using namespace std;
using namespace cv;
// 
// typedef struct ThreadParamAdress
// {
// // 	DWORD CameraID;
// // 	DWORD HumanID;
// // 	Mat imaMat;
// // 	CRect initRect;
// };

// typedef struct TrackerInfo
// {
// 	DWORD   HunaID; //׷�ٶ���ID
// 	CString CamName; //�����������
// 	CRect   initRect; //������ʼ����
// 	Mat     img; //��ǰͼ��
// 	CRect   resRect; //׷�ٽ��
// 	BOOL    bIsTracked; //�Ƿ��Ѿ���ʼ׷��
// 	BOOL    bIsLost; //�Ƿ�ʧȥ׷��Ŀ��
// };
// static map<DWORD, TrackerInfo> TrackerMap;//׷������

// CMUTThreadDlg �Ի���
class CMUTThreadDlg : public CDialogEx
{
// ����
public:
	CMUTThreadDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MUTTHREAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	//�̳߳ض���
	CDICThreadPool m_ThreadPool;
	//��Ϣ
	CString m_InfoDis;
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonThreadPoolCreat();
	
	afx_msg void OnBnClickedButtonThreadPoolClose();
	afx_msg void OnBnClickedButtonNewTask();
	afx_msg void OnBnClickedButtonBreak();
	afx_msg void OnBnClickedButtonNewTask2();
	afx_msg void OnBnClickedButtonNewTask3();
};
