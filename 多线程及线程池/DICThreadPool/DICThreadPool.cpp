#include "stdafx.h"
#include "DICThreadPool.h"
#include <process.h>
#include <assert.h>
#define FLTEST

void OutputDebug( const WCHAR * strOutputString,...) 
{ 
    WCHAR strBuffer[4096] = {0}; 
    va_list vlArgs; 
    va_start( vlArgs,strOutputString ); 
    _vsnwprintf_s( strBuffer, ARRAYSIZE(strBuffer) -1, ARRAYSIZE(strBuffer) -1, strOutputString, vlArgs ); 
    va_end( vlArgs ); 
    OutputDebugString( strBuffer ); 
}

CDICThreadPool::CDICThreadPool()
{
	// this is used to protect the shared data like the list and map
	InitializeCriticalSection(&m_csTaskList); 
	InitializeCriticalSection(&m_csThreads); 
}

CDICThreadPool::~CDICThreadPool()
{
	Destroy();
	DeleteCriticalSection(&m_csTaskList);
	DeleteCriticalSection(&m_csThreads);
}

BOOL CDICThreadPool::Create( DWORD dwMaxCount, DWORD dwMinCount, 
                          DWORD dwChangeCount, DWORD dwMinFreeCount )
{
    if ( Ready == m_state )
    {
        return FALSE;
    }

    if ( FALSE == CheckUserSetParam( dwMaxCount, dwMinCount, 
                                     dwChangeCount, dwMinFreeCount ) )
    {
        OutputDebug( L"DeInCreaseThreadPool:Param Error %d %d %d %d\n", 
                     dwMaxCount, dwMinCount, 
                     dwChangeCount, dwMinFreeCount );
        ATLASSERT( FALSE );
        return FALSE;
    }

    m_dwThreadMaxCount = dwMaxCount;
    m_dwThreadMinCount = dwMinCount;
    m_dwThreadChangeCount = dwChangeCount;
    m_dwMinFreeThreadCount = dwMinFreeCount;

    m_state = UnReady;

    // ��Ҫ��������������̳߳�
    BOOL bSuc = CreateNewThreads( m_dwThreadMinCount );

    OutputDebug( L"DeInCreaseThreadPool:Create %d Threads\n", m_dwThreadMinCount );
    
    m_state = Ready;

    return bSuc;
}

