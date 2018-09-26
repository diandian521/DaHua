#include "stdafx.h"
#include "TrackerMap.h"

map<DWORD, TrackerInfo> CTrackerMap::_TrackerMap;
CRITICAL_SECTION  CTrackerMap::_mapToTrackerCriticalSection;

BOOL CTrackerMap::_bIsMonitor;    

map<CString, Mat> CTrackerMap::_CamImgMap;
//CRITICAL_SECTION  CTrackerMap::_CamImgMapCriticalSection;
SRWLOCK CTrackerMap::_CamImgMapSRWLOCK;

CDICThreadPool CTrackerMap::_ThreadPool;

DWORD CTrackerMap::_CurCreatTaskID;
CRITICAL_SECTION  CTrackerMap::_curTsIDCriticalSection;

map<CString, vector<TrackingRes>*> CTrackerMap::_trackingResMap;
//CRITICAL_SECTION  CTrackerMap::_trackingResMapCriticalSection;
SRWLOCK  CTrackerMap::_trackingResMapSRWLOCK;
CTrackerMap::CTrackerMap()
{
	//资源临界区及Map初始化
	_TrackerMap.clear();
	_bIsMonitor = FALSE;
	InitializeCriticalSection(&_mapToTrackerCriticalSection);

	_CurCreatTaskID = 0;
	InitializeCriticalSection(&_curTsIDCriticalSection);
	_trackingResMap.clear();
	//InitializeCriticalSection(&_trackingResMapCriticalSection);
	InitializeSRWLock(&_trackingResMapSRWLOCK);
	//InitializeCriticalSection(&_CamImgMapCriticalSection);
	InitializeSRWLock(&_CamImgMapSRWLOCK);
}

CTrackerMap::~CTrackerMap()
{
	//临界区及map资源释放
	EnterCriticalSection(&_mapToTrackerCriticalSection);
	_TrackerMap.clear();
	LeaveCriticalSection(&_mapToTrackerCriticalSection);
	DeleteCriticalSection(&_mapToTrackerCriticalSection);

	LeaveCriticalSection(&_curTsIDCriticalSection);
	DeleteCriticalSection(&_curTsIDCriticalSection);

	AcquireSRWLockExclusive(&_trackingResMapSRWLOCK);//EnterCriticalSection(&_trackingResMapCriticalSection);
	if (!_trackingResMap.empty())
	for (map<CString, vector<TrackingRes>*> ::iterator iter = _trackingResMap.begin(); iter != _trackingResMap.end(); iter++)
	{
		vector<TrackingRes>* pRes = iter->second;
		delete pRes;
		pRes = NULL;
		iter->second = NULL;
	}
	_trackingResMap.clear();
	ReleaseSRWLockExclusive(&_trackingResMapSRWLOCK);//LeaveCriticalSection(&_trackingResMapCriticalSection);
	//DeleteCriticalSection(&_trackingResMapCriticalSection);

	//EnterCriticalSection(&_CamImgMapCriticalSection);
	AcquireSRWLockExclusive(&_CamImgMapSRWLOCK);
	_CamImgMap.clear();
	ReleaseSRWLockExclusive(&_CamImgMapSRWLOCK);
	//LeaveCriticalSection(&_CamImgMapCriticalSection);
	//DeleteCriticalSection(&_CamImgMapCriticalSection);
}

void CTrackerMap::SetMapImg(CString &camName, Mat &mat)
{
	if (!mat.empty())
	{
		AcquireSRWLockExclusive(&_CamImgMapSRWLOCK);
		_CamImgMap[camName] = mat.clone();
		ReleaseSRWLockExclusive(&_CamImgMapSRWLOCK);
	}
}

void CTrackerMap::GetCamGmf(CString &camName, vector<TrackingRes> &GmfVec)
{
	vector<TrackingRes>* resVec = NULL;
	
	AcquireSRWLockExclusive(&_trackingResMapSRWLOCK);
	if (_trackingResMap.count(camName))
	{
		for (map<CString, vector<TrackingRes>*> ::iterator iter = _trackingResMap.begin(); iter != _trackingResMap.end(); iter++)
		{
			if (iter->first == camName)
			{
				//resVec->assign(iter->second.begin(), iter->second.end());
				//GmfVec.assign(*_trackingResMap[camName]->begin(), *_trackingResMap[camName]->end());
				resVec = iter->second;
				break;
			}
		}
		GmfVec.assign(resVec->begin(), resVec->end());	
	}
	ReleaseSRWLockExclusive(&_trackingResMapSRWLOCK);
	
}

