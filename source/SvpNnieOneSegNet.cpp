#include<string>
#include<fstream>
#ifdef _MSC_VER
#include<io.h>
#else
#include <linux/limits.h>
#include <unistd.h>
#endif
#include"SvpNnieOneSegNet.h"
#include"SvpNnieCommon.h"
#include"common.h"
#define  MAX_BATCH_SIZE  256
namespace nnie
{
	STATIC HI_S32 loadModelFile(const char* pcModelFile, SVP_MEM_INFO_S* pstMemInfo)
	{
		HI_S32 s32Ret = HI_FAILURE;
		SAMPLE_CHECK_RETURN(NULL == pcModelFile, HI_FAILURE, "[ERROR]param model file path == NULL\n");
		SAMPLE_CHECK_RETURN(NULL == pstMemInfo, HI_FAILURE, "[ERROR]param SVP_MEM_INFO_S == NULL\n");
		HI_S32  WkLen = 0;
		std::string fileName((const char*)pcModelFile);
		SAMPLE_CHECK_RETURN(!(ACCESS(fileName.c_str(), 0) == 0), HI_FAILURE, ("[ERROR]" + fileName + "is not access\n").c_str());

		FILE* fp = fopen(fileName.c_str(), "rb");
		SAMPLE_CHECK_RETURN(NULL == fp, HI_FAILURE, ("[ERROR]" + fileName + "open filed\n").c_str());


		s32Ret = fseek(fp, 0L, SEEK_END);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, CLOSE_FILE, "[ERROR]fseek failed!\n");

		WkLen = ftell(fp);
		SAMPLE_CHECK_GOTO(0 != WkLen % 16, CLOSE_FILE, ("[ERROR]" + fileName + "WK Len %% 16 != 0 \n").c_str());

		SAMPLE_PRINT("Runtime WK Len: %d\n", WkLen);

		SAMPLE_CHECK_GOTO(0 != fseek(fp, 0L, SEEK_SET), CLOSE_FILE, "[ERROR]fseek fail");

