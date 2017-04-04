#include "stdafx.h"
#include "DecodeFuncs.h"
#include "..\3DCode\ColorsMask.h"
#include "..\3DCode\reedSolomon.h"
#include "..\3DCode\templatefuncs.h"
#include "..\3DCode\Transform.h"
#include "..\3DCode\K_Means.h"
#include "..\3DCode\PixelsOperation.h"
#include "..\3DCode\RS_ecc.h"

/** 缩放图像到该尺寸进行解码 */
#define IMAGE_ZOOM_TO 320

/** 
* @brief 如果p到A的距离更近，返回TRUE；否则FALSE. 
* @param[in] p 待确认点
* @param[in] A A点
* @param[in] B B点
* @return TRUE/FALSE
* @retval BOOL
*/
BOOL CLUSTER_FUN(float3 &p, float3 &A, float3 &B) 
{
	BOOL result = (_Distance(p, A) < _Distance(p, B)) ? COLOR_TAG : BACKGROUND_TAG;
	return result;
}


/** 
* @brief 寻找模最大元素的编号. 
* @param[in] a 第一个元素
* @param[in] b 第二个元素
* @param[in] c 第三个元素
* @return 哪个元素的模最大就返回其编号. 
* @retval int
*/
int FindMaxmumIndex(float2 a, float2 b, float2 c)
{
	if (fabs(a) > fabs(b))
	{
		return fabs(a) > fabs(c) ? 1 : 3;
	}
	else
	{
		return fabs(b) > fabs(c) ? 2 : 3;
	}
}


/** 
* @brief 获取二维码尺寸. 
* @details 记左下角到左上、右下角中心的距离为d1;
* 记右上角到左上、右下角中心的距离为d2; 联立方程:
* [(x+4)^2 + 1] * s^2 = d1^2
* (x+5) * s = d2
* 得到：(1-r)x^2 + (8-10r)x + (17-25r) = 0, r = d1^2 / d2^2
* 记为：ax^2 + bx + c = 0
* 其中，s为模块大小
* @param[in] centers 聚类中心点
* @return 返回模块大小. 
*/
int GetBarCodeSize(float2 centers[4])
{
	float d1 = (Distance(centers[0], centers[1]) + Distance(centers[0], centers[3])) / 2.0f;
	float d2 = (Distance(centers[2], centers[1]) + Distance(centers[2], centers[3])) / 2.0f;
	float r = d1 * d1 / d2 / d2;
	float a = 1 - r, b = 8 - 10 * r, c = 17 - 25 * r;
	float delta = b * b - 4 * a * c;
	if (delta < 0)
		return 0;
	float x1 = (-b + sqrt(delta)) / (2 * a);
	float x2 = (-b - sqrt(delta)) / (2 * a);
	float temp = max(x1, x2);
	int nModuleSize = (int)round(10.0f + temp);
	if (nModuleSize < 18 || nModuleSize > 81)
	{
		TRACE(" * Warning : Barcode size exception\n");
	}

	return nModuleSize;
}