UINT __stdcall CDICThreadPool::_ThreadProc(LPVOID pParam)
{
    ATLASSERT( NULL != pParam );
    if ( NULL == pParam )
    {
        OutputDebug( L"DeInCreaseThreadPool:Param Error in _ThreadProc %d \n", GetCurrentThreadId() );
        ATLASSERT( FALSE );
        return -1;
    }

    DWORD dwThreadId = GetCurrentThreadId();

    CDICThreadPool* pool = NULL;
    pool = static_cast<CDICThreadPool*>( pParam );

    HANDLE hWaits[2] = { NULL, NULL };
    // ֪ͨ�߳�Ѱ���µ�����ȥִ��
    hWaits[0] = pool->GetWaitHandle( dwThreadId );
    // ֪ͨ�߳������˳�
    hWaits[1] = pool->GetStopHandle( dwThreadId );

    DWORD dwWaitReson = 0;
    CDICTaskObject* runObject = NULL;
    BOOL bAutoDelete = TRUE;
    EThreadRoutineExitType eExitType = OTHERRESON;

    while ( TRUE )
    {
        dwWaitReson = WaitForMultipleObjects( 2, hWaits, FALSE, INFINITE );

        if( dwWaitReson - WAIT_OBJECT_0 == 0 )
        {
            // ��Run()��������У���ȥ��������̵߳�wait�¼�
            // �Ӷ���ִ�е�����ط�
            // ���߳�ȥ��������ص�ִ������
            if( FALSE == pool->GetThreadProc( dwThreadId, &runObject ) || NULL == runObject )
            {  
                // ���û���õ�����ҲҪ֪ͨ�̳߳ص����̸߳��߳�ִ�����
                // ��Ϊ�̳߳ص����߳��ڽ��յ����߳�ִ����Ϻ�Ҫ��wait�¼�����
                // ��������ã���ô���ܻᵼ�����̴߳�����ѭ��״̬
                OutputDebug( L"DeInCreaseThreadPool:_ThreadProc %d Can't get at task!\n", GetCurrentThreadId() );
            }
            else
            {
                // ��ֹͣ�¼����������
                runObject->m_hStopEvent = hWaits[1];
                // ֪ͨ�̳߳����̸߳��̴߳���æ��״̬
                pool->NotifyBusy( dwThreadId );

                bAutoDelete = runObject->AutoDelete();

                // ִ������
                eExitType = runObject->Run();

                if( bAutoDelete )
                {
                    OutputDebug( L"DeInCreaseThreadPool:Task of _ThreadProc %d is completed,delete the object of the task\n", GetCurrentThreadId() );
                    delete runObject;
                    runObject = NULL;
                }
            }
            
            // ֪ͨ�̳߳����̸߳��߳�Ҫ��������״̬
            BOOL bCloseSelf = pool->NotifyFinish( dwThreadId ); // tell the pool, i am now free

            if ( WAITEVENTOK == eExitType || FALSE != bCloseSelf )
            {
                // �����������Ϊ�߳��е����̽��ܵ��û�֪ͨ�رյ��¼�
                // һ�ǿ����̳߳�Ҫ�ر��ˣ����������̳߳�Ҫ�����߳���
                OutputDebug( L"DeInCreaseThreadPool:_ThreadProc %d Exit 0\n", GetCurrentThreadId() );
                break;
            }
        }
        else
        {
            // �̴߳��ڵȴ���ʱ���յ��˳�֪ͨ
            // һ�ǿ����̳߳�Ҫ�ر��ˣ����������̳߳�Ҫ�����߳���
            OutputDebug( L"DeInCreaseThreadPool:_ThreadProc %d Exit 1\n", GetCurrentThreadId() );
            break;
        }
    }
    return 0;
}

// �̳߳�����
VOID CDICThreadPool::Destroy()
{
    m_state = Destroyed;
    // �ر��߳��ͷ���Դ��
    ReleaseMemory();
}

// �߳��л�ȡ������ص���Ϣ
BOOL CDICThreadPool::GetThreadProc( DWORD dwThreadId, CDICTaskObject** runObject)
{
    LPTHREAD_START_ROUTINE lpResult = NULL;

    BOOL bSuc = FALSE;

    EnterCriticalSection(&m_csTaskList);
    //OutputDebug( L"EnterCriticalSection m_csTaskList in GetThreadProc()\n" );
    TaskListIter iter = m_ListTask.begin();
    if( m_ListTask.end() != iter )
    {
        OutputDebug( L"DeInCreaseThreadPool:Get a task from the list\n" );
        // ��ȡ��������
        *runObject = iter->runObject;

        // ������������б���ȥ��
        m_ListTask.pop_front();
        bSuc = TRUE;	
    }
    else
    {
        OutputDebug( L"DeInCreaseThreadPool:Can't get any task.\n" );
    }
    LeaveCriticalSection(&m_csTaskList);
    //OutputDebug( L"LeaveCriticalSection m_csTaskList in GetThreadProc()\n" );

    return bSuc;
}

