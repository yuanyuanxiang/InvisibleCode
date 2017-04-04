#include "stdafx.h"
#include "Encodefuncs.h"
#include <cmath>
#include "..\3DCode\ColorsMask.h"
#include "..\3DCode\reedSolomon.h"
#include "..\3DCode\RS_ecc.h"
#include "..\3DCode\PixelsOperation.h"

/** 
* @brief 编码定位元.
* @details 该函数编码位置探测元.
* @param[out] nCodes[81][81] 二维码
* @param[in] nSymbolSize 符号大小
* @return 无
* @retval void
*/
void EncodePositionElem(int nCodes[81][81], int nSymbolSize)
{
	// i表示列，j表示行
	// 左下角位置探测元
	for (int j = 0; j < 7; ++j)
	{
		for (int i = 0; i < 7; ++i)
		{
			nCodes[j][i] = POSITION_TAG;
		}
	}
	// 左上角位置探测元
	for (int j = nSymbolSize - 5; j < nSymbolSize; ++j)
	{
		for (int i = 0; i < 5; ++i)
		{
			nCodes[j][i] = POSITION_TAG;
		}
	}
	// 右下角位置探测元
	for (int j = 0; j < 5; ++j)
	{
		for (int i = nSymbolSize - 5; i < nSymbolSize; ++i)
		{
			nCodes[j][i] = POSITION_TAG;
		}
	}
	// 右上角位置探测元
	for (int j = nSymbolSize - 5; j < nSymbolSize; ++j)
	{
		for (int i = nSymbolSize - 5; i < nSymbolSize; ++i)
		{
			nCodes[j][i] = POSITION_TAG;
		}
	}
}