/** 
* @brief 解码数据头. 
* @details 该函数解码数据头. 
* @param[in] *pHead 图像头指针
* @param[in] nPixelSize 像素大小
* @param[in] nRowlen 图像每行元素个数
* @param[in] nChannel 图像元素通道数
* @param[in] cColorElem 彩色值
* @param[in] cBackgroundElem 背景值
* @param[in] nSymbolSize 二维码尺寸
* @param[out] &nVersion 版本大小
* @param[out] &nDataFormat 数据格式
* @param[out] &nDataLength 数据长度
* @param[out] &nEcLevel 纠错等级
* @param[out] &nMaskingNo 掩码模式
* @return 真或假
* @retval BOOL
*/
BOOL DecodeDataHeader(BYTE *pHead, int nPixelSize, int nRowlen, int nChannel, 
					  COLORREF cColorElem, COLORREF cBackgroundElem, int nSymbolSize, 
					  int &nVersion, int &nDataFormat, int &nDataLength, int &nEcLevel, int &nMaskingNo)
{
	// 彩色和背景色转为float3
	float3 fColorElem = RgbColorRef2Float3(cColorElem);
	float3 fBackgroundElem = RgbColorRef2Float3(cBackgroundElem);
	int decoded_header[72] = {0};

	// 右下角
	int k = 0;
	for (int j = 0; j < 7; ++j)
	{
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, (nSymbolSize - 7) * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, (nSymbolSize - 6) * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
	}
	for (int i = nSymbolSize - 5; i < nSymbolSize; ++i)
	{
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, 5 * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, 6 * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
	}
	// 右上角
	for (int j = nSymbolSize - 7; j < nSymbolSize; ++j)
	{
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, (nSymbolSize - 7) * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, (nSymbolSize - 6) * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
	}
	for (int i = nSymbolSize - 5; i < nSymbolSize; ++i)
	{
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, (nSymbolSize - 7) * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, (nSymbolSize - 6) * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
	}
	// 左上角
	for (int j = nSymbolSize - 7; j < nSymbolSize; ++j)
	{
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, 6 * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, 5 * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
	}
	for (int i = 4; i >= 0; --i)
	{
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, (nSymbolSize - 7) * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
		decoded_header[k++] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, (nSymbolSize - 6) * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
	}
	ASSERT(72 == k);

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
	XOR(decoded_header, HEADER_MASKINGS, 72);

	// 调试时输出
#ifdef _DEBUG
	TRACE("* Decoded dataheader bits:\n");
	for (int i = 0; i < 72; ++i)
	{
		TRACE("%d, ", decoded_header[i]);
	}
	TRACE("\n");
#endif

	// 72b按2bit合并成36个码字(3个一组进行解码)
	MergedIntBy2Bits(decoded_header, 72);
	// 将数据头进行解码
	int nHeader[24] = {0};
	int temp[3];
	reedSolomon rs(2, 1);
	for (int j = 0; j < 12; ++j)
	{
		memcpy(temp, decoded_header + 3 * j, 3 * sizeof(int));
		rs.rs_decode(temp);
		nHeader[2 * j    ] = 0x00000001 &  temp[2];
		nHeader[2 * j + 1] = 0x00000001 & (temp[2]>>1);
	}

#ifdef _DEBUG
	TRACE("* Decoded dataheader:\n");
	for (int i = 0; i < 24; ++i)
	{
		TRACE("%d, ", nHeader[i]);
	}
	TRACE("\n");
#endif

	// 1-6位：编码版本，从0到63
	nVersion = nHeader[0] + 2 * nHeader[1] + 4 * nHeader[2] + 8 * nHeader[3] + 16 * nHeader[4] + 32 * nHeader[5];
	// 7-8位：数据格式，从0到3
	nDataFormat = nHeader[6] + 2 * nHeader[7];
	// 9-18位：数据长度，从0到1023
	nDataLength = nHeader[8] + 2 * nHeader[9] + 4 * nHeader[10] + 8 * nHeader[11] + 16 * nHeader[12] + 32 * nHeader[13] 
					+ 64 * nHeader[14] + 128 * nHeader[15] + 256 * nHeader[16] + 512 * nHeader[17];
	// 19-21位：纠错等级，从1到7
	nEcLevel = nHeader[18] + 2 * nHeader[19] + 4 * nHeader[20];
	// 22-24位：掩码模式，从0到7
	nMaskingNo = nHeader[21] + 2 * nHeader[22] + 4 * nHeader[23];
	// 判断是否成功
	if (0 == nDataLength * nEcLevel)
		return FALSE;
	int nMaxDataLength = nSymbolSize * nSymbolSize - 4 * 49;
	if(nDataLength > nMaxDataLength)
		return FALSE;

	return TRUE;
}


