#include <cmath>
#include"SvpNnieYolov3.h"
#include"SvpNnieFun.h"
#include"common.h"
/* YOLO V3 */
#define SVP_SAMPLE_YOLOV3_SRC_WIDTH                (416)
#define SVP_SAMPLE_YOLOV3_SRC_HEIGHT               (416)
#define SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_82          (13)
#define SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_94          (26)
#define SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_106         (52)
#define SVP_SAMPLE_YOLOV3_RESULT_BLOB_NUM          (3)
#define SVP_SAMPLE_YOLOV3_BOXNUM                   (3)


#define SVP_SAMPLE_YOLOV3_CHANNLENUM               (255)
#define SVP_SAMPLE_YOLOV3_PARAMNUM                 (85)
#define SVP_SAMPLE_YOLOV3_CLASSNUM                 (80)

#define SVP_SAMPLE_YOLOV3_MAX_BOX_NUM              (10)
#define SVP_SAMPLE_YOLOV3_NMS_THREASH              (0.45f)


typedef enum hiSVP_SAMPLE_YOLOV3_SCALE_TYPE
{
	CONV_82 = 0,
	CONV_94,
	CONV_106,
	SVP_SAMPLE_YOLOV3_SCALE_TYPE_MAX
}SVP_SAMPLE_YOLOV3_SCALE_TYPE_E;

typedef struct hiSVP_SAMPLE_RESULT_MEM_HEAD_S
{
	HI_U32 u32Type;
	HI_U32 u32Len;
	/* HI_U32* pu32Mem; */
}SVP_SAMPLE_RESULT_MEM_HEAD_S;



namespace nnie
{

	STATIC HI_DOUBLE SvpNnieYoloV3Bias[SVP_SAMPLE_YOLOV3_SCALE_TYPE_MAX][6] = {
		{116,90, 156,198, 373,326},
		{30,61, 62,45, 59,119},
		{10,13, 16,30, 33,23}
	};

	STATIC HI_U32 SvpNnieYoloV3WKGetGridNum(SVP_SAMPLE_YOLOV3_SCALE_TYPE_E enScaleType)
	{
		switch (enScaleType)
		{
		case CONV_82:
			return SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_82;
			break;
		case CONV_94:
			return SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_94;
			break;
		case CONV_106:
			return SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_106;
			break;
		default:
			return 0;
		}
	}

	STATIC HI_U32 SvpNnieYoloV3WKGetBoxTotleNum(SVP_SAMPLE_YOLOV3_SCALE_TYPE_E enScaleType)
	{
		switch (enScaleType)
		{
		case CONV_82:
			return SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_82 * SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_82 * SVP_SAMPLE_YOLOV3_BOXNUM;
		case CONV_94:
			return SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_94 * SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_94 * SVP_SAMPLE_YOLOV3_BOXNUM;
		case CONV_106:
			return SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_106 * SVP_SAMPLE_YOLOV3_GRIDNUM_CONV_82 * SVP_SAMPLE_YOLOV3_BOXNUM;
		default:
			return 0;
		}
	}
	STATIC HI_DOUBLE SvpNnieYoloDetCalIou(SVP_SAMPLE_BOX_S *pstBox1, SVP_SAMPLE_BOX_S *pstBox2)
	{
		/*** Check the input ***/

		HI_FLOAT f32XMin = SVP_MAX(pstBox1->f32Xmin, pstBox2->f32Xmin);
		HI_FLOAT f32YMin = SVP_MAX(pstBox1->f32Ymin, pstBox2->f32Ymin);
		HI_FLOAT f32XMax = SVP_MIN(pstBox1->f32Xmax, pstBox2->f32Xmax);
		HI_FLOAT f32YMax = SVP_MIN(pstBox1->f32Ymax, pstBox2->f32Ymax);

		HI_FLOAT InterWidth = f32XMax - f32XMin;
		HI_FLOAT InterHeight = f32YMax - f32YMin;

		if (InterWidth <= 0 || InterHeight <= 0) {
			return HI_SUCCESS;
		}

		HI_DOUBLE f64InterArea = InterWidth * InterHeight;
		HI_DOUBLE f64Box1Area = (pstBox1->f32Xmax - pstBox1->f32Xmin)* (pstBox1->f32Ymax - pstBox1->f32Ymin);
		HI_DOUBLE f64Box2Area = (pstBox2->f32Xmax - pstBox2->f32Xmin)* (pstBox2->f32Ymax - pstBox2->f32Ymin);

		HI_DOUBLE f64UnionArea = f64Box1Area + f64Box2Area - f64InterArea;

		return f64InterArea / f64UnionArea;
	}