/** 
* @brief 编码数据头. 
* @details 该函数编码数据头. 
* @param[out] nCodes[81][81] 二维码
* @param[in] nSymbolSize 符号大小
* @param[in] nVersion 版本大小
* @param[in] nDataFormat 数据格式
* @param[in] nDataLength 数据长度
* @param[in] nEcLevel 纠错等级
* @param[in] nMaskingNo 掩码模式
* @return 无
* @retval void
*/
void EncodeDataHeader(int nCodes[81][81], int nSymbolSize, int nVersion, int nDataFormat, int nDataLength, int nEcLevel, int nMaskingNo)
{
	int nHeader[24];
	memset(nHeader, 0, 24 * sizeof(int));
	// 1-6位：编码版本，从0到63
	BIT_CHECK_SET_ELEM(nVersion, 0, nHeader, 0);
	BIT_CHECK_SET_ELEM(nVersion, 1, nHeader, 1);
	BIT_CHECK_SET_ELEM(nVersion, 2, nHeader, 2);
	BIT_CHECK_SET_ELEM(nVersion, 3, nHeader, 3);
	BIT_CHECK_SET_ELEM(nVersion, 4, nHeader, 4);
	BIT_CHECK_SET_ELEM(nVersion, 5, nHeader, 5);
	// 7-8位：数据格式
	BIT_CHECK_SET_ELEM(nDataFormat, 0, nHeader, 6);
	BIT_CHECK_SET_ELEM(nDataFormat, 1, nHeader, 7);
	// 9-18位：数据长度
	BIT_CHECK_SET_ELEM(nDataLength, 0, nHeader, 8);
	BIT_CHECK_SET_ELEM(nDataLength, 1, nHeader, 9);
	BIT_CHECK_SET_ELEM(nDataLength, 2, nHeader, 10);
	BIT_CHECK_SET_ELEM(nDataLength, 3, nHeader, 11);
	BIT_CHECK_SET_ELEM(nDataLength, 4, nHeader, 12);
	BIT_CHECK_SET_ELEM(nDataLength, 5, nHeader, 13);
	BIT_CHECK_SET_ELEM(nDataLength, 6, nHeader, 14);
	BIT_CHECK_SET_ELEM(nDataLength, 7, nHeader, 15);
	BIT_CHECK_SET_ELEM(nDataLength, 8, nHeader, 16);
	BIT_CHECK_SET_ELEM(nDataLength, 9, nHeader, 17);
	// 19-21位：纠错等级
	BIT_CHECK_SET_ELEM(nEcLevel, 0, nHeader, 18);
	BIT_CHECK_SET_ELEM(nEcLevel, 1, nHeader, 19);
	BIT_CHECK_SET_ELEM(nEcLevel, 2, nHeader, 20);
	// 22-24位：掩码模式
	BIT_CHECK_SET_ELEM(nMaskingNo, 0, nHeader, 21);
	BIT_CHECK_SET_ELEM(nMaskingNo, 1, nHeader, 22);
	BIT_CHECK_SET_ELEM(nMaskingNo, 2, nHeader, 23);

	// 调试时输出
#ifdef _DEBUG
	TRACE("* Encoded dataheader:\n");
	for (int i = 0; i < 24; ++i)
	{
		TRACE("%d, ", nHeader[i]);
	}
	TRACE("\n");
#endif

	// 24bits合并为12个码字
	MergedIntBy2Bits(nHeader, 24);
	// 将数据头进行编码
	int temp[3], encoded_header[72];
	reedSolomon rs(2, 1);
	for (int j = 0; j < 12; ++j)
	{
		ZeroMemory(temp, 3 * sizeof(int));
		temp[0] = nHeader[j];
		rs.rs_encode(temp);
		for (int i = 0; i < 3; ++i)
		{
			encoded_header[6 * j + 2 * i    ] = 0x00000001 &  temp[i];
			encoded_header[6 * j + 2 * i + 1] = 0x00000001 & (temp[i]>>1);
		}
	}
	// 数据头的掩码
	int HEADER_MASKINGS[72] = {
		0, 0, 1, 0, 0, 1, 0, 0, 1,
		0, 0, 1, 0, 0, 1, 0, 0, 1,
		0, 0, 1, 0, 0, 1, 0, 0, 1,
		0, 0, 1, 0, 0, 1, 0, 0, 1,
		0, 0, 1, 0, 0, 1, 0, 0, 1,
		0, 0, 1, 0, 0, 1, 0, 0, 1,
		0, 0, 1, 0, 0, 1, 0, 0, 1,
		0, 0, 1, 0, 0, 1, 0, 0, 1};
	XOR(encoded_header, HEADER_MASKINGS, 72);
	// 右下角
	int k = 0;
	for (int j = 0; j < 7; ++j)
	{
		ELEM_CHECK_SET(encoded_header, k++, nCodes, j, nSymbolSize - 7, COLOR_TAG);
		ELEM_CHECK_SET(encoded_header, k++, nCodes, j, nSymbolSize - 6, COLOR_TAG);
	}
	for (int i = nSymbolSize - 5; i < nSymbolSize; ++i)
	{
		ELEM_CHECK_SET(encoded_header, k++, nCodes, 5, i, COLOR_TAG);
		ELEM_CHECK_SET(encoded_header, k++, nCodes, 6, i, COLOR_TAG);
	}
	// 右上角
	for (int j = nSymbolSize - 7; j < nSymbolSize; ++j)
	{
		ELEM_CHECK_SET(encoded_header, k++, nCodes, j, nSymbolSize - 7, COLOR_TAG);
		ELEM_CHECK_SET(encoded_header, k++, nCodes, j, nSymbolSize - 6, COLOR_TAG);
	}
	for (int i = nSymbolSize - 5; i < nSymbolSize; ++i)
	{
		ELEM_CHECK_SET(encoded_header, k++, nCodes, nSymbolSize - 7, i, COLOR_TAG);
		ELEM_CHECK_SET(encoded_header, k++, nCodes, nSymbolSize - 6, i, COLOR_TAG);
	}
	// 左上角
	for (int j = nSymbolSize - 7; j < nSymbolSize; ++j)
	{
		ELEM_CHECK_SET(encoded_header, k++, nCodes, j, 6, COLOR_TAG);
		ELEM_CHECK_SET(encoded_header, k++, nCodes, j, 5, COLOR_TAG);
	}
	for (int i = 4; i >= 0; --i)
	{
		ELEM_CHECK_SET(encoded_header, k++, nCodes, nSymbolSize - 7, i, COLOR_TAG);
		ELEM_CHECK_SET(encoded_header, k++, nCodes, nSymbolSize - 6, i, COLOR_TAG);
	}
	ASSERT(k == 72);

	// 调试时输出
#ifdef _DEBUG
	TRACE("* Encoded dataheader bits:\n");
	for (int i = 0; i < 72; ++i)
	{
		TRACE("%d, ", encoded_header[i]);
	}
	TRACE("\n");
#endif
}


