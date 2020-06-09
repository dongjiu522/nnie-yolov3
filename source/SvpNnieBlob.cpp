#ifdef _MSC_VER
#include<io.h>
#else
#include <linux/limits.h>
#include <unistd.h>
#endif

#include<iostream>
#include"SvpNnieCommon.h"
#include"SvpNnieBlob.h"

namespace nnie
{
	HI_S32 SAMPLE_SVP_BlobAlloc(SVP_BLOB_S* pstBlob, int u32BlobSize, HI_BOOL bCached)
	{
		HI_S32 s32Ret = HI_SUCCESS;
		SAMPLE_CHECK_RETURN(NULL == pstBlob, HI_FAILURE, "[ERROR]param  SVP_BLOB_S == NULL!\n");
		SAMPLE_CHECK_RETURN(u32BlobSize <=0 , HI_FAILURE, "[ERROR]param  u32BlobSize <=0!\n");
		SVP_MEM_INFO_S stMem;
		memset(&stMem, 0, sizeof(stMem));
		stMem.u32Size = u32BlobSize;
		s32Ret = SAMPLE_SVP_AllocMem(u32BlobSize, &stMem, bCached);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, HI_FAILURE, "[ERROR]SAMPLE_SVP_AllocMem failed!\n");
		pstBlob->u64PhyAddr = stMem.u64PhyAddr;
		pstBlob->u64VirAddr = stMem.u64VirAddr;

		if (HI_FALSE == bCached)
		{
			return s32Ret;
		}
		memset((HI_VOID*)((HI_UL)pstBlob->u64VirAddr), 0, u32BlobSize);