	STATIC HI_U32 SvpNnieYolov3WKGetResultMemSize(SVP_SAMPLE_YOLOV3_SCALE_TYPE_E enScaleType)
	{
		HI_U32 u32GridNum = SvpNnieYoloV3WKGetGridNum(enScaleType);
		HI_U32 inputdate_size = u32GridNum * u32GridNum * SVP_SAMPLE_YOLOV3_CHANNLENUM * sizeof(HI_FLOAT);
		HI_U32 u32TmpBoxSize = u32GridNum * u32GridNum * SVP_SAMPLE_YOLOV3_CHANNLENUM * sizeof(HI_U32);
		HI_U32 u32BoxSize = u32GridNum * u32GridNum * SVP_SAMPLE_YOLOV3_BOXNUM * SVP_SAMPLE_YOLOV3_CLASSNUM * sizeof(SVP_SAMPLE_BOX_S);
		return (inputdate_size + u32TmpBoxSize + u32BoxSize);
	}

	STATIC HI_S32 SvpNnieYoloDetNonMaxSuppression(SVP_SAMPLE_BOX_S* pstBoxs, HI_U32 u32BoxNum, HI_FLOAT f32NmsThresh, HI_U32 u32MaxRoiNum)
	{
		for (HI_U32 i = 0, u32Num = 0; i < u32BoxNum && u32Num < u32MaxRoiNum; i++)
		{
			if (0 == pstBoxs[i].u32Mask)
			{
				u32Num++;
				for (HI_U32 j = i + 1; j < u32BoxNum; j++)
				{
					if (0 == pstBoxs[j].u32Mask)
					{
						HI_DOUBLE f64Iou = SvpNnieYoloDetCalIou(&pstBoxs[i], &pstBoxs[j]);
						if (f64Iou >= (HI_DOUBLE)f32NmsThresh)
						{
							pstBoxs[j].u32Mask = 1;
						}
					}
				}
			}
		}

		return HI_SUCCESS;
	}