/** 
* @brief 编码数据. 
* @details 该函数对字符串进行编码. 
* @param[out] nCodes[81][81] 二维码
* @param[in] nSymbolSize 符号大小
* @param[in] *code_bits 编码比特流
* @param[in] bitsLen 比特数
* @param[in] nMaskingNo 掩码模式
* @return 无
* @retval void
*/
void EncodeData(int nCodes[81][81], int nSymbolSize, int *code_bits, int bitsLen, int nMaskingNo)
{
	CDataMask *Mask = NULL;
	CDataMask0 Mask0;
	CDataMask1 Mask1;
	CDataMask2 Mask2;
	CDataMask3 Mask3;
	CDataMask4 Mask4;
	CDataMask5 Mask5;
	CDataMask6 Mask6;
	CDataMask7 Mask7;
	switch (nMaskingNo)
	{
	case 0 : Mask = &Mask0; break;
	case 1 : Mask = &Mask1; break;
	case 2 : Mask = &Mask2; break;
	case 3 : Mask = &Mask3; break;
	case 4 : Mask = &Mask4; break;
	case 5 : Mask = &Mask5; break;
	case 6 : Mask = &Mask6; break;
	case 7 : Mask = &Mask7; break;
	default: Mask = &Mask0; break;
	}
	int k = 0;
	srand(unsigned(time(0)));
	// 左边部分
	for (int j = 7; j < nSymbolSize - 7; ++j)
	{
		for (int i = 0; i < 7; ++i)
		{
			if (k < bitsLen)
			{
				ELEM_CHECK_SET(code_bits, k++, nCodes, j, i, COLOR_TAG);
				if (Mask->IsMasked(i, j))
					nCodes[j][i] = nCodes[j][i] ^ 1;
			}
			else
			{
				nCodes[j][i] = rand() & 1;
			}
		}
	}
	// 中间部分
	for (int j = 0; j < nSymbolSize; ++j)
	{
		for (int i = 7; i < nSymbolSize - 7; ++i)
		{
			if (k < bitsLen)
			{
				ELEM_CHECK_SET(code_bits, k++, nCodes, j, i, COLOR_TAG);
				if (Mask->IsMasked(i, j))
					nCodes[j][i] = nCodes[j][i] ^ 1;
			}
			else
			{
				nCodes[j][i] = rand() & 1;
			}
		}
	}
	// 右边部分
	for (int j = 7; j < nSymbolSize - 7; ++j)
	{
		for (int i = nSymbolSize - 7; i < nSymbolSize; ++i)
		{
			if (k < bitsLen)
			{
				ELEM_CHECK_SET(code_bits, k++, nCodes, j, i, COLOR_TAG);
				if (Mask->IsMasked(i, j))
					nCodes[j][i] = nCodes[j][i] ^ 1;
			}
			else
			{
				nCodes[j][i] = rand() & 1;
			}
		}
	}
}


/** 
* @brief 编码隐形二维码. 
* @details 
* @param[out] nCodes[81][81] 编码结果
* @param[in] *pUtf8 编码数据
* @param[in] nVersion 编码版本
* @param[in] nDataFormat 数据格式
* @param[in] nDataLength 数据长度
* @param[in] nEcLevel 纠错等级
* @param[in] nMaskingNo 掩码模式
* @return 函数返回符号大小，如果失败返回-1.
* @retval int
*/
int EncodeInvisibleCode(int nCodes[81][81], char *pUtf8, int nVersion, int nDataFormat, int nDataLength, int nEcLevel, int nMaskingNo)
{
	int nSymbolSize = nVersion + 18; // 符号大小
	int bitsLen = RS4BitsLen(nDataLength, nEcLevel); // 比特总数
	int *code_bits = RS4Encode((BYTE*)pUtf8, nDataLength, bitsLen, nEcLevel);
	int max_len = nSymbolSize * nSymbolSize - 196; // 可编码比特数
	// 比特总数不能超过向量长度
	int res = max_len - bitsLen;
	if (res < 0)
	{
		nSymbolSize = AutoVersion(bitsLen);
		max_len = nSymbolSize * nSymbolSize - 196;
		res = max_len - bitsLen;
	}
	// 统一设置为背景色
	for (int i = 0; i < nSymbolSize; ++i)
	{
		for (int j = 0; j < nSymbolSize; ++j)
		{
			nCodes[i][j] = BACKGROUND_TAG;
		}
	}
	// *** 编码位置元 *** //
	EncodePositionElem(nCodes, nSymbolSize);
	// *** 编码数据头(24 bits) *** //
	EncodeDataHeader(nCodes, nSymbolSize, nVersion, nDataFormat, nDataLength, nEcLevel, nMaskingNo);
	// *** 编码数据 *** //
	EncodeData(nCodes, nSymbolSize, code_bits, bitsLen, nMaskingNo);
	SAFE_DELETE(code_bits);
	return nSymbolSize;
}


