#ifndef THREAD_POOL_CLASS
#define THREAD_POOL_CLASS

#include <winbase.h>
#include <windows.h>
#include <vector>
#include <list>
#include <map>
#include "DICTaskObject.h"

#define POOL_SIZE		  30

typedef std::vector<DWORD> VecExistThread;
typedef VecExistThread::iterator VecExistThreadIter;
typedef VecExistThread::reverse_iterator VecExistThreadReIter;

typedef struct tagTaskData
{
	CDICTaskObject* runObject;
} _TaskData;

typedef struct tagThreadData
{
	BOOL	bFree;
	HANDLE	hWait;
	HANDLE	hThread;
    HANDLE  hStop;
	UINT	dwThreadId;
} _ThreadData;

typedef std::vector<_ThreadData> VecThread;
typedef VecThread::iterator VecThreadIter;

typedef std::list<_TaskData> TaskList;
typedef TaskList::iterator TaskListIter;

// 任务状态优先级
enum ThreadPriority
{
	Low,
	High
};

enum PoolState
{
    UnReady,    // 待创建线程池
	Ready,      // 线程池已经创建
	Destroyed   // 线程池已经销毁
};

class CDICThreadPool
{
public:
	CDICThreadPool();
	virtual ~CDICThreadPool();

public:

	// 创建线程池
    //************************************
    // Method:    Create
    // FullName:  CDICThreadPool::Create
    // Access:    public 
    // Returns:   BOOL
    // Qualifier:
    // Parameter: DWORD dwMaxCount 线程池中线程最大数量
    // Parameter: DWORD dwMinCount 线程池中线程最少数量
    // Parameter: DWORD dwChangeCount 线程池中每次递变（增减）的数量
    // Parameter: DWORD dwMinFreeCount 线程池中最少空闲线程，如果达到或者超过这个数字，则要创建更多线程
    //************************************
    BOOL Create( DWORD dwMaxCount, DWORD dwMinCount, 
                 DWORD dwChangeCount, DWORD dwMinFreeCount );
	
    // 销毁线程池
    VOID Destroy();

	BOOL Run( CDICTaskObject*, ThreadPriority priority = Low );

    VOID NotifyBusy( DWORD dwThreadId );
    BOOL NotifyFinish( DWORD dwThreadId );

private:
    BOOL IncreaseThreadPool();
    BOOL CreateNewThreads( DWORD dwCount );

    BOOL DecreaseThreadPool();
    BOOL ReleaseFreeThreads( DWORD dwNeedFreeThreadCount );

    BOOL CheckUserSetParam( DWORD dwMaxCount, DWORD dwMinCount, 
                            DWORD dwChangeCount, DWORD dwMinFreeCount );

    VOID ReleaseMemory();
    VOID ReleaseTaskList( TaskList & ListTask );
    BOOL CloseThreads( VecThread& MapThread );

    VOID CloseThradDataHandle( _ThreadData& TD );

    DWORD GetWorkingThreadCount();
    DWORD GetFreeThreadCount();
    DWORD GetThreadsCount();

    HANDLE GetWaitHandle( DWORD dwThreadId );
    HANDLE GetStopHandle( DWORD dwThreadId );

    BOOL GetThreadProc( DWORD dwThreadId, CDICTaskObject** ); 

    static UINT __stdcall _ThreadProc(LPVOID pParam);

    BOOL CreateThreadReuseItem( _ThreadData& threaddata );
private:
    // 线程最大个数
    DWORD m_dwThreadMaxCount;
    // 线程最少个数
    DWORD m_dwThreadMinCount;
    // 线程递变数
    DWORD m_dwThreadChangeCount;
    // 线程池中最少空闲线程数
    DWORD m_dwMinFreeThreadCount;

    CRITICAL_SECTION m_csTaskList;
    CRITICAL_SECTION m_csThreads;

    // 没有启动的任务列表
    // 使用m_csTaskList锁住
    TaskList m_ListTask;

    // 启动的线程Map
    // 使用m_csThreads锁住
    VecThread m_VecThreads;

    // 正在运行中的任务
    // 也是使用m_csThreads锁住，因为其操作和m_VecThreads的操作是同步的
    VecExistThread m_VecRunningTask;

    // 线程池状态
    volatile PoolState m_state;
};
//------------------------------------------------------------------------------
#endif
