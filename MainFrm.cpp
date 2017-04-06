
// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "InvisibleCode.h"

#include "MainFrm.h"
#include "DlgEncode.h"
#include <afxmdiframewndex.h>
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_ENCODE, &CMainFrame::OnEncode)
	ON_UPDATE_COMMAND_UI(ID_ENCODE, &CMainFrame::OnUpdateEncode)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	m_pDlgEncode = NULL;
}

CMainFrame::~CMainFrame()
{
	SAFE_DELETE(m_pDlgEncode);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("未能创建工具栏\n");
		return -1;      // 未能创建
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: 如果不需要可停靠工具栏，则删除这三行
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndToolBar);

	// 创建停靠窗口
	if (!CreateDockingWindows())
	{
		TRACE0("未能创建停靠窗口\n");
		return -1;
	}

	m_paneEncode.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_paneEncode);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序

BOOL CMainFrame::CreateDockingWindows()
{
	CRect ChildRect;

	// 创建编码对话框
	m_pDlgEncode = new CDlgEncode;
	m_pDlgEncode->Create(IDD_ENCODE, this);
	m_pDlgEncode->GetWindowRect(&ChildRect);
	// 使得对话框足够大，以覆盖整个View
	m_pDlgEncode->MoveWindow(0, 0, 1920, 1080);
	m_pDlgEncode->ShowWindow(SW_SHOW);

	// 创建输出窗口
	if (!m_paneEncode.Create(_T("编码"), this, CRect(0, 0, ChildRect.Width(), ChildRect.Height()), TRUE, ID_VIEW_ENCODE, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("未能创建编码窗口\n");
		return FALSE; // 未能创建
	}
	m_paneEncode.m_pEncodeView->AddChildWnd(m_pDlgEncode, ChildRect.Width(), ChildRect.Height());
	// 2015年12月11日注释：“编码”对话框必须设置为“Child”，否则在strcore.cpp出现错误

	return TRUE;
}


void CMainFrame::OnEncode()
{
	if (m_paneEncode.IsWindowVisible())
	{
		m_paneEncode.ShowPane(FALSE, FALSE, FALSE);
	}
	else
	{
		m_paneEncode.ShowPane(TRUE, FALSE, TRUE);
	}
}


void CMainFrame::OnUpdateEncode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_paneEncode.IsPaneVisible());
}