	STATIC void SvpNnieYoloV3GetResultForOneBlob(SVP_BLOB_S *pstDstBlob,HI_U8* pu8InputData,SVP_SAMPLE_YOLOV3_SCALE_TYPE_E enScaleType,HI_S32 *ps32ResultMem,
		SVP_SAMPLE_BOX_S** ppstBox,HI_U32 *pu32BoxNum,HI_FLOAT YOLOV3_THREASH)
	{
		// result calc para config
		HI_FLOAT f32ScoreFilterThresh = YOLOV3_THREASH;

		HI_U32 u32GridNum = SvpNnieYoloV3WKGetGridNum(enScaleType);
		HI_U32 u32CStep = u32GridNum * u32GridNum;
		HI_U32 u32HStep = u32GridNum;

		HI_U32 inputdate_size = u32GridNum * u32GridNum * SVP_SAMPLE_YOLOV3_CHANNLENUM;
		HI_U32 u32TmpBoxSize = u32GridNum * u32GridNum * SVP_SAMPLE_YOLOV3_CHANNLENUM;

		HI_FLOAT* pf32InputData = (HI_FLOAT*)ps32ResultMem;
		HI_FLOAT* pf32BoxTmp = (HI_FLOAT*)(pf32InputData + inputdate_size);////tep_box_size
		SVP_SAMPLE_BOX_S* pstBox = (SVP_SAMPLE_BOX_S*)(pf32BoxTmp + u32TmpBoxSize);////assit_box_size
#ifdef DEBUG
		//SAMPLE_PRINT("[INFO][DEBUG]n:%u, c:%u, h:%u, w:%u\n",pstDstBlob->u32Num,pstDstBlob->unShape.stWhc.u32Chn,pstDstBlob->unShape.stWhc.u32Height,pstDstBlob->unShape.stWhc.u32Width);
#endif

		if ((u32GridNum != pstDstBlob->unShape.stWhc.u32Height) ||
			(u32GridNum != pstDstBlob->unShape.stWhc.u32Width) ||
			(SVP_SAMPLE_YOLOV3_CHANNLENUM != pstDstBlob->unShape.stWhc.u32Chn))
		{
			SAMPLE_PRINT("[ERROR]error grid number!\n");
			return;
		}

		HI_U32 u32OneCSize = pstDstBlob->u32Stride * pstDstBlob->unShape.stWhc.u32Height;
		HI_U32 u32BoxsNum = 0;

		{
			HI_U32 n = 0;
			for (HI_U32 c = 0; c < SVP_SAMPLE_YOLOV3_CHANNLENUM; c++) {
				for (HI_U32 h = 0; h < u32GridNum; h++) {
					for (HI_U32 w = 0; w < u32GridNum; w++) {
						HI_S32* ps32Temp = (HI_S32*)(pu8InputData + c * u32OneCSize + h * pstDstBlob->u32Stride) + w;
						pf32InputData[n++] = (HI_FLOAT)(*ps32Temp) / SVP_WK_QUANT_BASE;
					}
				}
			}
		}
		{
			HI_U32 n = 0;
			for (HI_U32 h = 0; h < u32GridNum; h++) {
				for (HI_U32 w = 0; w < u32GridNum; w++) {
					for (HI_U32 c = 0; c < SVP_SAMPLE_YOLOV3_CHANNLENUM; c++) {
						pf32BoxTmp[n++] = pf32InputData[c * u32CStep + h * u32HStep + w];
					}
				}
			}
		}

		for (HI_U32 n = 0; n < u32GridNum * u32GridNum; n++)
		{
			//Grid
			HI_U32 w = n % u32GridNum;
			HI_U32 h = n / u32GridNum;
			for (HI_U32 k = 0; k < SVP_SAMPLE_YOLOV3_BOXNUM; k++)
			{
				HI_U32 u32Index = (n * SVP_SAMPLE_YOLOV3_BOXNUM + k) * SVP_SAMPLE_YOLOV3_PARAMNUM;
				HI_FLOAT x = ((HI_FLOAT)w + Sigmoid(pf32BoxTmp[u32Index + 0])) / u32GridNum;
				HI_FLOAT y = ((HI_FLOAT)h + Sigmoid(pf32BoxTmp[u32Index + 1])) / u32GridNum;
				HI_FLOAT f32Width = (HI_FLOAT)(exp(pf32BoxTmp[u32Index + 2]) * SvpNnieYoloV3Bias[enScaleType][2 * k]) / SVP_SAMPLE_YOLOV3_SRC_WIDTH;
				HI_FLOAT f32Height = (HI_FLOAT)(exp(pf32BoxTmp[u32Index + 3]) * SvpNnieYoloV3Bias[enScaleType][2 * k + 1]) / SVP_SAMPLE_YOLOV3_SRC_HEIGHT;

				HI_FLOAT f32ObjScore = Sigmoid(pf32BoxTmp[u32Index + 4]); //objscore;
				if (f32ObjScore <= f32ScoreFilterThresh) {
					continue;
				}

				for (HI_U32 classIdx = 0; classIdx < SVP_SAMPLE_YOLOV3_CLASSNUM; classIdx++)
				{
					HI_U32 u32ClassIdxBase = u32Index + 4 + 1;
					HI_FLOAT f32ClassScore = Sigmoid(pf32BoxTmp[u32ClassIdxBase + classIdx]); //objscore;
					HI_FLOAT f32Prob = f32ObjScore * f32ClassScore;

					pstBox[u32BoxsNum].f32Xmin = x - f32Width * 0.5f;  // xmin
					pstBox[u32BoxsNum].f32Xmax = x + f32Width * 0.5f;  // xmax
					pstBox[u32BoxsNum].f32Ymin = y - f32Height * 0.5f; // ymin
					pstBox[u32BoxsNum].f32Ymax = y + f32Height * 0.5f; // ymax
					pstBox[u32BoxsNum].f32ClsScore = f32Prob;          // predict prob
					pstBox[u32BoxsNum].u32MaxScoreIndex = classIdx;    // class score index
					pstBox[u32BoxsNum].u32Mask = 0;                    // Suppression mask

					u32BoxsNum++;
				}
			}
		}
		*ppstBox = pstBox;
		*pu32BoxNum = u32BoxsNum;

	}

