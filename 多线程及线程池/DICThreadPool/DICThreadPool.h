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

// ����״̬���ȼ�
enum ThreadPriority
{
	Low,
	High
};

enum PoolState
{
    UnReady,    // �������̳߳�
	Ready,      // �̳߳��Ѿ�����
	Destroyed   // �̳߳��Ѿ�����
};

class CDICThreadPool
{
public:
	CDICThreadPool();
	virtual ~CDICThreadPool();

public:

	// �����̳߳�
    //************************************
    // Method:    Create
    // FullName:  CDICThreadPool::Create
    // Access:    public 
    // Returns:   BOOL
    // Qualifier:
    // Parameter: DWORD dwMaxCount �̳߳����߳��������
    // Parameter: DWORD dwMinCount �̳߳����߳���������
    // Parameter: DWORD dwChangeCount �̳߳���ÿ�εݱ䣨������������
    // Parameter: DWORD dwMinFreeCount �̳߳������ٿ����̣߳�����ﵽ���߳���������֣���Ҫ���������߳�
    //************************************
    BOOL Create( DWORD dwMaxCount, DWORD dwMinCount, 
                 DWORD dwChangeCount, DWORD dwMinFreeCount );
	
    // �����̳߳�
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
    // �߳�������
    DWORD m_dwThreadMaxCount;
    // �߳����ٸ���
    DWORD m_dwThreadMinCount;
    // �̵߳ݱ���
    DWORD m_dwThreadChangeCount;
    // �̳߳������ٿ����߳���
    DWORD m_dwMinFreeThreadCount;

    CRITICAL_SECTION m_csTaskList;
    CRITICAL_SECTION m_csThreads;

    // û�������������б�
    // ʹ��m_csTaskList��ס
    TaskList m_ListTask;

    // �������߳�Map
    // ʹ��m_csThreads��ס
    VecThread m_VecThreads;

    // ���������е�����
    // Ҳ��ʹ��m_csThreads��ס����Ϊ�������m_VecThreads�Ĳ�����ͬ����
    VecExistThread m_VecRunningTask;

    // �̳߳�״̬
    volatile PoolState m_state;
};
//------------------------------------------------------------------------------
#endif
