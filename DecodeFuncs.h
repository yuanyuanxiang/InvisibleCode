/************************************************************************/
// @file	DecodeFuncs.h
// @brief	隐形二维码的解码函数
// @author	袁沅祥
// @date	2016/9/17
// 
/************************************************************************/
#include <vector>
using namespace std;

#include "..\3DCode\DataTypes.h"
#include "..\3DCode\PerspectTransform.h"

/// 位置元标签
#define POSITION_TAG 2

/// 彩色元标签
#define COLOR_TAG 1

/// 背景元标签
#define BACKGROUND_TAG 0

/// 如果p到A的距离更近，将其标记为COLOR_TAG(1)，否则标记为BACKGROUND_TAG(0)
#define CLUSTER(p, A, B) ((_Distance(p, A) < _Distance(p, B)) ? COLOR_TAG : BACKGROUND_TAG)

// 如果p到A的距离更近，将其标记为COLOR_TAG(1)，否则标记为BACKGROUND_TAG(0)
BOOL CLUSTER_FUN(float3 &p, float3 &A, float3 &B) ;

// 寻找模最大元素的编号
int FindMaxmumIndex(float2 a, float2 b, float2 c);

// 获取模块大小
int GetBarCodeSize(float2 centers[4]);

//////////////////////////////////////////////////////////////////////////

// 解码数据头
BOOL DecodeDataHeader(BYTE *pHead, int nPixelSize, int nRowlen, int nChannel, 
					  COLORREF cColorElem, COLORREF cBackgroundElem, int nSymbolSize, 
					  int &nVersion, int &nDataFormat, int &nDataLength, int &nEcLevel, int &nMaskingNo);
// 解码数据
char* DecodeData(BYTE *pHead, int nPixelSize, int nRowlen, int nChannel, 
				 COLORREF cColorElem, COLORREF cBackgroundElem, int nSymbolSize, 
				 int nVersion, int nDataFormat, int nDataLength, int nEcLevel, int nMaskingNo);

// 解析二维码图像
char* DecodeImage(BYTE* pHead, int nWidth, int nHeight, int nRowlen, int nChannel, RoiRect roi, 
				  COLORREF cPositionElem, COLORREF cColorElem, COLORREF cBackgroundElem, 
				  float2 Centers[4], int &nVersion, int &nDataFormat, int &nDataLength, int &nEcLevel, int &nMaskingNo);

//////////////////////////////////////////////////////////////////////////

#if _MSC_VER <= 1800
// 四舍五入
float round(float r);
#endif

// 初始化聚类中心
void InitializeClusterCenters(float2 *Center, vector<float2> & position, BOOL bTryHard = TRUE);

// 对坐标position进行聚类
vector<int> K_means(vector<float2> & position, float2 centers[4], int nWidth, int nHeight, int nMaxepoches = 24);

BOOL medianFilter(BYTE* pData, int nWidth, int nHeight, int nRowlen, int nSize);

// 对聚类中心进行排序
void SortClusterCenters(float2 centers[4]);

// 创建一个透视变换
PerspectTransform CreateTransform(float2 pos[4], int dimension, float fmodulesize);