	STATIC void SvpNnieYoloV3BoxPostProcess(SVP_SAMPLE_BOX_S* pstInputBbox, HI_U32 u32InputBboxNum,SVP_SAMPLE_BOX_S* pstResultBbox, HI_U32 *pu32BoxNum)
	{
		HI_FLOAT f32NmsThresh = SVP_SAMPLE_YOLOV3_NMS_THREASH;
		HI_U32 u32MaxBoxNum = SVP_SAMPLE_YOLOV3_MAX_BOX_NUM;
		HI_U32 u32SrcWidth = SVP_SAMPLE_YOLOV3_SRC_WIDTH;
		HI_U32 u32SrcHeight = SVP_SAMPLE_YOLOV3_SRC_HEIGHT;

		HI_U32 u32AssistStackNum = SvpNnieYoloV3WKGetBoxTotleNum(CONV_82)+ SvpNnieYoloV3WKGetBoxTotleNum(CONV_94)+ SvpNnieYoloV3WKGetBoxTotleNum(CONV_106);

		HI_U32 u32AssistStackSize = u32AssistStackNum * sizeof(SVP_SAMPLE_STACK_S);

		SVP_SAMPLE_STACK_S* pstAssistStack = (SVP_SAMPLE_STACK_S*)malloc(u32AssistStackSize);////assit_size
		SAMPLE_CHECK_RETURN_VOID(pstAssistStack == NULL, void, "[ERROR]malloc failed!\n");
		memset(pstAssistStack, 0, u32AssistStackSize);

		//quick_sort
		NonRecursiveArgQuickSortWithBox(pstInputBbox, 0, u32InputBboxNum - 1, pstAssistStack);
		free(pstAssistStack);

		//Nms
		SvpNnieYoloDetNonMaxSuppression(pstInputBbox, u32InputBboxNum, f32NmsThresh, u32MaxBoxNum);

		//Get the result
		HI_U32 u32BoxResultNum = 0;
		for (HI_U32 n = 0; (n < u32InputBboxNum) && (u32BoxResultNum < u32MaxBoxNum); n++) {
			if (0 == pstInputBbox[n].u32Mask) {
				pstResultBbox[u32BoxResultNum].f32Xmin = SVP_SAMPLE_MAX(pstInputBbox[n].f32Xmin * u32SrcWidth, 0);
				pstResultBbox[u32BoxResultNum].f32Xmax = SVP_SAMPLE_MIN(pstInputBbox[n].f32Xmax * u32SrcWidth, u32SrcWidth);
				pstResultBbox[u32BoxResultNum].f32Ymax = SVP_SAMPLE_MIN(pstInputBbox[n].f32Ymax * u32SrcHeight, u32SrcHeight);
				pstResultBbox[u32BoxResultNum].f32Ymin = SVP_SAMPLE_MAX(pstInputBbox[n].f32Ymin * u32SrcHeight, 0);
				pstResultBbox[u32BoxResultNum].f32ClsScore = pstInputBbox[n].f32ClsScore;
				pstResultBbox[u32BoxResultNum].u32MaxScoreIndex = pstInputBbox[n].u32MaxScoreIndex;

				u32BoxResultNum++;
			}
		}

		
		if (0 == u32BoxResultNum) {
			return;
		}

		*pu32BoxNum += u32BoxResultNum;

	}