/** 
* @brief 解码数据. 
* @details 该函数对图像进行解码. 
* @param[in] *pHead 图像头指针
* @param[in] nPixelSize 像素大小
* @param[in] nRowlen 图像每行元素个数
* @param[in] nChannel 图像元素通道数
* @param[in] cColorElem 彩色值
* @param[in] cBackgroundElem 背景值
* @param[in] nSymbolSize 二维码尺寸
* @param[out] &nVersion 版本大小
* @param[out] &nDataFormat 数据格式
* @param[out] &nDataLength 数据长度
* @param[out] &nEcLevel 纠错等级
* @param[out] &nMaskingNo 掩码模式
* @return 真或假
* @retval BOOL
*/
char* DecodeData(BYTE *pHead, int nPixelSize, int nRowlen, int nChannel, 
				 COLORREF cColorElem, COLORREF cBackgroundElem, int nSymbolSize, 
				 int nVersion, int nDataFormat, int nDataLength, int nEcLevel, int nMaskingNo)
{
	// 彩色和背景色转为float3
	float3 fColorElem = RgbColorRef2Float3(cColorElem);
	float3 fBackgroundElem = RgbColorRef2Float3(cBackgroundElem);
	// 4bits为一个符号，信息符号数
	int nMsgSymbols = 2 * nDataLength;
	// 信息占比,一次能编码的符号数
	int nDataRate = 15 - 2 * nEcLevel;
	// 15个符号为一块，块数
	int nBlocks = (int)ceil(1.f * nMsgSymbols / nDataRate);
	// 编码需要的符号总数
	int nRealMsgSymbols = nBlocks * 15;
	// 存放编码结果的位总数(符号数 X 每符号的位数)
	int bitsLen = 4 * nRealMsgSymbols;
	int *code_bits = new int[bitsLen];
	memset(code_bits, 0, bitsLen * sizeof(int));

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
	// 左边部分
	for (int j = 7; j < nSymbolSize - 7; ++j)
	{
		for (int i = 0; i < 7; ++i)
		{
			if (k < bitsLen)
			{
				code_bits[k] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
				if (Mask->IsMasked(i, j))
					code_bits[k] ^= 0x00000001;
				k++;
			}
			else
			{
				goto next;
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
				code_bits[k] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
				if (Mask->IsMasked(i, j))
					code_bits[k] ^= 0x00000001;
				k++;
			}
			else
			{
				goto next;
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
				code_bits[k] = CLUSTER_FUN(GetPixel(pHead, nPixelSize, j * nPixelSize, i * nPixelSize, nRowlen, nChannel), fColorElem, fBackgroundElem);
				if (Mask->IsMasked(i, j))
					code_bits[k] ^= 0x00000001;
				k++;
			}
			else
			{
				goto next;
			}
		}
	}
	// 数据超出长度了，可能解码已经失败
	bitsLen = nSymbolSize * nSymbolSize - 196;
	nRealMsgSymbols = bitsLen / 4;
	nBlocks = (int)ceil(nRealMsgSymbols / 15.0);
	nMsgSymbols = nBlocks * nDataRate;
	nDataLength = nMsgSymbols / 2;

next:
	char *ch = new char[nDataLength];
	int *result = RS4Decode(code_bits, nDataLength, bitsLen, nEcLevel);
	for (int i = 0; i < nDataLength; ++i)
	{
		ch[i] = Bits2Byte(result + 8 * i);
	}
	SAFE_DELETE(result);
	SAFE_DELETE(code_bits);

	return ch;
}