		s32Ret = SAMPLE_SVP_FlushCache(&stMem);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FAIL_0, "[ERROR]SAMPLE_SVP_FlushCache failed!\n");
		return s32Ret;
	FAIL_0:
		(HI_VOID)SAMPLE_SVP_FreeMem(&stMem);
		return s32Ret;
	}

	HI_S32 SAMPLE_SVP_BlobFree(SVP_BLOB_S* pstBlob)
	{
		HI_S32 s32Ret = HI_SUCCESS;
		SAMPLE_CHECK_RETURN(NULL == pstBlob, HI_FAILURE, "[ERROR]param  SVP_BLOB_S == NULL!\n");
		SVP_MEM_INFO_S stMem;
		memset(&stMem, 0, sizeof(stMem));
		stMem.u64PhyAddr = pstBlob->u64PhyAddr;
		stMem.u64VirAddr = pstBlob->u64VirAddr;
		stMem.u32Size = SAMPLE_SVP_BlobGetSize(pstBlob);

		s32Ret = SAMPLE_SVP_FreeMem(&stMem);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, HI_FAILURE, "SAMPLE_FreeMem failed!\n");
		return s32Ret;
	}
	HI_S32 SAMPLE_SVP_BlobGetStride(SVP_BLOB_TYPE_E type, HI_U32 width, HI_U32 align, HI_U32* pStride)
	{
		HI_S32 s32Ret = HI_SUCCESS;
		HI_U32 u32Size = 0;
		SAMPLE_CHECK_RETURN(width <=0, HI_FAILURE, "[ERROR]param  width <= 0\n");

		if (SVP_BLOB_TYPE_S32 == type || SVP_BLOB_TYPE_VEC_S32 == type || SVP_BLOB_TYPE_SEQ_S32 == type)
		{
			u32Size = sizeof(HI_U32);
		}
		else
		{
			u32Size = sizeof(HI_U8);
		}

#if 0

		if (SVP_BLOB_TYPE_SEQ_S32 == type)
		{
			if (ALIGN_16 == align)
			{
				*pStride = ALIGN16(u32Dim * u32Size);
			}
			else
			{
				*pStride = ALIGN32(u32Dim * u32Size);
			}

			total = step * *pStride;
		}
		else
#endif
		{
			if (ALIGN_16 == align)
			{
				*pStride = ALIGN16(width * u32Size);
			}
			else
			{
				*pStride = ALIGN32(width * u32Size);
			}
		}

		return s32Ret;
	}

	HI_S32 SAMPLE_SVP_BlobGetSize(SVP_BLOB_S* blob)
	{
		SAMPLE_CHECK_RETURN(NULL == blob, HI_FAILURE, "[ERROR]param  SVP_BLOB_S == NULL!\n");
		int num = blob->u32Num;
		int stride = blob->u32Stride;
		int height = blob->unShape.stWhc.u32Height;
		int chn = blob->unShape.stWhc.u32Chn;
		return num * stride * height * chn;
	}

	HI_S32 SAMPLE_SVP_BlobGetElementSize(const SVP_BLOB_S *blob)
	{

		SVP_BLOB_TYPE_E type = blob->enType;
		HI_U32 width = blob->unShape.stWhc.u32Width;
		HI_U32 Stride = blob->u32Stride;
		HI_U32  pStride_ALIGN16_U8;
		HI_U32  pStride_ALIGN32_U8;


		HI_U32  pStride_ALIGN16_U32;
		HI_U32  pStride_ALIGN32_U32;

		HI_U32 u32Size_U8 = 0;
		HI_U32 u32Size_U32 = 0;

		if (SVP_BLOB_TYPE_S32 == type || SVP_BLOB_TYPE_VEC_S32 == type || SVP_BLOB_TYPE_SEQ_S32 == type)
		{
			u32Size_U32 = sizeof(HI_U32);
		}
		else
		{
			u32Size_U8 = sizeof(HI_U8);
		}
		pStride_ALIGN16_U8 = ALIGN16(width * u32Size_U8);
		pStride_ALIGN32_U8 = ALIGN32(width * u32Size_U8);

		pStride_ALIGN16_U32 = ALIGN16(width * u32Size_U32);
		pStride_ALIGN32_U32 = ALIGN32(width * u32Size_U32);

		if (Stride == pStride_ALIGN16_U8 || Stride == pStride_ALIGN32_U8)
		{
			return sizeof(HI_U8);
		}
		else if (Stride == pStride_ALIGN16_U32 || Stride == pStride_ALIGN32_U32)
		{
			return sizeof(HI_U32);
		}
		else
		{
			return 0;
		}

	}

	HI_S32 SAMPLE_RUNTIME_LoadModelFile(const char* pcModelFile, SVP_MEM_INFO_S* pstMemInfo)
	{
		FILE* fp = HI_NULL;
		HI_S32  s32RuntimeWkLen = 0;
		HI_S32 s32Ret = HI_FAILURE;
		HI_CHAR acCanonicalPath[SVP_SAMPLE_MAX_PATH + 1] = { 0 };

		SAMPLE_CHECK_RETURN(!(ACCESS(pcModelFile, 0) == 0), HI_FAILURE, (std::string(pcModelFile) + "is not access").c_str());
		fp = fopen(pcModelFile, "rb");
		if (fp == NULL)
		{
			return HI_FAILURE;
		}

		s32Ret = fseek(fp, 0L, SEEK_END);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, CLOSE_FILE, "fseek failed!\n");

		s32RuntimeWkLen = ftell(fp);
		SAMPLE_CHECK_GOTO(0 != s32RuntimeWkLen % 16, CLOSE_FILE, "Runtime WK Len %% 16 != 0 \n");

		SAMPLE_PRINT("Runtime WK Len: %d\n", s32RuntimeWkLen);

		SAMPLE_CHECK_GOTO(0 != fseek(fp, 0L, SEEK_SET), CLOSE_FILE, "fseek fail");

		pstMemInfo->u32Size = s32RuntimeWkLen;
		s32Ret = SAMPLE_SVP_AllocMem(s32RuntimeWkLen, pstMemInfo, HI_FALSE);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, CLOSE_FILE, "SAMPLE_RUNTIME_MemAlloc failed!\n");

		s32Ret = (HI_S32)fread((HI_VOID*)((HI_UL)pstMemInfo->u64VirAddr), s32RuntimeWkLen, 1, fp);
		SAMPLE_CHECK_GOTO(1 != s32Ret, FREE_MEM, "Read WK failed!\n");

		fclose(fp);
		return HI_SUCCESS;
	FREE_MEM:
		SAMPLE_SVP_FreeMem(pstMemInfo);
	CLOSE_FILE:
		fclose(fp);
		return HI_FAILURE;
	}


	HI_S32 SAMPLE_SVP_BlobSetAndAlloc(SVP_BLOB_S* pstBlob,SVP_BLOB_TYPE_E enType,int u32Num,int u32Width,int u32Height,int u32Chn,int u32Align)
	{
		HI_S32 s32Ret = HI_SUCCESS;
		HI_U32 u32BlobSize = 0;
		memset(pstBlob, 0, sizeof(*pstBlob));
		pstBlob->enType = enType;
		pstBlob->u32Num = u32Num;
		//if (enType == SVP_BLOB_TYPE_VEC_S32)
		//{
		//    pstBlob->unShape.stWhc.u32Width = u32Chn;
		//    pstBlob->unShape.stWhc.u32Chn = u32Width;
		//}
		//else
		//{
		pstBlob->unShape.stWhc.u32Width = u32Width;
		pstBlob->unShape.stWhc.u32Chn = u32Chn;
		//}
		pstBlob->unShape.stWhc.u32Height = u32Height;
		SAMPLE_SVP_BlobGetStride(enType, pstBlob->unShape.stWhc.u32Width, u32Align, &(pstBlob->u32Stride));

		SAMPLE_CHECK_GOTO(((HI_U64)pstBlob->u32Num * pstBlob->u32Stride * pstBlob->unShape.stWhc.u32Height * pstBlob->unShape.stWhc.u32Chn > (HI_U32)-1), FAIL_0, "the blobsize is too large [%llu]\n", (HI_U64)pstBlob->u32Num * pstBlob->u32Stride * pstBlob->unShape.stWhc.u32Height * pstBlob->unShape.stWhc.u32Chn);
		u32BlobSize = SAMPLE_SVP_BlobGetSize(pstBlob);

		s32Ret = SAMPLE_SVP_BlobAlloc(pstBlob, u32BlobSize, HI_TRUE);
		SAMPLE_CHECK_GOTO(HI_SUCCESS != s32Ret, FAIL_0, "[ERROR]SAMPLE_SVP_BlobAlloc is failed\n");

		return HI_SUCCESS;
	FAIL_0:
		return HI_FAILURE;
	}


}