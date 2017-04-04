/**
* @file InvisibleCodeView.h 
* @brief CInvisibleCodeView 类的接口
*/

#pragma once

class CInvisibleCodeDoc;

/**
* @class CInvisibleCodeView
* @brief InvisibleCode视图
*/
class CInvisibleCodeView : public CScrollView
{
protected: // 仅从序列化创建
	CInvisibleCodeView();
	DECLARE_DYNCREATE(CInvisibleCodeView)

// 特性
public:
	CInvisibleCodeDoc* GetDocument() const;

	/// 更新滚动视图大小
	void UpdateScrollSize();

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // 构造后第一次调用
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CInvisibleCodeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDecode();
	afx_msg void OnUpdateDecode(CCmdUI *pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // InvisibleCodeView.cpp 中的调试版本
inline CInvisibleCodeDoc* CInvisibleCodeView::GetDocument() const
   { return reinterpret_cast<CInvisibleCodeDoc*>(m_pDocument); }
#endif

// 将坐标上下翻转
void FlipPosition(float2 position[], int K, int nHeight);