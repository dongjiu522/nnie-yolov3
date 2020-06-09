#ifndef __SVP_NNIE_COMMON_H__
#define __SVP_NNIE_COMMON_H__

#ifdef _MSC_VER
#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#define SVP_SAMPLE_MAX_PATH  _MAX_PATH

#else
#include <linux/limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#define SVP_SAMPLE_MAX_PATH  PATH_MAX
#endif

#ifdef _MSC_VER
#define ACCESS _access
#else
#define ACCESS access
#endif

#define STATIC static
#define VOID void

#define DETECION_DBG (0)
#define STRIDE_ALIGN  (16)
#define HI_PI (3.1415926535897932384626433832795)

#ifndef SVP_WK_PROPOSAL_WIDTH
#define SVP_WK_PROPOSAL_WIDTH (6)
#endif

#ifndef SVP_WK_COORDI_NUM
#define SVP_WK_COORDI_NUM (4)
#endif

#ifndef SVP_WK_SCORE_NUM
#define SVP_WK_SCORE_NUM (2)
#endif

#ifndef SVP_WK_QUANT_BASE
#define SVP_WK_QUANT_BASE  (0x1000)
#endif

#define MAX_STACK_DEPTH (50000)


#define SVP_NNIE_MAX_REPORT_NODE_CNT  (16) /*NNIE max report num*/
#define SVP_NNIE_MAX_RATIO_ANCHOR_NUM (32) /*NNIE max ratio anchor num*/
#define SVP_NNIE_MAX_SCALE_ANCHOR_NUM (32) /*NNIE max scale anchor num*/
#define SVP_NNIE_HALF 0.5f


#define SVP_WK_PROPOSAL_WIDTH (6)
#define SVP_WK_QUANT_BASE (0x1000)
#define SVP_WK_COORDI_NUM (4)
#define MAX_STACK_DEPTH (50000)


#ifndef ALIGN_16
#define ALIGN_16 16
#define ALIGN16(u32Num) ((u32Num + ALIGN_16-1) / ALIGN_16 * ALIGN_16)
#endif
#ifndef ALIGN_32
#define ALIGN_32 32
#define ALIGN32(u32Num) ((u32Num + ALIGN_32-1) / ALIGN_32 * ALIGN_32)
#endif

#ifndef SVP_MAX
#define SVP_MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef SVP_MIN
#define SVP_MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define SVP_SAMPLE_MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define SVP_SAMPLE_MIN(a,b)    (((a) < (b)) ? (a) : (b))

#define SVP_SAMPLE_ALIGN32(addr) ((((addr) + 32 - 1)/32)*32)
#define SVP_SAMPLE_ALIGN16(addr) ((((addr) + 16 - 1)/16)*16)

#define SAFE_ROUND(val) double(((double)(val) > 0)? floor((double)(val)+0.5):ceil((double)(val)-0.5))



#ifndef SAMPLE_PRINT
#define SAMPLE_PRINT printf
#endif


#ifndef SAMPLE_FPRINT
#define SAMPLE_FPRINT fprintf
#endif

#ifndef SAMPLE_FUN_LINE_PRINT()
#define SAMPLE_FUN_LINE_PRINT() SAMPLE_PRINT("[INFO]%s-%s-%d\n",__FILE__ ,__FUNCTION__, __LINE__)
#endif



#define SAMPLE_CHECK_EQ_RETURN(a,b,ret,...) \
		if((a) != (b))\
		{	\
			SAMPLE_FUN_LINE_PRINT();\
			SAMPLE_PRINT(__VA_ARGS__);\
			return ret;\
		}

#define SAMPLE_CHECK_RETURN(val, ret, ...) \
        if ((val)) \
        { \
			SAMPLE_FUN_LINE_PRINT();\
            SAMPLE_PRINT(__VA_ARGS__); \
			return ret; \
        }

#define SAMPLE_CHECK_RETURN_VOID(val, ret, ...) \
        if ((val)) \
        { \
			SAMPLE_FUN_LINE_PRINT();\
            SAMPLE_PRINT(__VA_ARGS__); \
			return ; \
        }

#define SAMPLE_CHECK_GOTO(val, ret, ...) \
        if ((val)) \
        { \
			SAMPLE_FUN_LINE_PRINT();\
            SAMPLE_PRINT(__VA_ARGS__); \
			goto ret; \
        }






#endif //__HI_SVP_SAMPLE_COM_H__