/** 
* @brief 确定符号版本. 
* @details 
* @param[in] nBitsLength 需要编码的比特数
* @return 二维码版本
* @retval int
*/
int AutoVersion(int nBitsLength)
{
	// 公式：S*S - 196 >= N
	return int (sqrt(196 + nBitsLength) + 1);
}


/** 
* @brief 由二维码矩阵创建图像. 
* @details 
* @param[out] *pImage 目标图像
* @param[in] nCodes[81][81] 二维码矩阵
* @param[in] nSymbolSize 符号大小
* @param[in] cPositionElem 定位元
* @param[in] cColorElem 彩色元
* @param[in] cBackgroundElem 背景元
* @param[in] nPixelSiz 像素大小
* @return 函数返回图像的大小. 
* @retval int
*/
int Convert2Image(CyImage *pImage, int nCodes[81][81], int nSymbolSize, COLORREF cPositionElem, COLORREF cColorElem, COLORREF cBackgroundElem, int nPixelSize)
{
	int nImageSize = nPixelSize * nSymbolSize + QR_MARGIN * 2;
	pImage->Create(nImageSize, nImageSize, 24, 0UL);
	BYTE* pHead = pImage->GetHeadAddress();
	int nRowlen = pImage->GetRowlen();
	int nChannel = pImage->GetChannel();
	memset(pHead, 255, nImageSize * nRowlen * sizeof(BYTE));
	for (int j = 0; j < nSymbolSize; ++j)
	{
		for (int i = 0; i < nSymbolSize; ++i)
		{
			switch (nCodes[j][i])
			{
			case POSITION_TAG:
				SetPixel(pHead, nPixelSize, j * nPixelSize, i * nPixelSize, nRowlen, nChannel, cPositionElem);
				break;
			case COLOR_TAG:
				SetPixel(pHead, nPixelSize, j * nPixelSize, i * nPixelSize, nRowlen, nChannel, cColorElem);
				break;
			case BACKGROUND_TAG:
				SetPixel(pHead, nPixelSize, j * nPixelSize, i * nPixelSize, nRowlen, nChannel, cBackgroundElem);
				break;
			default:
				break;
			}
		}
	}
	
	return nImageSize;
}


/** 
* @brief 绘制位置探测图形. 
* @details 函数在图像的rect方形区域绘制一个内切圆. 
* @param[in] * pHead 图像数据
* @param[in] nWidth 图像宽度
* @param[in] nHeight 图像高度
* @param[in] nRowlen 每行字节数
* @param[in] nChannel 图像通道
* @param[in] rect：	方形区域
* @param[in] R 背景色R分量
* @param[in] G 背景色G分量
* @param[in] B 背景色B分量
* @param[in] nPixelSize 像素大小
* @return 无
* @retval void
*/
void PaintPositionPartten(BYTE* pHead, int nWidth, int nHeight, int nRowlen, int nChannel, CMyRect rect, int R, int G, int B, int nPixelSize)
{
	int rtWidth = rect.Width();
	if (rtWidth != rect.Height())
		return;
	float r = rtWidth / 2.0f;
	float Cx = (rect.left + rect.right) / 2.0f;
	float Cy = (rect.top + rect.bottom) / 2.0f;
	for (int j = rect.top; j < rect.bottom; ++j)
	{
		for (int i = rect.left; i < rect.right; ++i)
		{
			float x = i + 0.5f - Cx, y = j + 0.5f - Cy;
			float dis = sqrt(x * x + y * y);
			if (dis > r || (1/3.0f * r < dis && dis < 2/3.0f * r))
			{
				pHead[    i * nChannel + j * nRowlen] = B;
				pHead[1 + i * nChannel + j * nRowlen] = G;
				pHead[2 + i * nChannel + j * nRowlen] = R;
			}
		}
	}
}


/** 
* @brief 是否可编码. 
* @details 判断当前版本是否可以编码当前长度的数据. 
* @param[in] nSymbolSize 符号大小
* @param[in] nBitsLen 编码比特数
* @return TRUE or FALSE
* @retval BOOL
*/
BOOL CodeAble(int nSymbolSize, int nBitsLen)
{
	// 可编码比特数
	int max_len = nSymbolSize * nSymbolSize - 196;
	// 比特总数不能超过向量长度
	return max_len >= nBitsLen;
}