		pstMemInfo->u32Size = WkLen;
		s32Ret = SAMPLE_SVP_AllocMem(WkLen, pstMemInfo, HI_FALSE);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FREE_MEM, "[ERROR]SAMPLE_SVP_AllocMem is faile\n");
		s32Ret = SAMPLE_SVP_FlushCache(pstMemInfo);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, CLOSE_FILE, "[ERROR]SAMPLE_SVP_FlushCache failed!\n");

		s32Ret = (HI_S32)fread((HI_VOID*)((HI_UL)pstMemInfo->u64VirAddr), WkLen, 1, fp);
		SAMPLE_CHECK_GOTO(1 != s32Ret, FREE_MEM, "[ERROR]Read WK failed!\n");

		fclose(fp);
		return HI_SUCCESS;
	FREE_MEM:
		SAMPLE_SVP_FreeMem(pstMemInfo);
	CLOSE_FILE:
		fclose(fp);
		return HI_FAILURE;
	}

	NetOneSeg::NetOneSeg(const char * wkfile, HI_U32 maxBatch)
	{
		this->init(wkfile,maxBatch);
	}
	NetOneSeg::~NetOneSeg()
	{
		this->destory();
	}

	HI_S32 NetOneSeg::init(const char * wkfile, HI_U32 maxBatch)
	{
		HI_U32 batchSize = maxBatch;
		HI_U32 maxRoiNum = 0;
		HI_U32 u32MaxTaskSize = 0;
		if (maxBatch <= 0)
		{
			batchSize = 1;
		}
		if (maxBatch >= MAX_BATCH_SIZE)
		{
			batchSize = MAX_BATCH_SIZE;
		}
#ifdef DEBUG
		uint64_t timeStart = getTimeMs();
#endif 

		/******************** step1, load wk file, *******************************/
		HI_S32 s32Ret = HI_SUCCESS;
		s32Ret = loadModelFile(wkfile, &NNIEOneSegNetParam.modelBuffer);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]loadModelFile  failed\n");

		NNIEOneSegNetParam.modelBufferSize = NNIEOneSegNetParam.modelBuffer.u32Size;
		s32Ret = HI_MPI_SVP_NNIE_LoadModel(&NNIEOneSegNetParam.modelBuffer, &NNIEOneSegNetParam.model);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]HI_MPI_SVP_NNIE_LoadModel is faile\n");

		SAMPLE_CHECK_GOTO(NNIEOneSegNetParam.model.u32NetSegNum != 1, FAIL_1, "netSegNum should be = 1");


		/******************** step2, malloc tmp_buf *******************************/
		//s32Ret = nnie::SAMPLE_RUNTIME_HiMemAlloc(NNIEOneSegNetParam.stModel.u32TmpBufSize, &NNIEOneSegNetParam.nnieCtrl.stTmpBuf, HI_TRUE);
		s32Ret = SAMPLE_SVP_AllocMem(NNIEOneSegNetParam.model.u32TmpBufSize, &NNIEOneSegNetParam.nnieCtrl.stTmpBuf, HI_FALSE);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FAIL_1, "[ERROR]SAMPLE_SVP_AllocMem is faile\n");
		s32Ret = SAMPLE_SVP_FlushCache(&NNIEOneSegNetParam.nnieCtrl.stTmpBuf);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FAIL_2, "[ERROR]SAMPLE_SVP_FlushCache failed!\n");


		NNIEOneSegNetParam.tmpBuffer = NNIEOneSegNetParam.nnieCtrl.stTmpBuf;
		NNIEOneSegNetParam.tmpBufferSize = NNIEOneSegNetParam.model.u32TmpBufSize;


		/******************** step3, get tsk_buf size *******************************/
		HI_U32 au32TaskBufSize[SVP_NNIE_MAX_NET_SEG_NUM];
		memset(au32TaskBufSize, 0, sizeof(au32TaskBufSize));
		s32Ret = HI_MPI_SVP_NNIE_GetTskBufSize(SVP_NNIE_MAX_INPUT_NUM, maxRoiNum, &NNIEOneSegNetParam.model, au32TaskBufSize, NNIEOneSegNetParam.model.u32NetSegNum);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]HI_MPI_SVP_NNIE_GetTskBufSize is faile\n");

		for (HI_U32 i = 0; i < NNIEOneSegNetParam.model.u32NetSegNum; i++)
		{
			if (u32MaxTaskSize < au32TaskBufSize[i]) 
			{
				u32MaxTaskSize = au32TaskBufSize[i];
			}
		}

		NNIEOneSegNetParam.nnieCtrl.stTskBuf.u32Size = u32MaxTaskSize;

		/******************** step4, malloc tsk_buf size *******************************/
		s32Ret = SAMPLE_SVP_AllocMem(u32MaxTaskSize, &NNIEOneSegNetParam.nnieCtrl.stTskBuf, HI_FALSE);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FAIL_2, "[ERROR]SAMPLE_SVP_AllocMem is faile\n");
		s32Ret = SAMPLE_SVP_FlushCache(&NNIEOneSegNetParam.nnieCtrl.stTskBuf);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FAIL_3, "[ERROR]SAMPLE_SVP_FlushCache failed!\n");


		NNIEOneSegNetParam.taskBuffer = NNIEOneSegNetParam.nnieCtrl.stTskBuf;
		NNIEOneSegNetParam.taskBufferSize = u32MaxTaskSize;


		SVP_NNIE_NODE_S* pNodeSrc;
		pNodeSrc = NNIEOneSegNetParam.model.astSeg[0].astSrcNode;
		SVP_NNIE_NODE_S* pNodeDst;
		pNodeDst  = NNIEOneSegNetParam.model.astSeg[0].astDstNode;

		for (int x = 0; x < NNIEOneSegNetParam.model.astSeg[0].u16SrcNum; x++)
		{
			SVP_BLOB_TYPE_E enType = pNodeSrc->enType;
			HI_U32 u32SrcC = pNodeSrc->unShape.stWhc.u32Chn;
			HI_U32 u32SrcW = pNodeSrc->unShape.stWhc.u32Width;
			HI_U32 u32SrcH = pNodeSrc->unShape.stWhc.u32Height;
			std::string name = pNodeSrc->szName;
			std::pair<std::string, SVP_BLOB_S> package;
			package.first = name;
			SVP_BLOB_S blob;
			s32Ret = SAMPLE_SVP_BlobSetAndAlloc(&blob,enType, batchSize, u32SrcW, u32SrcH, u32SrcC, ALIGN_16);
			SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FAIL_3, "[ERROR]SAMPLE_SVP_BlobSetAndAlloc is failed\n");
			package.second = blob;
			srcBlobs.push_back(package);
			++pNodeSrc;
		}

		for (int x = 0; x < NNIEOneSegNetParam.model.astSeg[0].u16DstNum; x++)
		{
			SVP_BLOB_TYPE_E enType = pNodeDst->enType;
			HI_U32 u32SrcC = pNodeDst->unShape.stWhc.u32Chn;
			HI_U32 u32SrcW = pNodeDst->unShape.stWhc.u32Width;
			HI_U32 u32SrcH = pNodeDst->unShape.stWhc.u32Height;
			std::string name = pNodeDst->szName;
			std::pair<std::string, SVP_BLOB_S> package;
			package.first = name;
			SVP_BLOB_S blob;
			s32Ret = SAMPLE_SVP_BlobSetAndAlloc(&blob,enType, batchSize, u32SrcW, u32SrcH, u32SrcC, ALIGN_16 );
			SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FAIL_3, "[ERROR]SAMPLE_SVP_BlobSetAndAlloc is failed\n");
			package.second = blob;
			dstBlobs.push_back(package);
			++pNodeDst;
		}

		memset(NNIEOneSegNetParam.srcBlobs, 0, sizeof(NNIEOneSegNetParam.srcBlobs));
		memset(NNIEOneSegNetParam.dstBlobs, 0, sizeof(NNIEOneSegNetParam.dstBlobs));

		for (int i = 0; i < SVP_NNIE_MAX_INPUT_NUM && i < srcBlobs.size(); i++)
		{
			NNIEOneSegNetParam.srcBlobs[i] = srcBlobs[i].second;
		}
		for (int i = 0; i < SVP_NNIE_MAX_INPUT_NUM && i < dstBlobs.size(); i++)
		{
			NNIEOneSegNetParam.dstBlobs[i] = dstBlobs[i].second;
		}
