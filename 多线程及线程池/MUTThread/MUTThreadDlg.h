
// MUTThreadDlg.h : 头文件
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
// 	DWORD   HunaID; //追踪对象ID
// 	CString CamName; //所属相机名字
// 	CRect   initRect; //特征初始区域
// 	Mat     img; //当前图像
// 	CRect   resRect; //追踪结果
// 	BOOL    bIsTracked; //是否已经开始追踪
// 	BOOL    bIsLost; //是否失去追踪目标
// };
// static map<DWORD, TrackerInfo> TrackerMap;//追踪容器

// CMUTThreadDlg 对话框
class CMUTThreadDlg : public CDialogEx
{
// 构造
public:
	CMUTThreadDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MUTTHREAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	//线程池对象
	CDICThreadPool m_ThreadPool;
	//消息
	CString m_InfoDis;
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
