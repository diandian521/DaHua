
#ifndef _TRACKER_MAP_DEF_
#define _TRACKER_MAP_DEF_
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

#include "../DICThreadPool/DICTaskObject.h"
#include "../DICThreadPool/DICThreadPool.h"	

#include <map>
#include <vector>

using namespace std;
using namespace cv;

typedef struct _TrackerInfo
{
	DWORD   HunaID; //׷�ٶ���ID
	CString CamName; //�����������
	CRect   initRect; //������ʼ����
	Mat     img; //��ǰͼ��
	BOOL    bIsTracked; //�Ƿ��Ѿ���ʼ׷��
	BOOL    bIsLost; //�Ƿ�ʧȥ׷��Ŀ��
	_TrackerInfo()
	{
		HunaID = 0;
		CamName = L"";
		bIsTracked = FALSE;
		bIsLost = FALSE;
	}
}TrackerInfo;

typedef struct _TrackingRes
{
	DWORD   HunaID; //׷�ٶ���ID
	CString CamName; //�����������
	cv::Rect resRect; //׷�ٽ����������Ϣ
	_TrackingRes()
	{
		HunaID = 0;
		CamName = L"";
		resRect = Rect(0, 0, 0, 0);
	}
}TrackingRes;

inline TrackingRes InTrackingRes(DWORD   HunaID, CString CamName, cv::Rect rect)
{
	TrackingRes res;
	res.CamName = CamName;
	res.HunaID = HunaID;
	res.resRect = rect;
	return res;
}

class CTrackerMap
{
public:
	CTrackerMap();
	~CTrackerMap();
	static map<DWORD, TrackerInfo> _TrackerMap;//������IDΪ��ֵ��׷������map
	static CRITICAL_SECTION  _mapToTrackerCriticalSection;//map��Դ�ٽ���

	static BOOL _bIsMonitor;//�Ƿ�ɼ�ʶ��־
	static CDICThreadPool _ThreadPool;//�̳߳ض���

	static map<CString, Mat> _CamImgMap;//�����Ϊ��ֵ��ͼ��Map
	static SRWLOCK _CamImgMapSRWLOCK; //ͼ��Map��Դ��д��

	///////////////////////׷��////////////////////////////
	static DWORD _CurCreatTaskID;// ��ǰ�¿������ID(׷�������ID)
	static CRITICAL_SECTION  _curTsIDCriticalSection;//CurID��Դ�ٽ���

	static map<CString, vector<TrackingRes>*> _trackingResMap;//�������Ϊ��ֵ��׷�ٽ��map
	static SRWLOCK _trackingResMapSRWLOCK;//׷�ٽ����д��Դ��

public:
	void SetMapImg(CString &camName, Mat &mat);//��_CamImgMap�м�ֵΪcamName�ļ�¼����ͼ��
	void GetCamGmf(CString &camName, vector<TrackingRes> &GmfVec);//��_trackingResMap�м�ֵΪcamName��vector�л�ȡ��������ͼ��
};
#endif




