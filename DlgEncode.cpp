// DlgEncode.cpp : 实现文件
//

#include "stdafx.h"
#include "InvisibleCode.h"
#include "DlgEncode.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "InvisibleCodeView.h"
#include "InvisibleCodeDoc.h"

#include "Encodefuncs.h"
#include "../3DCode/CodeTransform.h"

// CDlgEncode 对话框

IMPLEMENT_DYNAMIC(CDlgEncode, CDialogEx)

CDlgEncode::CDlgEncode(CWnd* pParent) : CDialogEx(CDlgEncode::IDD, pParent)
{
	m_nVersion = 0;
	m_nDataFormat = 0;
	m_nDataLength = 0;
	m_nEcLevel = 0;
	m_nMaskingNo = 0;
	ZeroMemory(m_nCodes, 81 * 81 * sizeof(int));
	m_nSymbolSize = 0;
	m_cPositionElem = RGB(255, 0, 0);
	m_cColorElem = RGB(0, 255, 0);
	m_cBackgroundElem = RGB(255, 255, 255);
	m_nPixelSize = 4;
	m_pImage = NULL;
	m_bAutoVersion = FALSE;
	m_bAutoMaskingNo = FALSE;
}

CDlgEncode::~CDlgEncode()
{
}

void CDlgEncode::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_VERSION, m_ComboBoxVersion);
	DDX_Control(pDX, IDC_COMBO_MASKING_NO, m_ComboBoxMaskingNo);
	DDX_Control(pDX, IDC_COMBO_DATA_FORMAT, m_ComboBoxDataFormat);
	DDX_Control(pDX, IDC_COMBO_EC_LEVEL, m_ComboBoxEcLevel);
	DDX_Control(pDX, IDC_STATIC_IMAGE_SIZE, m_StaticImageSize);
	DDX_Control(pDX, IDC_AUTO_VERSION, m_ButtonAutoVersion);
	DDX_Control(pDX, IDC_AUTO_MASKING_NO, m_ButtonAutoMaskingNo);
	DDX_Control(pDX, IDC_BUTTON_POSITION_ELEM, m_ButtonPositionElem);
	DDX_Control(pDX, IDC_BUTTON_COLOR_ELEM, m_ButtonColorElem);
	DDX_Control(pDX, IDC_BUTTON_BACKGROUND_ELEM, m_ButtonBackgroundElem);
	DDX_Control(pDX, IDC_DATA, m_EditData);
	DDX_Control(pDX, IDC_EDIT_PIXEL_SIZE, m_EditPixelSize);
	DDX_Text(pDX, IDC_EDIT_PIXEL_SIZE, m_nPixelSize);
	DDV_MinMaxInt(pDX, m_nPixelSize, 1, 1024);
	DDX_Control(pDX, IDC_POSITION_ELEM_R, m_EditPositionR);
	DDX_Control(pDX, IDC_POSITION_ELEM_G, m_EditPositionG);
	DDX_Control(pDX, IDC_POSITION_ELEM_B, m_EditPositionB);
	DDX_Control(pDX, IDC_COLOR_ELEM_R, m_EditColorR);
	DDX_Control(pDX, IDC_COLOR_ELEM_G, m_EditColorG);
	DDX_Control(pDX, IDC_COLOR_ELEM_B, m_EditColorB);
	DDX_Control(pDX, IDC_BACKGROUND_ELEM_R, m_EditBackgroundR);
	DDX_Control(pDX, IDC_BACKGROUND_ELEM_G, m_EditBackgroundG);
	DDX_Control(pDX, IDC_BACKGROUND_ELEM_B, m_EditBackgroundB);
	DDX_CBIndex(pDX, IDC_COMBO_VERSION, m_nVersion);
	DDX_CBIndex(pDX, IDC_COMBO_MASKING_NO, m_nMaskingNo);
	DDX_CBIndex(pDX, IDC_COMBO_DATA_FORMAT, m_nDataFormat);
	DDX_CBIndex(pDX, IDC_COMBO_EC_LEVEL, m_nEcLevel);
	DDX_Check(pDX, IDC_AUTO_VERSION, m_bAutoVersion);
	DDX_Check(pDX, IDC_AUTO_MASKING_NO, m_bAutoMaskingNo);
}


BEGIN_MESSAGE_MAP(CDlgEncode, CDialogEx)
	ON_EN_CHANGE(IDC_DATA, &CDlgEncode::OnChangeData)
	ON_EN_CHANGE(IDC_EDIT_PIXEL_SIZE, &CDlgEncode::OnEnChangeEditPixelSize)
	ON_CBN_SELCHANGE(IDC_COMBO_VERSION, &CDlgEncode::OnCbnSelchangeComboVersion)
	ON_CBN_SELCHANGE(IDC_COMBO_MASKING_NO, &CDlgEncode::OnCbnSelchangeComboMaskingNo)
	ON_CBN_SELCHANGE(IDC_COMBO_DATA_FORMAT, &CDlgEncode::OnCbnSelchangeComboDataFormat)
	ON_CBN_SELCHANGE(IDC_COMBO_EC_LEVEL, &CDlgEncode::OnCbnSelchangeComboEcLevel)
	ON_BN_CLICKED(IDC_AUTO_VERSION, &CDlgEncode::OnBnClickedAutoVersion)
	ON_BN_CLICKED(IDC_AUTO_MASKING_NO, &CDlgEncode::OnBnClickedAutoMaskingNo)