#ifdef DEBUG
		uint64_t timeEnd;
		timeEnd  = getTimeMs();
		SAMPLE_PRINT("[INFO] == LOAD MODEL TIME : %ld ms== \n", timeEnd - timeStart);
		SAMPLE_PRINT("%s-%d-done:%s\n", __FUNCTION__, __LINE__, wkfile);
#endif
		return HI_SUCCESS;

	FAIL_3:
		s32Ret = SAMPLE_SVP_FreeMem(&NNIEOneSegNetParam.nnieCtrl.stTskBuf);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]SAMPLE_SVP_FreeMem is faile\n");

	FAIL_2:
		s32Ret = SAMPLE_SVP_FreeMem(&NNIEOneSegNetParam.nnieCtrl.stTmpBuf);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]SAMPLE_SVP_FreeMem is faile\n");

	FAIL_1:
		s32Ret = HI_MPI_SVP_NNIE_UnloadModel(&NNIEOneSegNetParam.model);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]HI_MPI_SVP_NNIE_UnloadModel is faile\n");
	FAIL_0:
		s32Ret = SAMPLE_SVP_FreeMem(&NNIEOneSegNetParam.modelBuffer);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]SAMPLE_SVP_FreeMem is faile\n");
		return HI_FAILURE;

	}
	HI_S32 NetOneSeg::destory()
	{
		HI_U32 s32Ret;
		//free model
		s32Ret = SAMPLE_SVP_FreeMem(&NNIEOneSegNetParam.modelBuffer);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]SAMPLE_FreeMem is failed\n");
		//free buffer
		s32Ret = SAMPLE_SVP_FreeMem(&NNIEOneSegNetParam.nnieCtrl.stTskBuf);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]SAMPLE_FreeMem is failed\n");
		s32Ret = SAMPLE_SVP_FreeMem(&NNIEOneSegNetParam.nnieCtrl.stTmpBuf);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]SAMPLE_FreeMem is failed\n");


		for (std::vector<std::pair<std::string, SVP_BLOB_S>>::iterator itNetOne = srcBlobs.begin(); itNetOne != srcBlobs.end(); itNetOne++)
		{
				s32Ret = SAMPLE_SVP_BlobFree(&itNetOne->second);
				SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]SAMPLE_SVP_BlobFree is failed\n");
		}

		for (std::vector<std::pair<std::string, SVP_BLOB_S>>::iterator itNetOne = dstBlobs.begin(); itNetOne != dstBlobs.end(); itNetOne++)
		{
				s32Ret = SAMPLE_SVP_BlobFree(&itNetOne->second);
				SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]SAMPLE_SVP_BlobFree is failed\n")
		}
		// unload model
		s32Ret = HI_MPI_SVP_NNIE_UnloadModel(&NNIEOneSegNetParam.model);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, s32Ret, "[ERROR]HI_MPI_SVP_NNIE_UnloadModel is faile\n");
		return s32Ret;
	}

	HI_S32  NetOneSeg::setBlob(const HI_U8 * bgr, HI_U32 n, HI_U32 chn, HI_U32 height, HI_U32 width, HI_U32 stride, SVP_BLOB_S * srcBlob)
	{
		SVP_BLOB_S * pstBlob = srcBlob;
		HI_U32 u16Index = 0;
		SVP_BLOB_TYPE_E blob_type = pstBlob->enType;
		HI_U32 u32BlobChn = pstBlob->unShape.stWhc.u32Chn;
		HI_U32 u32BlobWidth = pstBlob->unShape.stWhc.u32Width;
		HI_U32 u32BlobHeight = pstBlob->unShape.stWhc.u32Height;
		HI_U32 blob_stride = pstBlob->u32Stride;
		HI_U8* pu8DstAddr = (HI_U8*)(pstBlob->u64VirAddr + u16Index * u32BlobChn * u32BlobHeight * pstBlob->u32Stride);
#ifdef DEBUG
		HI_U8* pu8DstAddrDebug = (HI_U8*)(pstBlob->u64VirAddr + u16Index * u32BlobChn * u32BlobHeight * pstBlob->u32Stride);
#endif
		if (width != u32BlobWidth || height != u32BlobHeight || chn != u32BlobChn )
		{
			SAMPLE_PRINT("[ERROR]bgr's shape !=  blob's shape\n");
			return HI_FAILURE;
		}


		HI_U32 blob_OneElementSize = SAMPLE_SVP_BlobGetElementSize(pstBlob);

		HI_U8* pu8SrcAddr = (HI_U8*)(bgr);

		if (blob_OneElementSize != sizeof(HI_U8) || blob_type != SVP_BLOB_TYPE_U8)
		{
			SAMPLE_PRINT("[ERROR]blob_OneElementSize is not uint8_t\n");
			return HI_FAILURE;
		}


		//blob   bbbgggrrr
		for (HI_U32 c = 0; c < u32BlobChn; c++)
		{

			for (HI_U32 h = 0; h < u32BlobHeight; h++)
			{
				HI_U32 index = 0;
				HI_U8 * line_bgr = pu8SrcAddr + h * stride;   //此处需要跟mpp的image的stride概念相同，需要注意，如不同，需要修改
				if (h >= (HI_U32)height)
				{
					for (HI_U32 w = 0; w < u32BlobWidth; w++)
					{
						pu8DstAddr[index++] = 0;
					}
				}
				else
				{
					for (HI_U32 w = 0; w < u32BlobWidth; w++)
					{
						HI_U8 * point_bgr = line_bgr + w * chn;
						if (w >= (HI_U32)width)
						{
							pu8DstAddr[index++] = 0;
						}
						else
						{
							pu8DstAddr[index++] = point_bgr[c];
						}
					}
				}
				pu8DstAddr += pstBlob->u32Stride;
			}
		}
#ifdef USE_OPENCV
		cv::Mat debug(u32BlobHeight, u32BlobWidth, CV_8UC3, pu8DstAddrDebug);
#endif
		return HI_SUCCESS;
	}

	HI_S32  NetOneSeg::forword()
	{
		HI_S32 s32Ret;
		HI_BOOL bInstant = HI_TRUE;
		HI_BOOL bFinish = HI_FALSE;
#ifdef DEBUG
		uint64_t timeStart = getTimeMs();
#endif
		/************************** step8, set ctrl param **************************/
		NNIEOneSegNetParam.nnieCtrl.enNnieId = NNIEDevId;


		NNIEOneSegNetParam.nnieCtrl.u32NetSegId = 0;
		NNIEOneSegNetParam.nnieCtrl.u32SrcNum = NNIEOneSegNetParam.model.astSeg[NNIEOneSegNetParam.nnieCtrl.u32NetSegId].u16SrcNum;
		NNIEOneSegNetParam.nnieCtrl.u32DstNum = NNIEOneSegNetParam.model.astSeg[NNIEOneSegNetParam.nnieCtrl.u32NetSegId].u16DstNum;

		s32Ret = HI_MPI_SVP_NNIE_Forward(&NNIEHandle, NNIEOneSegNetParam.srcBlobs, &NNIEOneSegNetParam.model, NNIEOneSegNetParam.dstBlobs, &NNIEOneSegNetParam.nnieCtrl, bInstant);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, false, "[EEROR]HI_MPI_SVP_NNIE_Forward is failed\n");
		s32Ret = HI_MPI_SVP_NNIE_Query(NNIEOneSegNetParam.nnieCtrl.enNnieId, NNIEHandle, &bFinish, bInstant);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, false, "[EEROR]HI_MPI_SVP_NNIE_Query is failed\n");
		


#ifdef DEBUG
		uint64_t timeEnd;
		timeEnd = getTimeMs();
		SAMPLE_PRINT("[INFO] == FORWORD TIME SPEND : %ld ms== \n", timeEnd - timeStart);
		SAMPLE_PRINT("[INFO] %s-%d-done\n", __FUNCTION__, __LINE__);
#endif

		return HI_SUCCESS;
	}




}