// ��������õ�����
BOOL CDICThreadPool::Run( CDICTaskObject* runObject, ThreadPriority priority )
{
    if ( Ready != m_state )
    {
        return FALSE;
    }

    ATLASSERT( NULL != runObject );

    if ( NULL == runObject )
    {
        // �̳߳��������ٹ����У����������߳�ʵ����ʧ��
        OutputDebug( L"DeInCreaseThreadPool:Thread can't run,because ThreadPool is destroying or destroyed\n" );
        return FALSE;
    }

    _TaskData funcdata;
    funcdata.runObject = runObject;

    // ��������������б�
    EnterCriticalSection(&m_csTaskList);
    //OutputDebug( L"EnterCriticalSection m_csTaskList in Run()\n" );
    if( Low == priority )
    {
        // �������ȼ��ͣ����List�������
        // �����������������һ����ִ�е�����
        OutputDebug( L"DeInCreaseThreadPool:Inset a low priority task in the list\n" );
        m_ListTask.push_back(funcdata);
    }
    else
    {
        // ��Listǰ����룬���ȱ�ִ��
        OutputDebug( L"DeInCreaseThreadPool:Inset a high priority task in the list\n" );
        m_ListTask.push_front(funcdata);
    }
    LeaveCriticalSection(&m_csTaskList);
    //OutputDebug( L"LeaveCriticalSection m_csTaskList in Run()\n" );

    // ÿ����������Ҫ���룬�����һ���̳߳������߼��ж�
    IncreaseThreadPool();

    VecThreadIter iter;
    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in Run()\n" );
    for( iter = m_VecThreads.begin(); iter != m_VecThreads.end(); iter++ )
    {

        // ��Ϊ������һ����������Ѱ��һ�����е��߳�
        // ����һ��һ�Ĺ�ϵ�����Բ����Ա�֤����ȡ�õ��߳̾���ִ�������Ǹ������
        // ������Ϊһ��һ�Ĺ�ϵ�����Կ��Ա�֤������Ա�ִ��
        if( false != iter->bFree )
        {
            // �����߳����ó�æ״̬
            iter->bFree = false;

            // ���õȴ��¼�������sleep�е��߳�ִ��GetThreadProc()
            // ȡ������ȥִ��
            SetEvent( iter->hWait ); 

            break;
        }
    }

    if ( m_VecThreads.end() == iter )
    {
        OutputDebug( L"DeInCreaseThreadPool:ThreadPool is Full,the task is waiting for a free thread to excute\n" );
    }
    else
    {
        OutputDebug( L"DeInCreaseThreadPool:ThreadPool is not Full,the task is being excuted in thread %d\n", iter->dwThreadId );
    }

    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in Run()\n" );

    return TRUE;
}

BOOL CDICThreadPool::IncreaseThreadPool()
{
    // ��ȡ�����߳���
    DWORD dwFreeThreadsCount = GetFreeThreadCount();

    if ( m_dwMinFreeThreadCount + 1 < dwFreeThreadsCount )
    {
        // ������õ���С�����߳���С��Ŀǰ��С�����߳���
        // ����Ϊ�����㹻���̣߳��̳߳ز���Ҫ����
        return FALSE;
    }

    // ��Ҫ�����̳߳�
    OutputDebug( L"DeInCreaseThreadPool:There is %d free threads,but the min count of free threads is %d,so I will create more threads\n", 
                 dwFreeThreadsCount - 1, m_dwMinFreeThreadCount );
   
    DWORD dwCount = m_dwThreadChangeCount;

    DWORD dwThreadCurrentCount = GetThreadsCount();

    if ( dwThreadCurrentCount + dwCount > m_dwThreadMaxCount )
    {
        // ���Ҫ�½����̺߳͵�ǰ�߳�������������߳�����������Ҫ�������߳���Ϊ
        // ����߳����͵�ǰ�߳����Ĳ�ֵ
        
        dwCount = m_dwThreadMaxCount - dwThreadCurrentCount;
        OutputDebug( L"DeInCreaseThreadPool:There is %d threads,but the max count of threads is %d,so I can't create %d threads\n,I will create %d threads",
                     dwThreadCurrentCount, m_dwThreadMaxCount, m_dwThreadChangeCount, dwCount);
    }

    return CreateNewThreads( dwCount );
}

