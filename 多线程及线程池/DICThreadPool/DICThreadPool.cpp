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

    // 按要求最低数量建立线程池
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
    // 通知线程寻找新的任务去执行
    hWaits[0] = pool->GetWaitHandle( dwThreadId );
    // 通知线程例程退出
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
            // 在Run()这个函数中，会去触发这个线程的wait事件
            // 从而会执行到这个地方
            // 该线程去拿任务相关的执行数据
            if( FALSE == pool->GetThreadProc( dwThreadId, &runObject ) || NULL == runObject )
            {  
                // 如果没有拿到任务，也要通知线程池的主线程该线程执行完毕
                // 因为线程池的主线程在接收到该线程执行完毕后，要将wait事件重置
                // 如果不设置，那么可能会导致这线程处于死循环状态
                OutputDebug( L"DeInCreaseThreadPool:_ThreadProc %d Can't get at task!\n", GetCurrentThreadId() );
            }
            else
            {
                // 将停止事件传入给对象
                runObject->m_hStopEvent = hWaits[1];
                // 通知线程池主线程该线程处于忙的状态
                pool->NotifyBusy( dwThreadId );

                bAutoDelete = runObject->AutoDelete();

                // 执行任务
                eExitType = runObject->Run();

                if( bAutoDelete )
                {
                    OutputDebug( L"DeInCreaseThreadPool:Task of _ThreadProc %d is completed,delete the object of the task\n", GetCurrentThreadId() );
                    delete runObject;
                    runObject = NULL;
                }
            }
            
            // 通知线程池主线程该线程要处于闲置状态
            BOOL bCloseSelf = pool->NotifyFinish( dwThreadId ); // tell the pool, i am now free

            if ( WAITEVENTOK == eExitType || FALSE != bCloseSelf )
            {
                // 这种情况是因为线程中的例程接受到用户通知关闭的事件
                // 一是可能线程池要关闭了，二可能是线程池要减少线程数
                OutputDebug( L"DeInCreaseThreadPool:_ThreadProc %d Exit 0\n", GetCurrentThreadId() );
                break;
            }
        }
        else
        {
            // 线程处于等待的时候，收到退出通知
            // 一是可能线程池要关闭了，二可能是线程池要减少线程数
            OutputDebug( L"DeInCreaseThreadPool:_ThreadProc %d Exit 1\n", GetCurrentThreadId() );
            break;
        }
    }
    return 0;
}

// 线程池销毁
VOID CDICThreadPool::Destroy()
{
    m_state = Destroyed;
    // 关闭线程释放资源等
    ReleaseMemory();
}

// 线程中获取任务相关的信息
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
        // 获取任务数据
        *runObject = iter->runObject;

        // 将任务从任务列表中去掉
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