	STATIC void SvpNnieYoloV3WKGetResult(SVP_BLOB_S *pstDstBlob, HI_S32 *ps32ResultMem, SVP_SAMPLE_BOX_RESULT_INFO_S *pstResultBoxInfo,
		HI_U32 *pu32BoxNum, HI_FLOAT YOLOV3_THREASH)
	{
		SVP_SAMPLE_BOX_S* pstResultBbox = pstResultBoxInfo->pstBbox;
		HI_U32 u32ResultBoxNum = 0;
		SVP_SAMPLE_BOX_RESULT_INFO_S stTempBoxResultInfo;

		SVP_SAMPLE_BOX_S* pstTempBbox = NULL;
		HI_U32 u32TempBoxNum = 0;

		for (HI_U32 u32NumIndex = 0; u32NumIndex < pstDstBlob->u32Num; u32NumIndex++)
		{
			SVP_SAMPLE_BOX_S* apstBox[SVP_SAMPLE_YOLOV3_RESULT_BLOB_NUM] = { NULL };
			HI_U32 au32BoxNum[SVP_SAMPLE_YOLOV3_RESULT_BLOB_NUM] = { 0 };

			SVP_SAMPLE_RESULT_MEM_HEAD_S *pstHead = (SVP_SAMPLE_RESULT_MEM_HEAD_S *)ps32ResultMem;

			for (HI_U32 u32resBlobIdx = 0; u32resBlobIdx < SVP_SAMPLE_YOLOV3_RESULT_BLOB_NUM; u32resBlobIdx++)
			{
				SVP_BLOB_S* pstTempBlob = &pstDstBlob[u32resBlobIdx];
				HI_U32 u32OneCSize = pstTempBlob->u32Stride * pstTempBlob->unShape.stWhc.u32Height;
				HI_U32 u32FrameStride = u32OneCSize * pstTempBlob->unShape.stWhc.u32Chn;
				HI_U8* pu8InputData = (HI_U8*)pstTempBlob->u64VirAddr + u32NumIndex * u32FrameStride;

				if (HI_NULL != pstHead)
				{
					SvpNnieYoloV3GetResultForOneBlob(pstTempBlob,
						pu8InputData,
						(SVP_SAMPLE_YOLOV3_SCALE_TYPE_E)pstHead->u32Type,
						(HI_S32*)(pstHead + 1),
						&apstBox[u32resBlobIdx], &au32BoxNum[u32resBlobIdx], YOLOV3_THREASH);
				}

				if (u32resBlobIdx < SVP_SAMPLE_YOLOV3_RESULT_BLOB_NUM - 1) {
					pstHead = (SVP_SAMPLE_RESULT_MEM_HEAD_S*)((HI_U8 *)(pstHead + 1) + pstHead->u32Len);
				}
			}

			u32TempBoxNum = au32BoxNum[0] + au32BoxNum[1] + au32BoxNum[2];
			pstTempBbox = (SVP_SAMPLE_BOX_S*)malloc(sizeof(SVP_SAMPLE_BOX_S) * u32TempBoxNum);
			SAMPLE_CHECK_RETURN_VOID(pstTempBbox == NULL,void,"[ERROR]malloc failed!\n");
			memcpy((HI_U8*)pstTempBbox, (HI_U8*)apstBox[0], sizeof(SVP_SAMPLE_BOX_S) * au32BoxNum[0]);
			memcpy((HI_U8*)(pstTempBbox + au32BoxNum[0]), (HI_U8*)apstBox[1], sizeof(SVP_SAMPLE_BOX_S) * au32BoxNum[1]);
			memcpy((HI_U8*)(pstTempBbox + au32BoxNum[0] + au32BoxNum[1]), (HI_U8*)apstBox[2], sizeof(SVP_SAMPLE_BOX_S) * au32BoxNum[2]);

			SvpNnieYoloV3BoxPostProcess(pstTempBbox, u32TempBoxNum, &pstResultBbox[u32ResultBoxNum], &pu32BoxNum[u32NumIndex]);

			stTempBoxResultInfo.u32OriImHeight = pstResultBoxInfo->u32OriImHeight;
			stTempBoxResultInfo.u32OriImWidth = pstResultBoxInfo->u32OriImWidth;
			stTempBoxResultInfo.pstBbox = &pstResultBbox[u32ResultBoxNum];

			u32ResultBoxNum = u32ResultBoxNum + pu32BoxNum[u32NumIndex];
			if (u32ResultBoxNum >= 1024)
			{
				SAMPLE_PRINT("[WARING]Box number reach max 1024!\n");
				free(pstTempBbox);
				return;
			}
			free(pstTempBbox);
		}
	}