BOOL CDICThreadPool::CreateNewThreads( DWORD dwCount )
{
    HANDLE hThread = NULL;
    UINT uThreadId = 0;	
    _ThreadData ThreadData;

    // ������ʱ�����߳���Ϣ�����߳�ȫ������ϣ��ٱ��浽m_VecThreads��
    VecThread TmpThreadMap;

    // ����ָ�������߳�
    for ( DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++ )
    {
        hThread = (HANDLE)_beginthreadex( NULL, 0, CDICThreadPool::_ThreadProc, this,  
            CREATE_SUSPENDED, (UINT*)&uThreadId );

        ATLASSERT( INVALID_HANDLE_VALUE != hThread );
        ATLASSERT( NULL != hThread );
        if ( INVALID_HANDLE_VALUE == hThread || NULL == hThread  )
        {
            // �������ʧ�ܣ�Ҫ��֮ǰ�����ɹ����̼߳������߳���
            break;
        }
        
        OutputDebug( L"DeInCreaseThreadPool:Create thread %d and suspend it\n", uThreadId );

        // add the entry to the map of threads
        ThreadData.bFree		= TRUE;
        ThreadData.hWait	    = CreateEvent( NULL, TRUE, FALSE, NULL );
        ThreadData.hStop	    = CreateEvent( NULL, TRUE, FALSE, NULL );
        ThreadData.hThread		= hThread;
        ThreadData.dwThreadId	= uThreadId;

        // ���߳����ݱ��浽��ʱMap��
        TmpThreadMap.push_back( ThreadData );		
    }

    // ���߳����ݱ��浽m_VecThreads�У������������߳�
    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in CreateNewThreads()\n" );
    if ( UnReady == m_state || Ready == m_state )
    {
        for ( VecThreadIter it = TmpThreadMap.begin(); it != TmpThreadMap.end(); it++ )
        {
            m_VecThreads.push_back( *it );
            ResumeThread( it->hThread ); 
            OutputDebug( L"DeInCreaseThreadPool:Resume thread %d\n", it->dwThreadId );
        }
    }
    else if ( Destroyed == m_state  )
    {
        // �ж��߳��Ƿ��Ƿ�Ϊ�գ�Ϊ������Ϊ�Ѿ����٣������ٸ����մ������߳�
        for ( VecThreadIter it = TmpThreadMap.begin(); it != TmpThreadMap.end(); it++ )
        {
            TerminateThread( it->hThread, 1 ); 
            OutputDebug( L"DeInCreaseThreadPool:Thread Pool is destroyed.I will terminate thread %d\n", it->dwThreadId );
        }
    }
    LeaveCriticalSection( &m_csThreads );
    //OutputDebug( L"LeaveCriticalSection m_csThreads in CreateNewThreads()\n" );

    return TRUE;
}

BOOL CDICThreadPool::DecreaseThreadPool()
{
    DWORD dwWorkingThreadCount = GetWorkingThreadCount();
    DWORD dwThreadCurrentCount = GetThreadsCount();

    if ( m_dwThreadMinCount >= dwThreadCurrentCount )
    {
        // �������߳����͵�ǰ�߳�����ȣ����ͷ��κ��߳�
        return FALSE;
    }

    if ( dwWorkingThreadCount > dwThreadCurrentCount / 2 )
    {
        // ���������ǰ�̳߳����߳�����һ�����ϵ��߳��ڹ��������ͷ�
        OutputDebug( L"DeInCreaseThreadPool:There is %d working threads,more than half of the count(%d) of threads,I will not decrease the size of the thread pool\n",
                     dwWorkingThreadCount, dwThreadCurrentCount );
        return FALSE;
    }

    OutputDebug( L"DeInCreaseThreadPool:There is %d working threads,more than half of the count(%d) of threads,I will attempt to decrease the size of the thread pool\n",
        dwWorkingThreadCount, dwThreadCurrentCount );

    // ��ͼ���ٵ��߳���
    DWORD dwNeedFreeThread = dwThreadCurrentCount - dwWorkingThreadCount;
    
    // ���ٵ��߳�������m_dwThreadChangeCountΪ��λ��
    dwNeedFreeThread = ( dwNeedFreeThread / m_dwThreadChangeCount ) * m_dwThreadChangeCount;

    if ( dwThreadCurrentCount < m_dwThreadMinCount + dwNeedFreeThread )
    {
        // ������ٺ���߳�������С�߳�����С������ٵ�����߳���
        dwNeedFreeThread = dwThreadCurrentCount - m_dwThreadMinCount;
    }

    if ( dwThreadCurrentCount <= dwWorkingThreadCount + dwNeedFreeThread + m_dwMinFreeThreadCount )
    {
        // dwThreadCurrentCount - dwNeedFreeThread - dwWorkingThreadCount <= m_dwMinFreeThreadCount 
        // ������ٺ���߳����еĿ����߳�������Ϳ����߳�������ݼ��߳���
        OutputDebug( L"DeInCreaseThreadPool:The Count of Current Threads is %d.\n", dwThreadCurrentCount );
        OutputDebug( L"DeInCreaseThreadPool:The Count of Current Working Threads is %d.\n", dwWorkingThreadCount );
        OutputDebug( L"DeInCreaseThreadPool:The Count of Need Free Threads is %d.\n", dwNeedFreeThread );
        OutputDebug( L"DeInCreaseThreadPool:The Count of Min Free Threads is %d.\n", m_dwMinFreeThreadCount );
        OutputDebug( L"DeInCreaseThreadPool:If I decrease thread pool size,the count of free threads is less than min free count.I will not decrease the size of thread pool\n" );
        return FALSE;
    }

    OutputDebug( L"DeInCreaseThreadPool:I will decrease the thread pool by %d\n", dwNeedFreeThread );
    return ReleaseFreeThreads( dwNeedFreeThread );
}

