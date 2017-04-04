
// InvisibleCodeDoc.cpp : CInvisibleCodeDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "InvisibleCode.h"
#endif

#include "InvisibleCodeDoc.h"
#include "InvisibleCodeView.h"
#include <propkey.h>
#include "DecodeFuncs.h"
#include "..\3DCode\CodeTransform.h"
#include "..\3DCode\AfxGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CInvisibleCodeDoc

IMPLEMENT_DYNCREATE(CInvisibleCodeDoc, CDocument)

BEGIN_MESSAGE_MAP(CInvisibleCodeDoc, CDocument)
END_MESSAGE_MAP()


// CInvisibleCodeDoc 构造/析构

CInvisibleCodeDoc::CInvisibleCodeDoc()
{
	// TODO: 在此添加一次性构造代码

}

CInvisibleCodeDoc::~CInvisibleCodeDoc()
{
}

BOOL CInvisibleCodeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CInvisibleCodeDoc 序列化

void CInvisibleCodeDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CInvisibleCodeDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CInvisibleCodeDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:  strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CInvisibleCodeDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CInvisibleCodeDoc 诊断

#ifdef _DEBUG
void CInvisibleCodeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CInvisibleCodeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CInvisibleCodeDoc 命令


BOOL CInvisibleCodeDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (m_Image.IsNull())
		return CDocument::OnSaveDocument(lpszPathName);
	return 	SUCCEEDED(m_Image.Save(lpszPathName, Gdiplus::ImageFormatBMP));
}


BOOL CInvisibleCodeDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	HRESULT hr = m_Image.Load(lpszPathName);
	BOOL success = SUCCEEDED(hr);
	if (success)
	{
		CInvisibleCodeView *pView = GetView();
		pView->UpdateScrollSize();
	}
	return success;
}


/// 解析二维码
void CInvisibleCodeDoc::Decode()
{
	if (m_Image.IsNull())
		return;

	BYTE* pHead = m_Image.GetHeadAddress();
	int nWidth = m_Image.GetWidth();
	int nHeight = m_Image.GetHeight();
	int nRowlen = m_Image.GetRowlen();
	int nChannel = m_Image.GetChannel();
	RoiRect roi = RoiRect(0, 0, 0, 0);
	COLORREF cPositionElem = RGB(255, 0, 0);
	COLORREF cColorElem = RGB(0, 255, 0);
	COLORREF cBackgroundElem = RGB(255, 255, 255);

	// 解码二维码图像
	int nVersion, nDataFormat, nDataLength, nEcLevel, nMaskingNo;
	char * result = DecodeImage(pHead, nWidth, nHeight, nRowlen, nChannel, roi, 
		cPositionElem, cColorElem, cBackgroundElem, 
		m_Centers, nVersion, nDataFormat, nDataLength, nEcLevel, nMaskingNo);
	if (result)
	{
		CString str = UTF8Convert2Unicode(result, nDataLength);
		AfxMessageBox(str, MB_OK | MB_ICONINFORMATION);
		SAFE_DELETE(result);
	}
}


/// 获取文档对应的CQrEncodeView活动视图
CInvisibleCodeView* CInvisibleCodeDoc::GetView()
{
	CView* pView = NULL;
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CInvisibleCodeView)))
			return (CInvisibleCodeView*)pView;
		else
			continue;
	}
	if (pView->IsKindOf(RUNTIME_CLASS(CInvisibleCodeView)))
		return (CInvisibleCodeView*)pView;
	else
	{
		TRACE(" * 不能定位视图。\n");
		return NULL;
	}
}


/// 返回CyImage地址
CyImage* CInvisibleCodeDoc::GetImage()
{
	return &m_Image;
}


/// 返回CSize = (nWidth, nHeight)
CSize CInvisibleCodeDoc::GetImageSize() const
{
	if (m_Image.IsNull())
		return CSize(0, 0);
	return CSize(m_Image.GetWidth(), m_Image.GetHeight());
}