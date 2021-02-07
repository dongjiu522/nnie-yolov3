#include <stdlib.h>

#ifdef ON_BOARD
#include "mpi_sys.h"
#endif
#include"SvpNnieCommon.h"
#include"SvpNnieMem.h"
namespace nnie
{

	HI_S32 SAMPLE_SVP_AllocMem(uint32_t size, SVP_MEM_INFO_S * mem, bool bCached)
	{
		HI_S32 s32Ret = HI_SUCCESS;
		SAMPLE_CHECK_RETURN(NULL == mem, HI_FAILURE, "[ERROR]param SVP_MEM_INFO_S == NULL\n");
		SAMPLE_CHECK_RETURN(size <= 0, HI_FAILURE, "[ERROR]param size < 0\n");
		mem->u32Size = size;
#ifdef ON_BOARD

		if (bCached)
		{
			s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&mem->u64PhyAddr, (HI_VOID**)&(mem->u64VirAddr), NULL, HI_NULL, mem->u32Size);
		}
		else
		{
			s32Ret = HI_MPI_SYS_MmzAlloc(&mem->u64PhyAddr, (HI_VOID**)&(mem->u64VirAddr), NULL, HI_NULL, mem->u32Size);
		}
#else
		mem->u64VirAddr = (HI_U64)((HI_UL)malloc(mem->u32Size));
		mem->u64PhyAddr = mem->u64VirAddr;
		if (mem->u64VirAddr == NULL)
		{
			s32Ret = HI_FAILURE;
		}
#endif

		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, HI_FAILURE, "[ERROR]SAMPLE_SVP_AllocMem is failed\n");
		return s32Ret;
	}
	HI_S32 SAMPLE_SVP_FlushCache(SVP_MEM_INFO_S *pstMemInfo)
	{
		HI_S32 s32Ret = HI_SUCCESS;
		SAMPLE_CHECK_RETURN(NULL == pstMemInfo, HI_FAILURE, "[ERROR]param SVP_MEM_INFO_S == NULL\n");
		SAMPLE_CHECK_RETURN(NULL == pstMemInfo->u64VirAddr, HI_FAILURE, "[ERROR]param SVP_MEM_INFO_S's u64VirAddr == NULL\n");
		SAMPLE_CHECK_RETURN(NULL == pstMemInfo->u64PhyAddr, HI_FAILURE, "[ERROR]param SVP_MEM_INFO_S's u64PhyAddr == NULL\n");

#ifdef ON_BOARD
		s32Ret = HI_MPI_SYS_MmzFlushCache(pstMemInfo->u64PhyAddr, (HI_VOID*)((HI_UL)pstMemInfo->u64VirAddr), pstMemInfo->u32Size);
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, HI_FAILURE, "[ERROR]HI_MPI_SYS_MmzFlushCache is failed\n");
#endif
		return s32Ret;

	}
	HI_S32 SAMPLE_SVP_FreeMem(SVP_MEM_INFO_S *pstMemInfo)
	{
		HI_S32 s32Ret = HI_SUCCESS;
		SAMPLE_CHECK_RETURN(NULL == pstMemInfo, HI_FAILURE, "[ERROR]param SVP_MEM_INFO_S == NULL\n");
		SAMPLE_CHECK_RETURN(NULL == pstMemInfo->u64VirAddr, HI_FAILURE, "[ERROR]param SVP_MEM_INFO_S's u64VirAddr == NULL\n");
		SAMPLE_CHECK_RETURN(NULL == pstMemInfo->u64PhyAddr, HI_FAILURE, "[ERROR]param SVP_MEM_INFO_S's u64PhyAddr == NULL\n");

#ifdef ON_BOARD
		s32Ret = HI_MPI_SYS_MmzFree(pstMemInfo->u64PhyAddr, (HI_VOID*)((HI_UL)pstMemInfo->u64VirAddr));
		SAMPLE_CHECK_RETURN(HI_SUCCESS != s32Ret, HI_FAILURE, "[ERROR]HI_MPI_SYS_MmzFree is failed\n");
#else
		free((HI_U8*)((HI_UL)pstMemInfo->u64VirAddr));
		pstMemInfo->u64PhyAddr = 0;
		pstMemInfo->u64VirAddr = 0;
#endif
		return s32Ret;
	}


}