BOOL CDICThreadPool::ReleaseFreeThreads( DWORD dwNeedFreeThreadCount )
{
    BOOL bCloseSelf = FALSE;

    // ������ʱ�����߳���Ϣ
    VecThread TmpThreadMap;

    DWORD dwFreeCount = 0;
    BOOL bCanFree = FALSE;
    // �ҳ�С�ڵ���dwNeedFreeThreadCount�����̣߳���m_VecThreads��ɾ���������浽TmpThreadMap��
    // ֮����˵����С�ڵ��������Ϊ��֮ǰѡȡ�ͷ�����������������������˼����̣߳����¿��е�û��ô��
    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in ReleaseFreeThreads()\n" );
    for ( VecThreadIter it = m_VecThreads.begin(); it != m_VecThreads.end(); it++)
    {
        if ( false != it->bFree )
        {
            dwFreeCount++;
            if ( dwFreeCount >= dwNeedFreeThreadCount )
            {
                // �����㹻�Ŀ���ɾ�����߳�
                bCanFree = TRUE;
                break;
            }
        }
    }
    dwFreeCount = 0;
    if ( FALSE != bCanFree )
    {
        for ( VecThreadIter it = m_VecThreads.begin(); it != m_VecThreads.end(); )
        {
            if ( false != it->bFree )
            {
                // ���浽��ʱMap�У��Թ��Ժ��ͷ�
                TmpThreadMap.push_back( *it );

                // ���߳���Ϣ��map��ɾ��
                it = m_VecThreads.erase( it );   // ��ɾ�������һ����������it

                dwFreeCount++;
                if ( dwFreeCount >= dwNeedFreeThreadCount )
                {
                    // �Ѵﵽɾ���̵߳ĸ�����ֱ���˳�ѭ��
                    break;
                }

                // �ϲ�it��ָ����һ����Ա����ֱ��continue�����ú�����
                continue;
            }
            // ����ö�ٵ����ڹ������߳���ֱ�ӽ�������ָ���һ��Ԫ��
            it++;
        }
    }
    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in ReleaseFreeThreads()\n" );

    bCloseSelf = CloseThreads( TmpThreadMap );

#ifdef FLTEST
    DWORD dwCurrent = GetThreadsCount();
    OutputDebug( L"DeInCreaseThreadPool:There is %d threads in thread pool after decreaseing the thread pool \n", dwCurrent );
#endif
    
    return bCloseSelf;
}

// ��ȡ�������̵߳ĸ���
DWORD CDICThreadPool::GetWorkingThreadCount()
{
    DWORD dwCount = 0;

    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in GetWorkingThreadCount()\n" );
    for ( VecThreadIter iter = m_VecThreads.begin(); iter != m_VecThreads.end(); iter++) 
    {
        if ( false == (*iter).bFree ) 
        {
            dwCount++;
        }
    }
    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in GetWorkingThreadCount()\n" );

    return dwCount;
}