END_MESSAGE_MAP()


// CDlgEncode 消息处理程序


BOOL CDlgEncode::Encode()
{
	CString strData;
	m_EditData.GetWindowText(strData);
	char *pUtf8 = UnicodeConvert2UTF8(strData, m_nDataLength);
	if (pUtf8 == NULL)
		return FALSE;
	m_nVersion = m_ComboBoxVersion.GetCurSel();
	m_nDataFormat = m_ComboBoxDataFormat.GetCurSel();
	m_nEcLevel = m_ComboBoxEcLevel.GetCurSel();
	m_nMaskingNo = m_ComboBoxMaskingNo.GetCurSel();
	ZeroMemory(m_nCodes, 81 * 81 * sizeof(int));
	m_nSymbolSize = EncodeInvisibleCode(m_nCodes, pUtf8, m_nVersion, m_nDataFormat, m_nDataLength, m_nEcLevel, m_nMaskingNo);
	SAFE_DELETE(pUtf8);
	// 更新组合框的版本
	m_nVersion = m_nSymbolSize - 18;
	CString str;
	str.Format(_T("%d"), m_nVersion);
	m_ComboBoxVersion.SetWindowText(str);
	// 更新文档/视图
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CChildFrame* pChildFrame = (CChildFrame*)pMainFrame->GetActiveFrame();
	CInvisibleCodeDoc* pDoc = (CInvisibleCodeDoc*)pChildFrame->GetActiveDocument();
	CInvisibleCodeView* pView = (CInvisibleCodeView*)pChildFrame->GetActiveView();
	m_pImage = pDoc->GetImage();
	int nImageSize = Convert2Image(m_pImage, m_nCodes, m_nSymbolSize, m_cPositionElem, m_cColorElem, m_cBackgroundElem, m_nPixelSize);
	if (!m_pImage->IsNull())
	{
		CString str;
		str.Format(_T("%dx%d"), nImageSize, nImageSize);
		m_StaticImageSize.SetWindowText(str);
		pView->Invalidate(TRUE);
	}
	return TRUE;
}


void CDlgEncode::OnChangeData()
{
	Encode();
}


BOOL CDlgEncode::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 符号版本
	for (int i = 0; i < 64; ++i)
	{
		m_ComboBoxVersion.InsertString(i, Num2String(i));
	}
	m_ComboBoxVersion.SetCurSel(0);

	// 掩码模式
	for (int i = 0; i < 8; ++i)
	{
		m_ComboBoxMaskingNo.InsertString(i, Num2String(i));
	}
	m_ComboBoxMaskingNo.SetCurSel(0);

	// 数据格式
	for (int i = 0; i < 4; ++i)
	{
		m_ComboBoxDataFormat.InsertString(i, Num2String(i));
	}
	m_ComboBoxDataFormat.SetCurSel(0);
	
	// 纠错等级
	for (int i = 0; i < 7; ++i)
	{
		m_ComboBoxEcLevel.InsertString(i, Num2String(i + 1));
	}
	m_ComboBoxEcLevel.SetCurSel(2);

	m_ButtonAutoVersion.SetCheck(TRUE);
	m_ButtonAutoMaskingNo.SetCheck(TRUE);
	m_EditPixelSize.SetWindowText(_T("4"));
	m_StaticImageSize.SetWindowText(_T("0x0"));
	
	// 定位元/彩色元/背景元
	m_EditPositionR.SetWindowText(Num2String(0));
	m_EditPositionG.SetWindowText(Num2String(0));
	m_EditPositionB.SetWindowText(Num2String(255));
	m_EditColorR.SetWindowText(Num2String(0));
	m_EditColorG.SetWindowText(Num2String(255));
	m_EditColorB.SetWindowText(Num2String(0));
	m_EditBackgroundR.SetWindowText(Num2String(255));
	m_EditBackgroundG.SetWindowText(Num2String(255));
	m_EditBackgroundB.SetWindowText(Num2String(255));

	return TRUE;
}


void CDlgEncode::OnEnChangeEditPixelSize()
{
	UpdateData(TRUE);
	Encode();
}


void CDlgEncode::OnCbnSelchangeComboVersion()
{
	UpdateData(TRUE);
	Encode();
}


void CDlgEncode::OnCbnSelchangeComboMaskingNo()
{
	UpdateData(TRUE);
	Encode();
}


void CDlgEncode::OnCbnSelchangeComboDataFormat()
{
	UpdateData(TRUE);
	Encode();
}


void CDlgEncode::OnCbnSelchangeComboEcLevel()
{
	UpdateData(TRUE);
	Encode();
}


void CDlgEncode::OnBnClickedAutoVersion()
{
	UpdateData(TRUE);
	Encode();
}


void CDlgEncode::OnBnClickedAutoMaskingNo()
{
	UpdateData(TRUE);
	Encode();
}


void CDlgEncode::OnOK()
{
	// 屏蔽RETURN
	return;
}


BOOL CDlgEncode::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}