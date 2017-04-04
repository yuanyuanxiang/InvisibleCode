
// InvisibleCodeView.cpp : CInvisibleCodeView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "InvisibleCode.h"
#endif

#include "InvisibleCodeDoc.h"
#include "InvisibleCodeView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CInvisibleCodeView

IMPLEMENT_DYNCREATE(CInvisibleCodeView, CScrollView)

BEGIN_MESSAGE_MAP(CInvisibleCodeView, CScrollView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_DECODE, &CInvisibleCodeView::OnDecode)
	ON_UPDATE_COMMAND_UI(ID_DECODE, &CInvisibleCodeView::OnUpdateDecode)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


/// 将坐标上下翻转
void FlipPosition(float2 position[], int K, int nHeight)
{
	for (int i = 0; i < K; ++i)
	{
		position[i].y = nHeight - position[i].y;
	}
}


// CInvisibleCodeView 构造/析构

CInvisibleCodeView::CInvisibleCodeView()
{
	// TODO: 在此处添加构造代码

}

CInvisibleCodeView::~CInvisibleCodeView()
{
}

BOOL CInvisibleCodeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CScrollView::PreCreateWindow(cs);
}

// CInvisibleCodeView 绘制

void CInvisibleCodeView::OnDraw(CDC* pDC)
{
	CInvisibleCodeDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (!pDoc->m_Image.IsNull())
	{
		int nWidth = pDoc->m_Image.GetWidth();
		int nHeight = pDoc->m_Image.GetHeight();
		pDoc->m_Image.Draw(pDC->GetSafeHdc(), 0, 0, nWidth, nHeight);
		float2 m_Centers[4];
		memcpy(m_Centers, pDoc->m_Centers, 4 * sizeof(float2));
		FlipPosition(m_Centers, 4, nHeight);
		pDC->Ellipse((int)m_Centers[0].x - 4, (int)m_Centers[0].y - 4, (int)m_Centers[0].x + 4, (int)m_Centers[0].y + 4);
		pDC->Ellipse((int)m_Centers[1].x - 4, (int)m_Centers[1].y - 4, (int)m_Centers[1].x + 4, (int)m_Centers[1].y + 4);
		pDC->Ellipse((int)m_Centers[2].x - 4, (int)m_Centers[2].y - 4, (int)m_Centers[2].x + 4, (int)m_Centers[2].y + 4);
		pDC->Ellipse((int)m_Centers[3].x - 4, (int)m_Centers[3].y - 4, (int)m_Centers[3].x + 4, (int)m_Centers[3].y + 4);
	}
}

void CInvisibleCodeView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: 计算此视图的合计大小
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


// CInvisibleCodeView 打印

BOOL CInvisibleCodeView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CInvisibleCodeView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CInvisibleCodeView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CInvisibleCodeView 诊断

#ifdef _DEBUG
void CInvisibleCodeView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CInvisibleCodeView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CInvisibleCodeDoc* CInvisibleCodeView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CInvisibleCodeDoc)));
	return (CInvisibleCodeDoc*)m_pDocument;
}
#endif //_DEBUG


// CInvisibleCodeView 消息处理程序


// "解码"工具栏
void CInvisibleCodeView::OnDecode()
{
	CInvisibleCodeDoc *pDoc = GetDocument();
	pDoc->Decode();
	Invalidate(TRUE);
}


// 更新工具栏图标状态
void CInvisibleCodeView::OnUpdateDecode(CCmdUI *pCmdUI)
{
	CInvisibleCodeDoc *pDoc = GetDocument();
	CyImage *pImage = pDoc->GetImage();
	pCmdUI->Enable(!pImage->IsNull());
}


// 更新滚动视图大小
void CInvisibleCodeView::UpdateScrollSize()
{
	CInvisibleCodeDoc *pDoc = GetDocument();
	SetScrollSizes(MM_TEXT, pDoc->GetImageSize());
}


// 显示鼠标坐标和像素的RGB值
void CInvisibleCodeView::OnMouseMove(UINT nFlags, CPoint point)
{
	CString str;
	CDC* pDC = GetDC();
	COLORREF ref = GetPixel(pDC->GetSafeHdc(), point.x, point.y);
	int R = GetRValue(ref), G = GetGValue(ref), B = GetBValue(ref);
	float Gray = RgbColorRef2Gray(ref);
	str.Format(_T("x = %4d, y = %4d, g = %.2f, RGB(%3d, %3d, %3d)"), 
		point.x, point.y, Gray, R, G, B);
	CMainFrame* pMainFrm = (CMainFrame*) AfxGetApp()->GetMainWnd();
	pMainFrm->GetStatusBar().SetWindowText(str);

	CScrollView::OnMouseMove(nFlags, point);
}
