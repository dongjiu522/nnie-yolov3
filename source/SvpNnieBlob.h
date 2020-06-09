#ifndef  __SVP_NNIE_BLOB_H
#define  __SVP_NNIE_BLOB_H

#include"SvpNnieMem.h"

namespace nnie
{
		 HI_S32 SAMPLE_SVP_BlobAlloc(SVP_BLOB_S* pstBlob, int u32BlobSize, HI_BOOL bCached);
		 HI_S32 SAMPLE_SVP_BlobFree(SVP_BLOB_S* pstBlob);
		 HI_S32 SAMPLE_SVP_BlobGetStride(SVP_BLOB_TYPE_E type, HI_U32 width, HI_U32 align, HI_U32* pStride);
		 HI_S32 SAMPLE_SVP_BlobGetSize(SVP_BLOB_S* blob);
		 HI_S32 SAMPLE_SVP_BlobGetElementSize(const SVP_BLOB_S *blob);
		 HI_S32 SAMPLE_SVP_BlobSetAndAlloc(SVP_BLOB_S* pstBlob, SVP_BLOB_TYPE_E enType, int u32Num, int u32Width, int u32Height, int u32Chn, int u32Align);
		 HI_S32 SAMPLE_RUNTIME_LoadModelFile(const char* pcModelFile, SVP_MEM_INFO_S* pstMemInfo);
		 
}

























#endif // ! __SAMPLE_COMMON_ALL_H