DWORD CDICThreadPool::GetFreeThreadCount()
{
    DWORD dwCount = 0;

    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in GetWorkingThreadCount()\n" );
    for ( VecThreadIter iter = m_VecThreads.begin(); iter != m_VecThreads.end(); iter++) 
    {
        if ( WAIT_OBJECT_0 == WaitForSingleObject( iter->hThread, 10) )
        {
            // �߳�Ī���˳��ˣ��ͳ���������һ���߳��油
            CreateThreadReuseItem( *iter );
        }

        if ( false != (*iter).bFree ) 
        {
            dwCount++;
        }
    }
    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in GetWorkingThreadCount()\n" );

    return dwCount;
}

DWORD CDICThreadPool::GetThreadsCount()
{
    DWORD dwCount = 0;

    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in GetThreadsCount()\n" );
    dwCount = (DWORD) m_VecThreads.size();
    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in GetThreadsCount()\n" );

    return dwCount;
}

// ͨ���߳�ID��ȡ��ȴ����
HANDLE CDICThreadPool::GetWaitHandle( DWORD dwThreadId )
{
    HANDLE hWait = NULL;

    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in GetWaitHandle()\n" );
    for ( VecThreadIter iter = m_VecThreads.begin(); iter != m_VecThreads.end(); iter++ )
    {
        if ( dwThreadId == iter->dwThreadId )
        {
            hWait = iter->hWait;
            break;
        }
    }
    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in GetWaitHandle()\n" );

    return hWait;
}

// ͨ���߳�ID��ȡ��ֹͣ���
HANDLE CDICThreadPool::GetStopHandle( DWORD dwThreadId )
{
    HANDLE hStop = NULL;

    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in GetStopHandle()\n" );
    for ( VecThreadIter iter = m_VecThreads.begin(); iter != m_VecThreads.end(); iter++ )
    {
        if ( dwThreadId == iter->dwThreadId )
        {
            hStop = iter->hStop;
            break;
        }
    }
    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in GetStopHandle()\n" );

    return hStop;
}

// ���߳�����Ϊæ״̬
VOID CDICThreadPool::NotifyBusy( DWORD dwThreadId )
{
    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in NotifyBusy()\n" );
    for ( VecThreadIter iter = m_VecThreads.begin(); iter != m_VecThreads.end(); iter++ )
    {
        if ( dwThreadId == iter->dwThreadId )
        {
            iter->bFree = false;
        }
    }
    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in NotifyBusy()\n" );
}

// ���߳�����Ϊ����״̬
BOOL CDICThreadPool::NotifyFinish(DWORD dwThreadId)
{
    BOOL bCloseSelf = FALSE;

    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in NotifyFinish()\n" );
    for ( VecThreadIter iter = m_VecThreads.begin(); iter != m_VecThreads.end(); iter++ )
    {
        if ( dwThreadId == iter->dwThreadId )
        {
            iter->bFree = TRUE;

            // m_ListRunningTask�Ĳ���Ҳ��ʹ��m_csThreads��ס
            for ( VecExistThreadIter it = m_VecRunningTask.begin(); 
                it != m_VecRunningTask.end(); it++ )
            {
                if ( dwThreadId == *it )
                {
                    m_VecRunningTask.erase( it );
                    break;
                }
            }

            if( false != m_ListTask.empty() )
            {
                // ���û��Ҫִ�е����������߳�����
                ResetEvent( iter->hWait );
            }
            break;
        }
    }

    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in NotifyFinish()\n" );

    bCloseSelf = DecreaseThreadPool();
    return bCloseSelf;
}

// �ر�_ThreadData�����о��
VOID CDICThreadPool::CloseThradDataHandle( _ThreadData& TD )
{
    if ( NULL != TD.hWait )
    {
        CloseHandle( TD.hWait );
        TD.hWait = NULL;
    }

    if ( NULL != TD.hStop )
    {
        CloseHandle( TD.hStop );
        TD.hStop = NULL;
    }

    if ( NULL != TD.hThread )
    {
        CloseHandle( TD.hThread );
        TD.hThread = NULL;
    }
}

