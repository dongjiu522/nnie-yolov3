#ifndef __SVP_NNIE_FUN_H_
#define __SVP_NNIE_FUN_H_


#include "hi_type.h"
enum RPN_SUPRESS_FLAG
{
	RPN_SUPPRESS_FALSE = 0,
	RPN_SUPPRESS_TRUE = 1,
	RPN_SUPPRESS_BUTT
};

typedef struct hiSVP_SAMPLE_STACK_S {
    HI_S32     s32Min;      /*The minimum position coordinate */
    HI_S32     s32Max;      /*The maximum position coordinate */
} SVP_SAMPLE_STACK_S;

typedef struct hiSVP_NNIE_COORD_S
{
    HI_S32 Xmin;
    HI_S32 Ymin;
    HI_S32 Xmax;
    HI_S32 Ymax;
}SVP_COORD_S;

typedef struct hiSVP_NNIE_BBOX_INFO
{
    SVP_COORD_S stCoord;
    HI_U32 u32score;
    HI_U32 u32class;
}SVP_BBOX_INFO;


typedef struct hiSVP_SAMPLE_BOX_S
{
    HI_FLOAT f32Xmin;
    HI_FLOAT f32Xmax;
    HI_FLOAT f32Ymin;
    HI_FLOAT f32Ymax;
    HI_FLOAT f32ClsScore;
    HI_U32 u32MaxScoreIndex;
    HI_U32 u32Mask;
}SVP_SAMPLE_BOX_S;

typedef struct hiSVP_SAMPLE_BOX_RESULT_INFO_S
{
    HI_U32 u32OriImHeight;
    HI_U32 u32OriImWidth;
    SVP_SAMPLE_BOX_S* pstBbox;
}SVP_SAMPLE_BOX_RESULT_INFO_S;


/*********************************************************
Function: QuickExp
Description: Do QuickExp with 20.12 input
*********************************************************/
HI_FLOAT QuickExp(HI_S32 u32X);

/*********************************************************
Function: FloatEqual
Description: float type equal
*********************************************************/
HI_U32 FloatEqual(HI_FLOAT a, HI_FLOAT b);

/*********************************************************
Function: SoftMax
Description: Do softmax on a HI_FLOAT vector af32Src with length s32ArraySize.
             Result will recode in af32Src(input will be modified).
*********************************************************/
HI_S32 SoftMax(HI_FLOAT *af32Src, HI_S32 s32ArraySize);
/*deal with num*/
HI_S32 SoftMax_N(HI_FLOAT *af32Src, HI_S32 s32ArraySize, HI_U32 u32Num);

/*********************************************************
Function: Sigmoid
Description: return Sigmoid calc value
*********************************************************/
HI_FLOAT Sigmoid(HI_FLOAT f32Val);

/*********************************************************
Function: Overlap
Description: Calculate the IOU of two bboxes
*********************************************************/
HI_S32 Overlap(
    HI_S32 s32XMin1, HI_S32 s32YMin1,
    HI_S32 s32XMax1, HI_S32 s32YMax1,
    HI_S32 s32XMin2, HI_S32 s32YMin2,
    HI_S32 s32XMax2, HI_S32 s32YMax2,
    HI_S64* ps64AreaSum, HI_S64* ps64AreaInter);

/**************************************************
Function: Argswap
Description: used in NonRecursiveQuickSort
***************************************************/
HI_S32 Argswap(HI_S32* ps32Src1, HI_S32* ps32Src2);

/**************************************************
Function: BoxArgswap
Description: swap box
***************************************************/
HI_S32 BoxArgswap(SVP_SAMPLE_BOX_S* pstBox1, SVP_SAMPLE_BOX_S* pstBox2);

/**************************************************
Function: NonRecursiveArgQuickSort
Description: sort with NonRecursiveArgQuickSort
***************************************************/
HI_S32 NonRecursiveArgQuickSort(HI_S32* aResultArray, HI_S32 s32Low, HI_S32 s32High, SVP_SAMPLE_STACK_S *pstStack);

