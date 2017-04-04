#pragma once

/**
* @file DlgEncode.h
* @brief 编码对话框
*/

#include "afxwin.h"
#include "..\3DCode\CyImage.h"

#ifndef IDD_ENCODE
#define IDD_ENCODE 310
#endif

/** 
* @class CDlgEncode 
* @brief 编码对话框
*/
class CDlgEncode : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEncode)

public:
	CDlgEncode(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgEncode();

	int m_nVersion;
	int m_nDataFormat;
	int m_nDataLength;
	int m_nEcLevel;
	int m_nMaskingNo;
	int m_nCodes[81][81];
	int m_nSymbolSize;
	COLORREF m_cPositionElem;
	COLORREF m_cColorElem;
	COLORREF m_cBackgroundElem;
	int m_nPixelSize;
	CyImage *m_pImage;
	BOOL m_bAutoVersion;
	BOOL m_bAutoMaskingNo;

	// 编码
	BOOL Encode();

// 对话框数据
	enum { IDD = IDD_ENCODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_ComboBoxVersion;
	CComboBox m_ComboBoxMaskingNo;
	CComboBox m_ComboBoxDataFormat;
	CComboBox m_ComboBoxEcLevel;
	CEdit m_EditPixelSize;
	CStatic m_StaticImageSize;
	CButton m_ButtonAutoVersion;
	CButton m_ButtonAutoMaskingNo;
	CButton m_ButtonPositionElem;
	CButton m_ButtonColorElem;
	CButton m_ButtonBackgroundElem;
	afx_msg void OnChangeData();
	virtual BOOL OnInitDialog();
	CEdit m_EditData;
	CEdit m_EditPositionR;
	CEdit m_EditPositionG;
	CEdit m_EditPositionB;
	CEdit m_EditColorR;
	CEdit m_EditColorG;
	CEdit m_EditColorB;
	CEdit m_EditBackgroundR;
	CEdit m_EditBackgroundG;
	CEdit m_EditBackgroundB;
	afx_msg void OnEnChangeEditPixelSize();
	afx_msg void OnCbnSelchangeComboVersion();
	afx_msg void OnCbnSelchangeComboMaskingNo();
	afx_msg void OnCbnSelchangeComboDataFormat();
	afx_msg void OnCbnSelchangeComboEcLevel();
	afx_msg void OnBnClickedAutoVersion();
	afx_msg void OnBnClickedAutoMaskingNo();
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};