VOID CDICThreadPool::ReleaseMemory()
{
//////////////////////////////////////////////////////////////////////////
    VecThread TmpMapThread;

    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in ReleaseMemory()\n" );
    if ( false == m_VecThreads.empty() )
    {
        TmpMapThread.assign( m_VecThreads.begin(), m_VecThreads.end() );
    }
    m_VecThreads.clear();
    LeaveCriticalSection(&m_csThreads);
    //OutputDebug( L"LeaveCriticalSection m_csThreads in ReleaseMemory()\n" );

    CloseThreads( TmpMapThread );
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
    TaskList TmpTaskList;

    EnterCriticalSection(&m_csTaskList);
    //OutputDebug( L"LeaveCriticalSection m_csTaskList in ReleaseMemory()\n" );
    if ( false != m_ListTask.empty() )
    {
        TmpTaskList.assign( m_ListTask.begin(), m_ListTask.end() );
    }
    m_ListTask.clear();
    LeaveCriticalSection(&m_csTaskList);

    ReleaseTaskList( TmpTaskList );
    //OutputDebug( L"LeaveCriticalSection m_csTaskList in ReleaseMemory()\n" );
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
    m_VecRunningTask.clear();
//////////////////////////////////////////////////////////////////////////
}

// �ͷ�ָ��TaskList�е�����
VOID CDICThreadPool::ReleaseTaskList( TaskList & ListTask )
{
    TaskListIter TaskIter = ListTask.end();

    for( TaskIter = ListTask.begin(); TaskIter != ListTask.end(); TaskIter++ ) 
    {
        if( NULL == TaskIter->runObject ) 
        {
            continue;
        }
        delete TaskIter->runObject;
    }
}

// �ر�ָ��Map�е��߳�
BOOL CDICThreadPool::CloseThreads( VecThread& MapThread )
{
    BOOL bCloseSelf = FALSE;

    DWORD dwResont = WAIT_TIMEOUT;

    // ����ͷ��߳�
    for ( VecThreadIter it = MapThread.begin(); it != MapThread.end(); it++ )
    {
        if ( NULL != it->hStop )
        {
            // �����¼����ȴ�100ms
            SetEvent( it->hStop );
            dwResont = WaitForSingleObject( it->hThread, 100 );
        }

        if ( NULL != it->hThread )
        {
            if ( WAIT_OBJECT_0 != dwResont )
            {
                if ( GetCurrentThreadId() == it->dwThreadId )
                {
                    bCloseSelf = TRUE;
                    continue;
                }
                // û�еȵ��̹߳رգ���ǿ�ƹر��߳�
                TerminateThread( it->hThread, 0 );
            }
        }

        CloseThradDataHandle( *it );
    }

    return bCloseSelf;
}

// ���������ز����Ƿ�Ϸ�
BOOL CDICThreadPool::CheckUserSetParam( DWORD dwMaxCount, DWORD dwMinCount, 
                                     DWORD dwChangeCount, DWORD dwMinFreeCount )
{
    if ( dwMaxCount < dwMinCount ||
         dwMinFreeCount >= dwMaxCount )
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    return TRUE;
}


BOOL CDICThreadPool::CreateThreadReuseItem( _ThreadData& threaddata )
{
    // �̹߳���
    OutputDebug( L"DeInCreaseThreadPool:Thread %d is crashed.I will create a new thread!!!!!\n", threaddata.dwThreadId );
    UINT uThreadId = 0;
    HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, CDICThreadPool::_ThreadProc, this,  
        CREATE_SUSPENDED, (UINT*)&uThreadId );

    ATLASSERT( INVALID_HANDLE_VALUE != hThread );
    ATLASSERT( NULL != hThread );
    if ( INVALID_HANDLE_VALUE == hThread || NULL == hThread  )
    {
        // �������ʧ�ܣ�Ҫ��֮ǰ�����ɹ����̼߳������߳���
        return FALSE;
    }

    OutputDebug( L"DeInCreaseThreadPool:Create thread %d and suspend it\n", uThreadId );

    // add the entry to the Vec of threads
    threaddata.bFree		= true;
    threaddata.hWait	    = CreateEvent( NULL, TRUE, FALSE, NULL );
    threaddata.hStop	    = CreateEvent( NULL, TRUE, FALSE, NULL );
    threaddata.hThread      = hThread;
    threaddata.dwThreadId	= uThreadId;

    ResumeThread( hThread );

    OutputDebug( L"DeInCreaseThreadPool:Resume thread %d\n", uThreadId );

    return TRUE;
}