// 任务类调用的例程
BOOL CDICThreadPool::Run( CDICTaskObject* runObject, ThreadPriority priority )
{
    if ( Ready != m_state )
    {
        return FALSE;
    }

    ATLASSERT( NULL != runObject );

    if ( NULL == runObject )
    {
        // 线程池正在销毁过程中，则启动新线程实例将失败
        OutputDebug( L"DeInCreaseThreadPool:Thread can't run,because ThreadPool is destroying or destroyed\n" );
        return FALSE;
    }

    _TaskData funcdata;
    funcdata.runObject = runObject;

    // 将任务加入任务列表
    EnterCriticalSection(&m_csTaskList);
    //OutputDebug( L"EnterCriticalSection m_csTaskList in Run()\n" );
    if( Low == priority )
    {
        // 如果任务等级低，则从List后面插入
        // 这样它将可能是最后一个被执行的任务
        OutputDebug( L"DeInCreaseThreadPool:Inset a low priority task in the list\n" );
        m_ListTask.push_back(funcdata);
    }
    else
    {
        // 从List前面插入，优先被执行
        OutputDebug( L"DeInCreaseThreadPool:Inset a high priority task in the list\n" );
        m_ListTask.push_front(funcdata);
    }
    LeaveCriticalSection(&m_csTaskList);
    //OutputDebug( L"LeaveCriticalSection m_csTaskList in Run()\n" );

    // 每当有新任务要加入，则会做一次线程池扩容逻辑判断
    IncreaseThreadPool();

    VecThreadIter iter;
    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in Run()\n" );
    for( iter = m_VecThreads.begin(); iter != m_VecThreads.end(); iter++ )
    {

        // 因为新增了一个任务，所以寻找一个空闲的线程
        // 这是一对一的关系，所以不可以保证本次取得的线程就是执行上面那个任务的
        // 但是因为一对一的关系，所以可以保证任务可以被执行
        if( false != iter->bFree )
        {
            // 将该线程设置成忙状态
            iter->bFree = false;

            // 设置等待事件，以让sleep中的线程执行GetThreadProc()
            // 取出任务去执行
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
    // 获取空闲线程数
    DWORD dwFreeThreadsCount = GetFreeThreadCount();

    if ( m_dwMinFreeThreadCount + 1 < dwFreeThreadsCount )
    {
        // 如果设置的最小空闲线程数小于目前最小空闲线程数
        // 则认为还有足够的线程，线程池不需要扩充
        return FALSE;
    }

    // 需要扩充线程池
    OutputDebug( L"DeInCreaseThreadPool:There is %d free threads,but the min count of free threads is %d,so I will create more threads\n", 
                 dwFreeThreadsCount - 1, m_dwMinFreeThreadCount );
   
    DWORD dwCount = m_dwThreadChangeCount;

    DWORD dwThreadCurrentCount = GetThreadsCount();

    if ( dwThreadCurrentCount + dwCount > m_dwThreadMaxCount )
    {
        // 如果要新建的线程和当前线程总数大于最大线程数，则设置要创建的线程数为
        // 最大线程数和当前线程数的差值
        
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

    // 用于临时保存线程信息，等线程全创建完毕，再保存到m_VecThreads中
    VecThread TmpThreadMap;

    // 创建指定个数线程
    for ( DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++ )
    {
        hThread = (HANDLE)_beginthreadex( NULL, 0, CDICThreadPool::_ThreadProc, this,  
            CREATE_SUSPENDED, (UINT*)&uThreadId );

        ATLASSERT( INVALID_HANDLE_VALUE != hThread );
        ATLASSERT( NULL != hThread );
        if ( INVALID_HANDLE_VALUE == hThread || NULL == hThread  )
        {
            // 如果创建失败，要将之前创建成功的线程加入总线程中
            break;
        }
        
        OutputDebug( L"DeInCreaseThreadPool:Create thread %d and suspend it\n", uThreadId );

        // add the entry to the map of threads
        ThreadData.bFree		= TRUE;
        ThreadData.hWait	    = CreateEvent( NULL, TRUE, FALSE, NULL );
        ThreadData.hStop	    = CreateEvent( NULL, TRUE, FALSE, NULL );
        ThreadData.hThread		= hThread;
        ThreadData.dwThreadId	= uThreadId;

        // 将线程数据保存到临时Map中
        TmpThreadMap.push_back( ThreadData );		
    }

    // 将线程数据保存到m_VecThreads中，并启动各个线程
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
        // 判断线程是否是否为空，为空则认为已经销毁，就销毁各个刚创建的线程
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
        // 如果最低线程数和当前线程数相等，则不释放任何线程
        return FALSE;
    }

    if ( dwWorkingThreadCount > dwThreadCurrentCount / 2 )
    {
        // 如果超过当前线程池中线程总数一半以上的线程在工作，则不释放
        OutputDebug( L"DeInCreaseThreadPool:There is %d working threads,more than half of the count(%d) of threads,I will not decrease the size of the thread pool\n",
                     dwWorkingThreadCount, dwThreadCurrentCount );
        return FALSE;
    }

    OutputDebug( L"DeInCreaseThreadPool:There is %d working threads,more than half of the count(%d) of threads,I will attempt to decrease the size of the thread pool\n",
        dwWorkingThreadCount, dwThreadCurrentCount );

    // 试图减少的线程数
    DWORD dwNeedFreeThread = dwThreadCurrentCount - dwWorkingThreadCount;
    
    // 减少的线程数（以m_dwThreadChangeCount为单位）
    dwNeedFreeThread = ( dwNeedFreeThread / m_dwThreadChangeCount ) * m_dwThreadChangeCount;

    if ( dwThreadCurrentCount < m_dwThreadMinCount + dwNeedFreeThread )
    {
        // 如果减少后的线程数比最小线程数还小，则减少到最低线程数
        dwNeedFreeThread = dwThreadCurrentCount - m_dwThreadMinCount;
    }

    if ( dwThreadCurrentCount <= dwWorkingThreadCount + dwNeedFreeThread + m_dwMinFreeThreadCount )
    {
        // dwThreadCurrentCount - dwNeedFreeThread - dwWorkingThreadCount <= m_dwMinFreeThreadCount 
        // 如果减少后的线程数中的空闲线程少于最低空闲线程数，则递减线程数
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

    // 用于临时保存线程信息
    VecThread TmpThreadMap;

    DWORD dwFreeCount = 0;
    BOOL bCanFree = FALSE;
    // 找出小于等于dwNeedFreeThreadCount个的线程，从m_VecThreads中删除，并保存到TmpThreadMap中
    // 之所以说存在小于的情况，因为在之前选取释放数后，其他请求可能又启动了几个线程，导致空闲的没那么多
    EnterCriticalSection(&m_csThreads);
    //OutputDebug( L"EnterCriticalSection m_csThreads in ReleaseFreeThreads()\n" );
    for ( VecThreadIter it = m_VecThreads.begin(); it != m_VecThreads.end(); it++)
    {
        if ( false != it->bFree )
        {
            dwFreeCount++;
            if ( dwFreeCount >= dwNeedFreeThreadCount )
            {
                // 存在足够的可以删除的线程
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
                // 保存到临时Map中，以供以后释放
                TmpThreadMap.push_back( *it );

                // 将线程信息从map中删除
                it = m_VecThreads.erase( it );   // 将删除后的下一个迭代器给it

                dwFreeCount++;
                if ( dwFreeCount >= dwNeedFreeThreadCount )
                {
                    // 已达到删除线程的个数就直接退出循环
                    break;
                }

                // 上步it已指向下一个成员，就直接continue，不用后退了
                continue;
            }
            // 对于枚举到仍在工作的线程则直接将迭代器指向后一个元素
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

// 获取工作中线程的个数
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
            // 线程莫名退出了，就尝试再启动一个线程替补
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

// 通过线程ID获取其等待句柄
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

// 通过线程ID获取其停止句柄
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

// 将线程设置为忙状态
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

// 将线程设置为闲置状态
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

            // m_ListRunningTask的操作也是使用m_csThreads锁住
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
                // 如果没有要执行的任务，则让线程休眠
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

// 关闭_ThreadData中所有句柄
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

// 释放指定TaskList中的任务
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

// 关闭指定Map中的线程
BOOL CDICThreadPool::CloseThreads( VecThread& MapThread )
{
    BOOL bCloseSelf = FALSE;

    DWORD dwResont = WAIT_TIMEOUT;

    // 逐个释放线程
    for ( VecThreadIter it = MapThread.begin(); it != MapThread.end(); it++ )
    {
        if ( NULL != it->hStop )
        {
            // 设置事件并等待100ms
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
                // 没有等到线程关闭，就强制关闭线程
                TerminateThread( it->hThread, 0 );
            }
        }

        CloseThradDataHandle( *it );
    }

    return bCloseSelf;
}

// 检测输入相关参数是否合法
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
    // 线程挂了
    OutputDebug( L"DeInCreaseThreadPool:Thread %d is crashed.I will create a new thread!!!!!\n", threaddata.dwThreadId );
    UINT uThreadId = 0;
    HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, CDICThreadPool::_ThreadProc, this,  
        CREATE_SUSPENDED, (UINT*)&uThreadId );

    ATLASSERT( INVALID_HANDLE_VALUE != hThread );
    ATLASSERT( NULL != hThread );
    if ( INVALID_HANDLE_VALUE == hThread || NULL == hThread  )
    {
        // 如果创建失败，要将之前创建成功的线程加入总线程中
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