/**************************************************
Function: NonRecursiveArgQuickSortWithBox
Description: NonRecursiveArgQuickSort with box input
***************************************************/
HI_S32 NonRecursiveArgQuickSortWithBox(SVP_SAMPLE_BOX_S* pstBoxs, HI_S32 s32Low, HI_S32 s32High, SVP_SAMPLE_STACK_S *pstStack);

/**************************************************
Function: NonMaxSuppression
Description: proposal NMS with u32NmsThresh
***************************************************/
HI_S32 NonMaxSuppression(HI_S32* pu32Proposals, HI_U32 u32NumAnchors, HI_U32 u32NmsThresh);

/**************************************************
Function: GetMaxVal
Description: return max value in array(float type)
***************************************************/
HI_FLOAT GetMaxVal(HI_FLOAT *pf32Val, HI_U32 u32Num, HI_U32 *pu32MaxValueIndex);

/**************************************************
Function: FilterLowScoreBbox
Description: remove low conf score proposal bbox
***************************************************/
HI_S32 FilterLowScoreBbox(HI_S32* pu32Proposals, HI_U32 u32NumAnchors, HI_U32 u32NmsThresh,
    HI_U32 u32FilterThresh, HI_U32* u32NumAfterFilter);

/**************************************************
Function: generate Base Anchors
Description: generate Base Anchors by give miniSize, ratios, and scales
***************************************************/
HI_S32 GenBaseAnchor(
    HI_FLOAT* pf32RatioAnchors, const HI_U32* pu32Ratios, HI_U32 u32NumRatioAnchors,
    HI_FLOAT* pf32ScaleAnchors, const HI_U32* pu32Scales, HI_U32 u32NumScaleAnchors,
    const HI_U32* au32BaseAnchor);

/**************************************************
Function: SetAnchorInPixel
Description: set base anchor to origin pic point based on pf32ScaleAnchors
***************************************************/
HI_S32 SetAnchorInPixel(
    HI_S32* ps32Anchors,
    const HI_FLOAT* pf32ScaleAnchors,
    HI_U32 u32ConvHeight,
    HI_U32 u32ConvWidth,
    HI_U32 u32NumAnchorPerPixel,
    HI_U32 u32SpatialScale);

/**************************************************
Function: BBox Transform
Description: parameters from Conv3 to adjust the coordinates of anchor
***************************************************/
HI_S32 BboxTransform(
    HI_S32* ps32Proposals,
    HI_S32* ps32Anchors,
    HI_S32* ps32BboxDelta,
    HI_FLOAT* pf32Scores);

/*deal with num*/
HI_S32 BboxTransform_N(
    HI_S32* ps32Proposals,
    HI_S32* ps32Anchors,
    HI_S32* ps32BboxDelta,
    HI_FLOAT* pf32Scores,
    HI_U32 u32NumAnchors);

HI_S32 BboxTransform_FLOAT(
    HI_FLOAT* pf32Proposals,
    HI_FLOAT* pf32Anchors,
    HI_FLOAT* pf32BboxDelta,
    HI_FLOAT  f32Scores);

/**************************************************
Function: BboxClip
Description: clip proposal bbox out of origin image range
***************************************************/

HI_S32 BboxClip(HI_S32* ps32Proposals, HI_U32 u32ImageW, HI_U32 u32ImageH);

/*deal with num*/
HI_S32 BboxClip_N(HI_S32* ps32Proposals, HI_U32 u32ImageW, HI_U32 u32ImageH, HI_U32 u32Num);

/* single size clip */
HI_S32 SizeClip(HI_S32 s32inputSize, HI_S32 s32sizeMin, HI_S32 s32sizeMax);

/**************************************************
Function: BboxSmallSizeFilter
Description: remove the bboxes which are too small
***************************************************/
HI_S32 BboxSmallSizeFilter(HI_S32* ps32Proposals, HI_U32 u32minW, HI_U32 u32minH);
HI_S32 BboxSmallSizeFilter_N(HI_S32* ps32Proposals, HI_U32 u32minW, HI_U32 u32minH, HI_U32 u32NumAnchors);

#endif
