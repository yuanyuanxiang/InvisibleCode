/**
* @file InvisibleCodeDoc.h 
* @brief CInvisibleCodeDoc 类的接口
*/


#pragma once
#include "..\3DCode\CyImage.h"

class CInvisibleCodeView;

/**
* @class CInvisibleCodeDoc
* @brief InvisibleCode文档
*/
class CInvisibleCodeDoc : public CDocument
{
protected: // 仅从序列化创建
	CInvisibleCodeDoc();
	DECLARE_DYNCREATE(CInvisibleCodeDoc)

// 特性
public:
	/// 图像对象
	CyImage m_Image;

	/// 聚类中心
	float2 m_Centers[4];

	/// 获取图像指针
	CyImage* GetImage();

	/// 获取图像大小
	CSize GetImageSize() const;

	/// 获取活动视图
	CInvisibleCodeView* GetView();

	/// 解析二维码
	void Decode();

// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CInvisibleCodeDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
};