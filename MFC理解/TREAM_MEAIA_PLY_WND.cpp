// TREAM_MEAIA_PLY_WND.cpp : 实现文件
//

#include "stdafx.h"
#include "CStreamMediaPlyWnd.h"
#include "TREAM_MEAIA_PLY_WND.h"
#include "afxdialogex.h"
//#include "../CommonClass/DataLog.h"

// CTREAM_MEAIA_PLY_WND 对话框
namespace CILENT_IMGWND_SPACE
{
	IMPLEMENT_DYNAMIC(CTREAM_MEAIA_PLY_WND, CBaseModelDialogEx)

	CTREAM_MEAIA_PLY_WND::CTREAM_MEAIA_PLY_WND(CWnd* pParent /*=NULL*/)
	: CBaseModelDialogEx(CTREAM_MEAIA_PLY_WND::IDD, pParent)
	{
		m_wndType = (DWORD)WND_TYPE;
		m_toolBar = NULL;
		m_bIsToolBarCreat = false;
		m_bIsDisToolBar = false;
		m_paintType = PAINT_WND_NONE;

		m_nImgWidthStep = 0;
		m_nImgChannels = 0;
		m_pImgData = NULL;
		m_bHadImg = false;
		m_adaptScale = 1.0;
		m_adaptScale = 1.0;
		m_changeScale = 1.0;

		m_displayModel = DISPLAY_ADAPTIVE_WNDSIZE;
		m_oldAdaptModel = DISPLAY_ADAPTIVE_WNDSIZE;
		m_adptType = ADAPT_WND_SIZE;
		m_chngType = CHNG_WND_MIN;
		m_playType = PLAY_CAMERA;
		m_soloType = CMBM_WND;
		m_bIsWndChange = false;

	    m_zoomMark = false;
		m_bIsDragHaul1 = false;
		m_bIsDragHaul2 = false;

		m_bIsDrawROIMark1 = true;
		m_bIsDrawROIMark2 = false;
		m_bIsDrawROIMark3 = false;
		
		m_ROIDrawStatic = 0;
		InitializeCriticalSection(&m_CriticalSection);
		m_newWidth = 0;// 817 cly add缩放无效的问题
		m_newHeight = 0;// 817 cly add缩放无效的问题
	}

	CTREAM_MEAIA_PLY_WND::~CTREAM_MEAIA_PLY_WND()
	{
		//销毁皮肤管理器资源
		//CSkinManager::UnLoadSkin();
		//关闭Gdiplus
		if (m_toolBar)
		{
			delete m_toolBar;
			Gdiplus::GdiplusShutdown(m_toolBar->m_gdiplusToken);
			m_toolBar = NULL;
		}
		EnterCriticalSection(&m_CriticalSection);
		if (NULL != m_pImgData)
		{
			delete m_pImgData;
			m_pImgData = NULL;
		}
		m_ImgBitmap.DeleteObject(); //删除位图
		m_BackBitmap.DeleteObject(); //删除位图
		LeaveCriticalSection(&m_CriticalSection);
		DeleteCriticalSection(&m_CriticalSection);
	}