/**
* @brief 解析二维码图像. 
* @param[in] * pHead 图像数据
* @param[in] nWidth 图像宽度
* @param[in] nHeight 图像高度
* @param[in] nRowlen 图像每行字节数
* @param[in] nChannel 图像通道个数
* @param[in] roi 感兴趣区域
* @param[in] cPositionElem 位置元
* @param[in] cColorElem 彩色元
* @param[in] cBackgroundElem 背景元
* @param[in] Centers 位置探测元中心
* @param[in] &nVersion 版本
* @param[in] &nDataFormat 数据格式
* @param[in] &nDataLength 数据长度
* @param[in] &nEcLevel 纠错等级
* @param[in] &nMaskingNo 掩码版本
*/
char* DecodeImage(BYTE* pHead, int nWidth, int nHeight, int nRowlen, int nChannel, RoiRect roi, 
				 COLORREF cPositionElem, COLORREF cColorElem, COLORREF cBackgroundElem, 
				 float2 Centers[4], int &nVersion, int &nDataFormat, int &nDataLength, int &nEcLevel, int &nMaskingNo)
{
	// 提取感兴趣区域
	BYTE *temp = ImageROI(pHead, nWidth, nHeight, nRowlen, roi);

	// 当图像有一边大于指定值时缩放
	LimitImage(&temp, nWidth, nHeight, nRowlen, nChannel, IMAGE_ZOOM_TO);

#ifdef _DEBUG
	ImageWrite(".\\ImageROI.txt", temp, nWidth, nHeight, nRowlen);
#endif

	// RGB转Gray
	BYTE* gray = Rgb2Gray(temp, nWidth, nHeight, nRowlen);
	int nRowlenNew = WIDTHBYTES(nWidth * 8);

#ifdef _DEBUG
	ImageWrite(".\\Rgb2Gray.txt", gray, nWidth, nHeight, nRowlenNew);
#endif

	// 中值滤波
	medianFilter(gray, nWidth, nHeight, nRowlenNew, 3);

#ifdef _DEBUG
	ImageWrite(".\\medianFilter.txt", gray, nWidth, nHeight, nRowlenNew);
#endif

	// K-means 聚类获取背景
	// a11		a12
	// a21		a22
	K_means(gray, nRowlenNew, RoiRect(0, 0, nWidth/2, nHeight/2), 3, RgbColorRef2Gray(cPositionElem), 10);// a11
	K_means(gray, nRowlenNew, RoiRect(nWidth/2, nHeight/2, nWidth, nHeight), 3, RgbColorRef2Gray(cPositionElem), 10);// a22
	K_means(gray, nRowlenNew, RoiRect(nWidth/2, 0, nWidth, nHeight/2), 3, RgbColorRef2Gray(cPositionElem), 10);// a12
	K_means(gray, nRowlenNew, RoiRect(0, nHeight/2, nWidth/2, nHeight), 3, RgbColorRef2Gray(cPositionElem), 10);// a21

#ifdef _DEBUG
	ImageWrite(".\\K_means.txt", gray, nWidth, nHeight, nRowlenNew);
#endif

	// 中值滤波
	medianFilter(gray, nWidth, nHeight, nRowlenNew, 7);

#ifdef _DEBUG
	ImageWrite(".\\medianFilter2.txt", gray, nWidth, nHeight, nRowlenNew);
#endif

	// 准备K均值聚类
	vector<float2> position;
	for (int j = 0 ; j < nHeight; ++j)
	{
		for (int i = 0; i < nWidth; ++i)
		{
			if (0 == gray[i + j * nRowlenNew])
			{
				float x = i + 0.5f;
				float y = j + 0.5f;
				position.push_back(float2(x, y));
			}
		}
	}
	vector<int> retval = K_means(position, Centers, nWidth, nHeight, 10);
	int nCount = retval.size();
	// 对原始图像进行分割
	for (int j = 0; j < nCount; ++j)
	{
		int x = position[j].x;
		int y = position[j].y;
		gray[x + y * nRowlenNew] = (retval[j] + 1) * 64;
	}

#ifdef _DEBUG
	ImageWrite(".\\K_means2.txt", gray, nWidth, nHeight, nRowlenNew);
#endif

	// 透视校正的QR码尺寸
	int nSymbolSize = GetBarCodeSize(Centers);

	// 透视变换后图像宽度
	int nDstSize = 0;
	// 每个模块的大小
	int nMoudleSize = 16;
	PerspectTransform transform = CreateTransform(Centers, nSymbolSize, (float)nMoudleSize);
	// 参数必须为BYTE*，否则会出现模糊
	BYTE *afterPPT = perspectiveTransform(temp, nWidth, nHeight, nRowlen, nChannel, 
		transform, Centers, nSymbolSize, nMoudleSize, nDstSize);
	int nRowlenTemp = WIDTHBYTES(nDstSize * 8 * nChannel);
#ifdef _DEBUG
	ImageWrite(".\\PerspectTransform.txt", afterPPT, nDstSize, nDstSize, nRowlenTemp);
#endif

	char *result = NULL;
	if (nSymbolSize)
	{
		// 翻转是不可少的，否则解码失败
		ImageTranspose(&afterPPT, nDstSize, nDstSize, nRowlenTemp);
#ifdef _DEBUG
		ImageWrite(".\\ImageTranspose.txt", afterPPT, nDstSize, nDstSize, nRowlenTemp);
#endif
		// 解码数据头
		if (DecodeDataHeader(afterPPT, nMoudleSize, nRowlenTemp, nChannel, 
			cColorElem, cBackgroundElem, nSymbolSize, 
			nVersion, nDataFormat, nDataLength, nEcLevel, nMaskingNo))
		{
			result = DecodeData(afterPPT, nMoudleSize, nRowlenTemp, nChannel, 
				cColorElem, cBackgroundElem, nSymbolSize, 
				nVersion, nDataFormat, nDataLength, nEcLevel, nMaskingNo);
		}

		SAFE_DELETE(afterPPT);
	}

	SAFE_DELETE(gray);

	SAFE_DELETE(temp);

	return result;
}


