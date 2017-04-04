/************************************************************************
* @file Encodefuncs.h
* @brief 隐形二维码的编码函数
* @author 袁沅祥
* @version 1.0
* @date 2016/9/18
*************************************************************************/


#include "..\3DCode\DataTypes.h"
#include "..\3DCode\CyImage.h"

/// 如果a的i位为1，则设置b的j个元素为1
#define BIT_CHECK_SET_ELEM(a, i, b, j) if(BIT_CHECK(a, i)) b[j] = 1;

/// 如果a的i个元素为1，则设置b的(r, c)个元素为x
#define ELEM_CHECK_SET(a, i, b, r, c, x) if(a[i]) b[r][c] = x;

/// 位置元标签
#define POSITION_TAG 2

/// 彩色元标签
#define COLOR_TAG 1

/// 背景元标签
#define BACKGROUND_TAG 0

// 编码定位元
void EncodePositionElem(int nCodes[81][81], int nSymbolSize);

// 编码数据头
void EncodeDataHeader(int nCodes[81][81], int nSymbolSize, int nVersion, int nDataFormat, int nDataLength, int nEcLevel, int nMaskingNo);

// 编码数据
void EncodeData(int nCodes[81][81], int nSymbolSize, int *code_bits, int bitsLen, int nMaskingNo);

// 编码隐形二维码
int EncodeInvisibleCode(int nCodes[81][81], char *pUtf8, int nVersion, int nDataFormat, int nDataLength, int nEcLevel, int nMaskingNo);

// 确定符号版本
int AutoVersion(int nBitsLength);

// 由二维码矩阵创建图像
int Convert2Image(CyImage *pImage, int nCodes[81][81], int nSymbolSize, COLORREF cPositionElem = RGB(255, 0, 0), COLORREF cColorElem = RGB(0, 255, 0), COLORREF cBackgroundElem = RGB(255, 255, 255), int nPixelSize = 4);

// 绘制位置探测图形
void PaintPositionPartten(BYTE* pHead, int nWidth, int nHeight, int nRowlen, int nChannel, CRect rect, int R, int G, int B, int nPixelSize = 4);

// 判断特定大小的符号可编码
BOOL CodeAble(int nSymbolSize, int nBitsLen);