	STATIC HI_S32*SvpNnieYoloV3AllocResultMem(void)
	{
		HI_S32 *ps32Mem = HI_NULL;
		HI_U32 u32ResultMemSize = 0;


		// yolov3 special mem init method
		HI_U32 u32ResultMemSize1 = 0;
		HI_U32 u32ResultMemSize2 = 0;
		HI_U32 u32ResultMemSize3 = 0;
		SVP_SAMPLE_RESULT_MEM_HEAD_S *pstHead = { 0 };
		u32ResultMemSize1 = SvpNnieYolov3WKGetResultMemSize(CONV_82);
		u32ResultMemSize2 = SvpNnieYolov3WKGetResultMemSize(CONV_94);
		u32ResultMemSize3 = SvpNnieYolov3WKGetResultMemSize(CONV_106);
		if (0 != (u32ResultMemSize1 + u32ResultMemSize2 + u32ResultMemSize3))
		{
			u32ResultMemSize = u32ResultMemSize1 + u32ResultMemSize2 + u32ResultMemSize3 + sizeof(SVP_SAMPLE_RESULT_MEM_HEAD_S) * 3;
			ps32Mem = (HI_S32*)malloc(u32ResultMemSize);
			if (HI_NULL != ps32Mem)
			{
				memset(ps32Mem, 0, u32ResultMemSize);

				pstHead = (SVP_SAMPLE_RESULT_MEM_HEAD_S *)ps32Mem;
				pstHead->u32Type = CONV_82;
				pstHead->u32Len = u32ResultMemSize1;

				pstHead = (SVP_SAMPLE_RESULT_MEM_HEAD_S *)(((HI_U8 *)pstHead) + sizeof(SVP_SAMPLE_RESULT_MEM_HEAD_S) + u32ResultMemSize1);
				pstHead->u32Type = CONV_94;
				pstHead->u32Len = u32ResultMemSize2;

				pstHead = (SVP_SAMPLE_RESULT_MEM_HEAD_S *)(((HI_U8 *)pstHead) + sizeof(SVP_SAMPLE_RESULT_MEM_HEAD_S) + u32ResultMemSize2);
				pstHead->u32Type = CONV_106;
				pstHead->u32Len = u32ResultMemSize3;
			}
		}


		return ps32Mem;
	}