/** 
* @brief 中值滤波. 
* @details 中值滤波比较耗时.
* @param[in] * pData 图像数据
* @param[in] nWidth 图像宽度
* @param[in] nHeight 图像高度
* @param[in] nRowlen 图像每行字节数
* @param[in] nSize 滤波核尺寸
* @warning 仅限彩色图像
* 在解码中最为耗时.
*/
BOOL medianFilter(BYTE* pData, int nWidth, int nHeight, int nRowlen, int nSize)
{
	// 滤波核尺寸必须为奇数
	ASSERT(nSize & 0x00000001);
	BYTE *pCopy = new BYTE[nHeight * nRowlen];
	memcpy(pCopy, pData, nHeight * nRowlen * sizeof(BYTE));
	int nChannel = nRowlen / nWidth;
	ASSERT(nChannel == 1);
	int K = nSize >> 1;// 滤波核半径
	BYTE *temp = new BYTE[nSize * nSize];// 存放领域像素
	// 逐个像素进行滤波(边界不处理，这样便不需要作越界判断)
	for (int r = K; r < nHeight - K; ++r)
	{
		for (int c = K; c < nWidth - K; ++c)
		{
			int rgbCount = 0;// 当前像素(r, c)的领域像素个数
			for (int r0 = r - K; r0 <= r + K; ++r0)
			{
				for (int c0 = c - K; c0 <= c + K; ++c0)
				{
					// (r0, c0)像素
					BYTE* Pixel = pData + r0*nRowlen + c0*nChannel;
					temp[rgbCount] = *Pixel++;
#if QUICK_SORT
					if (rgbCount > 0)
					{
						// 从小到大插入排序
						for (int j = rgbCount; j > 0 && temp[rgbCount] < temp[rgbCount - 1]; --j)
						{
							BYTE t;
							t = temp[j];
							temp[j] = temp[j - 1];
							temp[j - 1] = t;
						}
					}
#endif
					++rgbCount;
				}
			}
			ASSERT(rgbCount == nSize * nSize);
#if !(QUICK_SORT)
			for (int i = 0; i < (rgbCount>>1); ++i)
			{
				for (int j = i + 1; j < rgbCount; ++j)
				{
					if (temp[j] < temp[i])
					{
						BYTE t;
						t = temp[j];
						temp[j] = temp[j - 1];
						temp[j - 1] = t;
					}
				}
			}
#endif
			rgbCount >>= 1;// 除2,取中值
			BYTE* Pixel = pCopy + r*nRowlen + c*nChannel;
			*Pixel++ = temp[rgbCount];
		}
	}
	SAFE_DELETE(temp);
	memcpy(pData, pCopy, nHeight * nRowlen * sizeof(BYTE));
	SAFE_DELETE(pCopy);
	return TRUE;
}


#if _MSC_VER <= 1800
/** 
* @brief 四舍五入. 
*/
inline float round(float r)
{
	return (r > 0.0f) ? floor(r + 0.5f) : ceil(r - 0.5f);
}
#endif


/** 
* @brief 初始化聚类中心(2016/9/19). 
* @param[in] *Center 聚类中心
* @param[in] K 聚类个数
* @param[in] position
* @param[in] bTryHard 强力初始化
* @note 当@ref bTryHard为TRUE时，函数将随机生成一组聚类中心，取其中组间差异最大的. 
*/
void InitializeClusterCenters(float2 *Center, vector<float2> & position, BOOL bTryHard)
{
	int nCount = position.size();
	// 生成4个中心
	unsigned int seed = (unsigned int)time(NULL);
	for (int i = 0; i < 4; i++)
	{
		srand(seed);
		int id = rand() % nCount;
		Center[i] = position[id];
		TRACE("C[%d] = (%.6f, %.6f)\t", i, Center[i].x, Center[i].y);
		seed = rand();
	}
	TRACE("\n");
	// 判断是否继续
	if (!bTryHard)
		return;
	// 计算组间差异度量值
	float diff = Difference(Center, 4);
	float2 *newCenter = new float2[4];
	// 循环24次，生成聚类中心，检索是否有更好的
	for (int j = 0; j < 24; ++j)
	{
		for (int i = 0; i < 4; i++)
		{
			srand(seed);
			int id = rand() % nCount;
			newCenter[i] = position[id];
			seed = rand();
		}
		float new_diff = Difference(newCenter, 4);
		// 如果找到组间差异更大的聚类中心
		if (new_diff > diff)
		{
			diff = new_diff;
			memcpy(Center, newCenter, 4 * sizeof(float2));
			for (int i = 0; i < 4; i++)
			{
				TRACE("C[%d] = (%.6f, %.6f)\t", i, Center[i].x, Center[i].y);
			}
			TRACE("\n");
		}
	}
	delete [] newCenter;
}


