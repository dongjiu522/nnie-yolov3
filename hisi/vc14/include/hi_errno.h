/******************************************************************************
Copyright (C), 2001-2015, Hisilicon Tech. Co., Ltd.
******************************************************************************
File Name     : hi_errno.h
Version       : Initial Draft
Author        : Hi3511 MPP Team
Created       : 2006/11/09
Last Modified :
Description   : define the format of error code
Function List :
History       :
 1.Date        : 2007/02/14
   Author      :
   Modification: Created file

 2.Date        : 2007/12/11
   Author      :
   Modification: add some MOD_ID for several modules

 3.Date        : 2008/02/03
   Author      :
   Modification: reoorder MOD_ID to cleanup modules at specified order

 4.Date        : 2008/03/01
   Author      :
   Modification: move LOG_ERRLEVEL_E to hi_debug.h, and add new definion
   ERR_LEVLE_E, we should use this enumeration in our error code.

 5.Date        : 2008/04/30
   Author      :
   Modification: delete two error code "EN_ERR_NOCHN" and "EN_ERR_NODEV".

******************************************************************************/
#ifndef __HI_ERRNO_H__
#define __HI_ERRNO_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* 1010 0000b
 * VTOP use APPID from 0~31
 * so, hisilicon use APPID based on 32
 */
#define HI_ERR_APPID  (0x80000000L + 0x20000000L)
typedef enum hiERR_LEVEL_E
{
    EN_ERR_LEVEL_DEBUG = 0,  /* debug-level                                  */
    EN_ERR_LEVEL_INFO,       /* informational                                */
    EN_ERR_LEVEL_NOTICE,     /* normal but significant condition             */
    EN_ERR_LEVEL_WARNING,    /* warning conditions                           */
    EN_ERR_LEVEL_ERROR,      /* error conditions                             */
    EN_ERR_LEVEL_CRIT,       /* critical conditions                          */
    EN_ERR_LEVEL_ALERT,      /* action must be taken immediately             */
    EN_ERR_LEVEL_FATAL,      /* just for compatibility with previous version */
    EN_ERR_LEVEL_BUTT
}ERR_LEVEL_E;

/******************************************************************************
|----------------------------------------------------------------|
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

#define HI_DEF_ERR( module, level, errid) \
    ((HI_S32)( (HI_ERR_APPID) | ((module) << 16 ) | ((level)<<13) | (errid) ))

/* NOTE! the following defined all common error code,
** all module must reserved 0~63 for their common error code
*/
typedef enum hiEN_ERR_CODE_E
{
    EN_ERR_INVALID_DEVID = 1, /* invlalid device ID                           */
    EN_ERR_INVALID_CHNID = 2, /* invlalid channel ID                          */
    EN_ERR_ILLEGAL_PARAM = 3, /* at lease one parameter is illagal
                               * eg, an illegal enumeration value             */
    EN_ERR_EXIST         = 4, /* resource exists                              */
    EN_ERR_UNEXIST       = 5, /* resource unexists                            */

    EN_ERR_NULL_PTR      = 6, /* using a NULL point                           */

    EN_ERR_NOT_CONFIG    = 7, /* try to enable or initialize system, device
                              ** or channel, before configing attribute       */

    EN_ERR_NOT_SUPPORT   = 8, /* operation or type is not supported by NOW    */
    EN_ERR_NOT_PERM      = 9, /* operation is not permitted
                              ** eg, try to change static attribute           */

    EN_ERR_NOMEM         = 12,/* failure caused by malloc memory              */
    EN_ERR_NOBUF         = 13,/* failure caused by malloc buffer              */

    EN_ERR_BUF_EMPTY     = 14,/* no data in buffer                            */
    EN_ERR_BUF_FULL      = 15,/* no buffer for new data                       */

    EN_ERR_SYS_NOTREADY  = 16,/* System is not ready,maybe not initialed or
                              ** loaded. Returning the error code when opening
                              ** a device file failed.                        */

    EN_ERR_BADADDR       = 17,/* bad address,
                              ** eg. used for copy_from_user & copy_to_user   */

    EN_ERR_BUSY          = 18,/* resource is busy,
                              ** eg. destroy a venc chn without unregister it */

    EN_ERR_BUTT          = 63,/* maxium code, private error code of all modules
                              ** must be greater than it                      */
}EN_ERR_CODE_E;

/* copy from hi_common.h of himpp/cbb/include*/
typedef enum hiMOD_ID_E
{
    HI_ID_CMPI    = 0,
    HI_ID_VB      = 1,
    HI_ID_SYS     = 2,
    HI_ID_RGN     = 3,
    HI_ID_CHNL    = 4,
    HI_ID_VDEC    = 5,
    HI_ID_AVS     = 6,
    HI_ID_VPSS    = 7,
    HI_ID_VENC    = 8,
    HI_ID_SVP     = 9,
    HI_ID_H264E   = 10,
    HI_ID_JPEGE   = 11,
    HI_ID_MPEG4E  = 12,
    HI_ID_H265E   = 13,
    HI_ID_JPEGD   = 14,
    HI_ID_VO      = 15,
    HI_ID_VI      = 16,
    HI_ID_DIS     = 17,
    HI_ID_VALG    = 18,
    HI_ID_RC      = 19,
    HI_ID_AIO     = 20,
    HI_ID_AI      = 21,
    HI_ID_AO      = 22,
    HI_ID_AENC    = 23,
    HI_ID_ADEC    = 24,
    HI_ID_VPU    = 25,
    HI_ID_PCIV    = 26,
    HI_ID_PCIVFMW = 27,
    HI_ID_ISP     = 28,
    HI_ID_IVE     = 29,
    HI_ID_USER    = 30,
    HI_ID_DCCM    = 31,
    HI_ID_DCCS    = 32,
    HI_ID_PROC    = 33,
    HI_ID_LOG     = 34,
    HI_ID_VFMW    = 35,
    HI_ID_H264D   = 36,
    HI_ID_GDC     = 37,
    HI_ID_PHOTO   = 38,
    HI_ID_FB      = 39,
    HI_ID_HDMI    = 40,
    HI_ID_VOIE    = 41,
    HI_ID_TDE     = 42,
    HI_ID_HDR     = 43,
    HI_ID_PRORES  = 44,
    HI_ID_VGS     = 45,

    HI_ID_FD      = 47,
    HI_ID_ODT     = 48, //Object detection trace
    HI_ID_VQA     = 49, //Video quality  analysis
    HI_ID_LPR     = 50, //Object detection trace
    HI_ID_SVP_NNIE     = 51,
    HI_ID_SVP_DSP      = 52,
    HI_ID_DPU_RECT     = 53,
    HI_ID_DPU_MATCH    = 54,
    HI_ID_BUTT,
} MOD_ID_E;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_ERRNO_H__ */



