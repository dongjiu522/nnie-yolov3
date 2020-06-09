#ifndef __SVP_NNIE_MEM_H
#define __SVP_NNIE_MEM_H
#include<stdint.h>
#include"hi_comm_svp.h"

namespace nnie
{


	 HI_S32   SAMPLE_SVP_AllocMem(uint32_t size, SVP_MEM_INFO_S * mem, bool bCached);
	 HI_S32   SAMPLE_SVP_FlushCache(SVP_MEM_INFO_S *pstMemInfo);
	 HI_S32   SAMPLE_SVP_FreeMem(SVP_MEM_INFO_S *pstMemInfo);

}

#endif
