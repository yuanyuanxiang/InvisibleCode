/**
* @file MainFrm.h 
* @brief CMainFrame 类的接口
*/

#pragma once
#include "..\3DCode\3DCodePane.h"

class CDlgEncode;

/// 编码对话框上面视图的ID
#define ID_VIEW_ENCODE               10000

/// 解码对话框上面视图的ID
#define ID_VIEW_DECODE               10001

/**
* @class CMainFrame
* @brief 主框架
*/
class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CMFCToolBar			m_wndToolBar;		/**< 工具栏 */
	CMFCStatusBar		m_wndStatusBar;		/**< 状态栏 */
	C3DCodePane			m_paneEncode;		/**< 编码面板 */

	/// 编码对话框
	CDlgEncode*			m_pDlgEncode;

	// 创建PANE
	BOOL CreateDockingWindows();

public:
	/// 获取编码对话框
	CDlgEncode* GetEncodeDlg(){ return m_pDlgEncode; }

	/// 获取状态栏引用
	CMFCStatusBar& GetStatusBar() { return m_wndStatusBar; }

	// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEncode();
	afx_msg void OnUpdateEncode(CCmdUI *pCmdUI);
};