/// 对坐标position进行聚类
vector<int> K_means(vector<float2> & position, float2 centers[4], int nWidth, int nHeight, int nMaxepoches)
{
	// 聚类中心与聚类个数
	float2 *oldCenter = new float2[4];
	float *sum = new float[4];
	int nCount = position.size();
	int *Cluster = new int[nCount];
	// 生成4个中心
	TRACE("正在初始化聚类中心...\n");
	InitializeClusterCenters(centers, position, TRUE);
	/* 多次迭代直至收敛，本次试验迭代nMaxepoches次 */
	for (int it = 0; it < nMaxepoches; ++it)
	{
		/* 求出每个样本点距应该属于哪一个聚类 */
		for (int j = 0; j < nCount; ++j)
		{
			/* 都初始化属于第0个聚类 */    
			int c = 0;
			float min_distance = _Distance(position[j], centers[c]);
			for (int s = 1; s < 4; s++)
			{
				float new_distance = _Distance(position[j], centers[s]);
				if (new_distance < min_distance)
				{
					min_distance = new_distance;
					c = s;
				}
			}
			// 将坐标标记为c类
			Cluster[j] = c;
		}
		/* 更新聚类中心 */
		memcpy(oldCenter, centers, 4 * sizeof(float2));
		ZeroMemory(sum, 4 * sizeof(float));
		ZeroMemory(centers, 4 * sizeof(float2));
		for (int j = 0; j < nCount; ++j)
		{
			centers[Cluster[j]] += position[j];
			sum[Cluster[j]]++;
		}
		float diff = 0;
		for (int i = 0; i < 4; ++i)
		{
			if (sum[i])
			{
				centers[i] /= sum[i];
			}
			diff += fabs(oldCenter[i] - centers[i]);
		}
		// 注销掉下述语句以监视迭代过程
		TRACE(" * 迭代次数 = %d：\n", it + 1);
		for (int i = 0; i < 4; ++i)
		{
			TRACE("C[%d] = (%.6f, %.6f)\t%.1f个\t", i, centers[i].x, centers[i].y, sum[i]);
		}
		TRACE("\n");
		// 当前后相邻两次中心变化较小时（0.01个像素）退出
		if (diff < 0.0004f)
			break;
	}
	// 使个数最多的类聚类中心排在第一
	for (int i = 1; i < 4; ++i)
	{
		if (sum[i] > sum[0])
		{
			float temp(sum[0]);
			sum[0] = sum[i];
			sum[i] = temp;
			float2 temp2(centers[0]);
			centers[0] = centers[i];
			centers[i] = temp2;
		}
	}
	SortClusterCenters(centers);
	vector<int> retval(Cluster, Cluster + nCount);
	delete [] oldCenter;
	delete [] sum;
	delete [] Cluster;
	return retval;
}


/** 
* @brief 对聚类进行排序. 
* @details 排序方向：c0-左下、c1-右下、c2右上、c3左上.
*/
void SortClusterCenters(float2 centers[4])
{
	// 寻找右上角的点
	float2 vec[3];
	vec[0] = centers[1] - centers[0];
	vec[1] = centers[2] - centers[0];
	vec[2] = centers[3] - centers[0];
	int maxmumIndex = FindMaxmumIndex(vec[0], vec[1], vec[2]);
	// 交换位置
	if (maxmumIndex != 2)
	{
		float2 temp(centers[2]);
		centers[2] = centers[maxmumIndex];
		centers[maxmumIndex] = temp;
	}
	// 交换左上角、右下角两个点
	if (acos(vec[2].y / fabs(vec[2])) - acos(vec[0].y / fabs(vec[0])) < 0)
	{
		float2 temp(centers[1]);
		centers[1] = centers[3];
		centers[3] = temp;
	}
}


/** 
* @brief 参考自ZXing - Detector
* @param[in] pos 位置探测元
* @param[in] dimension QR码尺寸
* @param[in] fmodulesize 模块大
*/
PerspectTransform CreateTransform(float2 pos[4], int dimension, float fmodulesize)
{
	float dimMinusTwo = (dimension - 2.5f) * fmodulesize;

	return PerspectTransform::quadrilateralToQuadrilateral(
		3.5f * fmodulesize, 3.5f * fmodulesize, 
		dimMinusTwo, 2.5f * fmodulesize, 
		dimMinusTwo, dimMinusTwo, 
		2.5f * fmodulesize, dimMinusTwo, 
		pos[0].x, pos[0].y, 
		pos[1].x, pos[1].y, 
		pos[2].x, pos[2].y, 
		pos[3].x, pos[3].y);
}