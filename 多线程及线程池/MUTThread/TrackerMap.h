
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
	DWORD   HunaID; //追踪对象ID
	CString CamName; //所属相机名字
	CRect   initRect; //特征初始区域
	Mat     img; //当前图像
	BOOL    bIsTracked; //是否已经开始追踪
	BOOL    bIsLost; //是否失去追踪目标
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
	DWORD   HunaID; //追踪对象ID
	CString CamName; //所属相机名字
	cv::Rect resRect; //追踪结果的区域信息
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
	static map<DWORD, TrackerInfo> _TrackerMap;//以人物ID为键值的追踪容器map
	static CRITICAL_SECTION  _mapToTrackerCriticalSection;//map资源临界区

	static BOOL _bIsMonitor;//是否可见识标志
	static CDICThreadPool _ThreadPool;//线程池对象

	static map<CString, Mat> _CamImgMap;//以相机为键值的图像Map
	static SRWLOCK _CamImgMapSRWLOCK; //图像Map资源读写锁

	///////////////////////追踪////////////////////////////
	static DWORD _CurCreatTaskID;// 当前新开任务的ID(追踪人物的ID)
	static CRITICAL_SECTION  _curTsIDCriticalSection;//CurID资源临界区

	static map<CString, vector<TrackingRes>*> _trackingResMap;//以相机名为键值的追踪结果map
	static SRWLOCK _trackingResMapSRWLOCK;//追踪结果读写资源锁

public:
	void SetMapImg(CString &camName, Mat &mat);//向_CamImgMap中键值为camName的记录更新图像
	void GetCamGmf(CString &camName, vector<TrackingRes> &GmfVec);//从_trackingResMap中键值为camName的vector中获取待画几何图形
};
#endif