	STATIC HI_S32 SvpNnieYoloV3GetResult(SVP_NNIE_ONE_SEG_S *pstDetParam, HI_S32 *ps32ResultMem,HI_FLOAT YOLOV3_THREASH, std::vector<TargetBox>&boxs)
	{

		HI_S32 s32Ret = HI_SUCCESS;
		SVP_SAMPLE_BOX_S astBoxesResult[1024] = { 0 };

		HI_U32* p32BoxNum = (HI_U32*)malloc(pstDetParam->srcBlobs->u32Num * sizeof(HI_U32));
		SAMPLE_CHECK_RETURN(p32BoxNum == NULL, HI_FAILURE, "[ERROR]malloc failed!\n");
		memset(p32BoxNum, 0, pstDetParam->srcBlobs->u32Num * sizeof(HI_U32));

		SVP_SAMPLE_BOX_RESULT_INFO_S stBoxesInfo = { 0 };
		stBoxesInfo.pstBbox = astBoxesResult;
		stBoxesInfo.u32OriImHeight = pstDetParam->srcBlobs[0].unShape.stWhc.u32Height;
		stBoxesInfo.u32OriImWidth = pstDetParam->srcBlobs[0].unShape.stWhc.u32Width;


		SvpNnieYoloV3WKGetResult(pstDetParam->dstBlobs, ps32ResultMem,&stBoxesInfo, p32BoxNum, YOLOV3_THREASH);

		//TBD: batch images result process
		//need get batch astBoxesResult from front detection result calculate

		for (HI_U32 j = 0; j < pstDetParam->dstBlobs->u32Num; j++)
		{
			for (HI_U32 i = 0; i < p32BoxNum[j]; i++)
			{
				TargetBox box;
				box.stRect.x = (HI_U32)astBoxesResult[i].f32Xmin;
				box.stRect.y = (HI_U32)astBoxesResult[i].f32Ymin;
				box.stRect.w = (HI_U32)(astBoxesResult[i].f32Xmax - astBoxesResult[i].f32Xmin);
				box.stRect.h = (HI_U32)(astBoxesResult[i].f32Ymax - astBoxesResult[i].f32Ymin);
				box.fScore = astBoxesResult[i].f32ClsScore;
				box.u32Class = astBoxesResult[i].u32MaxScoreIndex;
				boxs.push_back(box);
			}

		}
		free(p32BoxNum);
		return s32Ret;
	}




	NetYoloV3::NetYoloV3(const char * wkfile, HI_U32 maxBatch)
		:NetOneSeg(wkfile, maxBatch)
	{
		this->init();
	}
	NetYoloV3::~NetYoloV3()
	{
		this->destory();
	}
	void NetYoloV3::init()
	{
		data = SvpNnieYoloV3AllocResultMem();
	}

	void NetYoloV3::destory()
	{
		if (data != NULL)
		{
			free(data);
		}
	}

	HI_S32 NetYoloV3::detect(const HI_U8 * bgr, HI_U32 n, HI_U32 c, HI_U32 h, HI_U32 w, HI_U32 stride, HI_FLOAT threshold,std::vector<TargetBox> &boxs)
	{
#ifdef DEBUG
		float timeStart = getTimeMs();
#endif
		HI_S32 ret = NetOneSeg::setBlob(bgr, n, c, h, w, stride, &NetOneSeg::srcBlobs[0].second);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != ret, ret, "[ERROR]set Blob[%s] failed\n", NetOneSeg::srcBlobs[0].first.c_str());

		ret = NetOneSeg::forword();
		SAMPLE_CHECK_RETURN(HI_SUCCESS != ret, ret, "[ERROR]NNIE forword failed\n");

		
		ret = SvpNnieYoloV3GetResult(&(NetOneSeg::NNIEOneSegNetParam), (HI_S32*)data, threshold, boxs);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != ret, ret, "[ERROR]YoloV3GetResult failed\n");

#ifdef DEBUG
		float timeEnd = getTimeMs();
		SAMPLE_PRINT("[INFO] == DETECT TIME SPEND  : %f ms== \n", timeEnd - timeStart);
		SAMPLE_PRINT("[INFO] %s-%d-done\n", __FUNCTION__, __LINE__);
#endif

	}











}