	void CTREAM_MEAIA_PLY_WND::DoDataExchange(CDataExchange* pDX)
	{
		CBaseModelDialogEx::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_STATIC_IMAGE_RECT, m_StcImgWnd);
		DDX_Control(pDX, IDC_STATIC_TOOLBAR_RECT, m_StcTbrWnd);
	}


	BEGIN_MESSAGE_MAP(CTREAM_MEAIA_PLY_WND, CBaseModelDialogEx)
		ON_WM_SIZE()
		ON_WM_SIZING()
		ON_WM_PAINT()
		ON_WM_CTLCOLOR()
		ON_WM_ERASEBKGND()
		ON_WM_MOUSEMOVE()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONUP()
		ON_WM_RBUTTONDBLCLK()
		ON_WM_MOUSEWHEEL()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_RBUTTONDOWN()
	END_MESSAGE_MAP()

	// CTREAM_MEAIA_PLY_WND 消息处理程序
	BOOL CTREAM_MEAIA_PLY_WND::OnInitDialog()
	{
		CBaseModelDialogEx::OnInitDialog();
		//创建工具条
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		m_toolBar = new VidoProToolBar();
		if (m_toolBar->Create(VidoProToolBar::IDD, this))
		{
			m_bIsToolBarCreat = TRUE;
			m_toolBar->SetParentPoint(this);
			m_toolBar->ShowWindow(SW_HIDE);
		}	
		DWORD mm = GetCurrentThreadId();

		GetClientRect(m_mainWndRect);
		m_imgRect = m_mainWndRect;
		m_wndSize = gWndSize(m_mainWndRect.Width(), m_mainWndRect.Height());

		CDC* pDC = GetDC();
		m_BackBitmap.CreateCompatibleBitmap(pDC, m_imgRect.Width(), m_imgRect.Height());
		ReleaseDC(pDC);
		return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
	}

	void CTREAM_MEAIA_PLY_WND::OnSize(UINT nType, int cx, int cy)
	{
		CBaseModelDialogEx::OnSize(nType, cx, cy);
		//获取窗体位置
		CRect wndRect(0, 0, 0, 0);
		GetClientRect(&wndRect);
		m_mainWndRect = wndRect;
		m_StcImgWnd.MoveWindow(wndRect, TRUE);
		m_imgRect = wndRect;
		m_newWidth = wndRect.Width();// 817 cly add缩放无效的问题
		m_newHeight = wndRect.Height();// 817 cly add缩放无效的问题
		//影藏toolbar控件
		if (m_bIsToolBarCreat)
		{
			m_bIsDisToolBar = FALSE;
			ToorBarShow(SW_HIDE);
		}
		m_paintType = PAINT_WND_BACK;// 817 cly add缩放无效的问题
		//Invalidate(true);
		// TODO:  在此处添加消息处理程序代码
	}

	void CTREAM_MEAIA_PLY_WND::OnSizing(UINT fwSide, LPRECT pRect)
	{
		CBaseModelDialogEx::OnSizing(fwSide, pRect);
		//影藏toolbar控件
		if (m_bIsToolBarCreat)
		{
			m_bIsDisToolBar = FALSE;
			ToorBarShow(SW_HIDE);
		}
		// TODO:  在此处添加消息处理程序代码
	}

	void CTREAM_MEAIA_PLY_WND::OnPaint()
	{
		CPaintDC dc(this); // device context for painting
		// TODO:  在此处添加消息处理程序代码
		
		CRect allViewRect(0, 0, 0, 0);
		GetClientRect(&allViewRect);
		switch (m_paintType)
		{
		case PAINT_WND_NONE:
		{
			dc.FillSolidRect(allViewRect, m_wndBackColor);
			break;
		}
		case PAINT_WND_BACK:	
		{
			////绘制窗口背景颜色
			if (m_bIsToolBarCreat)
			{
				m_bIsDisToolBar = FALSE;
				ToorBarShow(SW_HIDE);
			}
			dc.FillSolidRect(allViewRect, m_wndBackColor);
			m_StcImgWnd.MoveWindow(m_imgRect, TRUE);

			//没有工具条时的图像显示区域大小
			m_wndSize = gWndSize(allViewRect.Width(), allViewRect.Height());

			CDC* pDC;
			pDC = GetDC();
			if (m_bHadImg)
			{
				m_adaptImgDisInfo = AdaptiveImgSize(m_imgSize, m_wndSize,
					m_adaptScale, img_wnd_size_no_toolBar);
				m_adaptWndDisInfo = AdaptiveWndSize(m_imgSize, m_wndSize,
					m_fullScale, img_wnd_size_no_toolBar);
				m_fImgCurCacult = m_adaptWndDisInfo;

				m_ImgBitmap.DeleteObject();//GDI  Bug修改
				m_ImgBitmap.CreateCompatibleBitmap(pDC, (int)m_adaptWndDisInfo.width,
					(int)m_adaptWndDisInfo.height);
			}
			m_BackBitmap.DeleteObject();//GDI
			m_BackBitmap.CreateCompatibleBitmap(pDC, m_wndSize.wndWidth, m_wndSize.wndHeight);
			ReleaseDC(pDC);

			if (m_bHadImg)
			{
				BindingBitMap(); 
				m_paintType = PAINT_WND_IMG;
				AdaptDisplay(m_displayModel);	
			}
			break;
		}	
		case PAINT_WND_IMG:
		{
			//UPDATAIMG:
			//CDC* pDc = m_StcImgWnd.GetDC();
			//绘制图像
			TwoBufferDraw(&dc);
			//ReleaseDC(pDc);
			break;
		}
		default:
			break;
		}
		if (m_bIsDrawROIMark3)
		{
			//获取图像西施区域信息
			//CDC* pDc = m_StcImgWnd.GetDC();
			//绘制图像
			CurImgWndInfo info = GetImgDisRectInfo();
			DrawROI(dc, m_mousrRect, info.imgContainerStrPnt, m_ROIDrawStatic);
			//ReleaseDC(pDc);
			
		}
	}

	HBRUSH CTREAM_MEAIA_PLY_WND::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
	{
		HBRUSH hbr = CBaseModelDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

		// TODO:  在此更改 DC 的任何特性

		// TODO:  如果默认的不是所需画笔，则返回另一个画笔
		return hbr;
	}

	BOOL CTREAM_MEAIA_PLY_WND::OnEraseBkgnd(CDC* pDC)
	{
		// TODO:  在此添加消息处理程序代码和/或调用默认值
		//不刷新背景
		return TRUE/*CBaseModelDialogEx::OnEraseBkgnd(pDC)*/;
	}

	ImgSize CTREAM_MEAIA_PLY_WND::GetImgSize()
	{
		//AcquireSRWLockShared(&m_ImgSizeSRWLOCK);
		return m_imgSize;
		//ReleaseSRWLockShared(&m_ImgSizeSRWLOCK);
	}

	WndSize CTREAM_MEAIA_PLY_WND::GetWndSize()
	{
		return m_wndSize;
	}

	void  CTREAM_MEAIA_PLY_WND::ToorBarShow(int nCmdShow)
	{
		m_toolBar->ShowWindow(nCmdShow);
	}

	void  CTREAM_MEAIA_PLY_WND::ToorBarMove(CRect &wndSize)
	{
		int nWidth = wndSize.Width();
		int nHight = wndSize.Height();
		//工具条位置
		CRect toolbarRect(0, 0, 0, 0);
		//如果大于 WND_MIDD_WDTH + 2 * CNTRL_CLRNCE则使用大按钮
		if (m_bIsToolBarCreat)
		{
			if (nWidth > WND_MIDD_WDTH)
			{
				toolbarRect.SetRect(wndSize.left, wndSize.bottom - 3 * CNTRL_CLRNCE - PLAY_BTN_MAX_SIZE,
					wndSize.right, wndSize.bottom);
			}
			else
			{
				toolbarRect.SetRect(wndSize.left, wndSize.bottom - 3 * CNTRL_CLRNCE - PLAY_BTN_MIN_SIZE,
					wndSize.right, wndSize.bottom);
			}
			m_toolBar->MoveWindow(toolbarRect, TRUE);
			m_tbrRect = toolbarRect;
		}
	}

	void  CTREAM_MEAIA_PLY_WND::SetWndPaintType(ImgWndPaintType type)
	{
		m_paintType = type;
	}

	void CTREAM_MEAIA_PLY_WND::OnRButtonDblClk(UINT nFlags, CPoint point)
	{
		// TODO:  在此添加消息处理程序代码和/或调用默认值
		if (m_zoomMark)
		{
			m_zoomMark = FALSE;
		}
		else
		{
			m_zoomMark = TRUE;
			m_oldAdaptModel = m_displayModel;
		}
		CBaseModelDialogEx::OnRButtonDblClk(nFlags, point);
	}

	BOOL CTREAM_MEAIA_PLY_WND::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		if (m_bHadImg && m_zoomMark)
		{
			//m_oldAdaptModel = m_displayModel;
			m_displayModel = DISPLAY_FIXED_SCALE_DOING;
			if (zDelta > 0)
			{
				m_zoomType = ZOOM_TYPE_BLOW;
				m_changeScale = 1.05;
			}
			if (zDelta < 0)
			{
				m_zoomType = ZOOM_TYPE_SHRINK;
				m_changeScale = 0.95;
			}
			ZoomDisplay();
		}
		return CBaseModelDialogEx::OnMouseWheel(nFlags, zDelta, pt);
	}

	void CTREAM_MEAIA_PLY_WND::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		
		CBaseModelDialogEx::OnLButtonDblClk(nFlags, point);
	}

	void CTREAM_MEAIA_PLY_WND::OnRButtonDown(UINT nFlags, CPoint point)
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		BOOL cnd = (DISPLAY_ADAPTIVE_WNDSIZE != m_displayModel && DISPLAY_ADAPTIVE_IMGSIZE != m_displayModel);
		if (cnd)
		{
			if (m_oldAdaptModel == DISPLAY_ADAPTIVE_WNDSIZE)
			{
				AdaptDisplay(DISPLAY_ADAPTIVE_WNDSIZE);
				m_displayModel = DISPLAY_ADAPTIVE_WNDSIZE;
			}
			else if (m_oldAdaptModel == DISPLAY_ADAPTIVE_IMGSIZE)
			{
				AdaptDisplay(DISPLAY_ADAPTIVE_IMGSIZE);
				m_displayModel = DISPLAY_ADAPTIVE_IMGSIZE;
			}
			else
			{
				AdaptDisplay(DISPLAY_ADAPTIVE_WNDSIZE);
				m_displayModel = DISPLAY_ADAPTIVE_WNDSIZE;
			}
		}
		CBaseModelDialogEx::OnRButtonDown(nFlags, point);
	}

	void CTREAM_MEAIA_PLY_WND::OnMouseMove(UINT nFlags, CPoint point)
	{
		// TODO:  在此添加消息处理程序代码和/或调用默认值
		if (m_bIsDragHaul2 && m_bIsDragHaul1)
		{
			m_curPoint.row = (double)point.y;
			m_curPoint.col = (double)point.x;
			DragDisplay();
		}

		if (true == m_bIsDrawROIMark3)
		{
			//获取图像ROI区域信息
			CurImgWndInfo info = GetImgDisRectInfo();
			CPoint newPoint = GetImgROIArightPoint(point, info.imgContainerStrPnt, info.imgContainerSize);
			m_mousrRect.endRow = newPoint.y;
			m_mousrRect.endCol = newPoint.x;
			m_ROIDrawStatic = 0;
			//OnPaint();
			Invalidate(true); //刷新界面
		}
		CBaseModelDialogEx::OnMouseMove(nFlags, point);
	}

	void CTREAM_MEAIA_PLY_WND::OnLButtonDown(UINT nFlags, CPoint point)
	{
		BOOL cnd = (DISPLAY_ADAPTIVE_WNDSIZE == m_displayModel || DISPLAY_ADAPTIVE_IMGSIZE == m_displayModel) && m_bIsDrawROIMark1 && m_bIsDrawROIMark2;
		if (cnd)
		{
			//获取图像显示区域信息
			CurImgWndInfo info = GetImgDisRectInfo();
			//获取正确的鼠标点坐标
			CPoint newPoint = GetImgROIArightPoint(point, info.imgContainerStrPnt, info.imgContainerSize);
			m_mousrRect.StarRow = newPoint.y;
			m_mousrRect.StarCol = newPoint.x;
			ImgSize imgSize = GetImgSize();
			//获取窗口到图像的映射坐标
			MousePoint transPoint = CalculateWpToIp(newPoint, info.imgContainerStrPnt, info.imgContainerSize, imgSize);
			m_wndRectToImgRect.StarRow = (int)transPoint.ImgRow;
			m_wndRectToImgRect.StarCol = (int)transPoint.ImgCol;
			m_bIsDrawROIMark3 = true;
		}
		if (m_bIsDragHaul1)
		{

			m_bIsDragHaul2 = true;
			m_refPoint.row = (double)point.y;
			m_refPoint.col = (double)point.x;
		}
		// TODO:  在此添加消息处理程序代码和/或调用默认值
		CBaseModelDialogEx::OnLButtonDown(nFlags, point);
	}

	void CTREAM_MEAIA_PLY_WND::OnLButtonUp(UINT nFlags, CPoint point)
	{
		// TODO:  在此添加消息处理程序代码和/或调用默认值
		if (true == m_bIsDrawROIMark3)
		{
			//获取图像ROI区域信息
			CurImgWndInfo info = GetImgDisRectInfo();
			//获取正确的鼠标点坐标
			CPoint newPoint = GetImgROIArightPoint(point, info.imgContainerStrPnt, info.imgContainerSize);
			m_mousrRect.endRow = newPoint.y;
			m_mousrRect.endCol = newPoint.x;
			ImgSize imgSize = GetImgSize();
			//获取窗口到图像的映射坐标
			MousePoint transPoint = CalculateWpToIp(newPoint, info.imgContainerStrPnt, info.imgContainerSize, imgSize);
			m_wndRectToImgRect.endRow = (int)transPoint.ImgRow;
			m_wndRectToImgRect.endCol = (int)transPoint.ImgCol;
			m_ROIDrawStatic = 1;
			Invalidate(true); //刷新界面

			RequstCallMsg* msg = new RequstCallMsg;
			msg->requestType = REQUEST_TYPE_ROI_GET;
			msg->roiRect = m_wndRectToImgRect;
			m_callbak((WPARAM)msg, (LPARAM)m_wndId, m_pParent);
			m_bIsDrawROIMark2 = false;
		}
		m_bIsDrawROIMark3 = false;
		m_bIsDragHaul2 = false;
		//出现工具栏控制
		if ((DISPLAY_ADAPTIVE_WNDSIZE == m_displayModel || DISPLAY_ADAPTIVE_IMGSIZE == m_displayModel))
		{
			if (m_bIsDisToolBar)
			{
				m_paintType = PAINT_WND_NONE;
				m_bIsDisToolBar = FALSE;
				ToorBarShow(SW_HIDE);

				m_StcImgWnd.MoveWindow(m_imgRect, TRUE);
			}
			else
			{
				m_paintType = PAINT_WND_NONE;
				CRect wndRect(0, 0, 0, 0);
				GetWindowRect(&wndRect);
				//移动toolbar
				ToorBarMove(wndRect);
				ToorBarShow(SW_SHOW);
				m_bIsDisToolBar = TRUE;

				CRect oldRect = m_imgRect[img_wnd_size_no_toolBar];
			}
			OnPaint();
			m_paintType = PAINT_WND_IMG;
		}
		CBaseModelDialogEx::OnLButtonUp(nFlags, point);
	}
		/*****************************************************************************************************************************/
}; 






