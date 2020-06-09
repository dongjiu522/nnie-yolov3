#ifndef __SVP_NNIE_ONE_SEG_NET_H_
#define __SVP_NNIE_ONE_SEG_NET_H_
#include<stdint.h>
#include<string>
#include<vector>
#include"hi_comm_svp.h"
#include"hi_nnie.h"
#include"mpi_nnie.h"
#include"SvpNnieCommon.h"
#include"SvpNnieBlob.h"


namespace nnie
{
	typedef struct hiSVP_NNIE_ONE_SEG_DET_S
	{
		SVP_NNIE_MODEL_S model;

		SVP_MEM_INFO_S  modelBuffer;
		SVP_MEM_INFO_S  tmpBuffer;
		SVP_MEM_INFO_S  taskBuffer;

		HI_U32 modelBufferSize;
		HI_U32 tmpBufferSize;
		HI_U32 taskBufferSize;
	
		SVP_SRC_BLOB_S srcBlobs[SVP_NNIE_MAX_INPUT_NUM];
		SVP_DST_BLOB_S dstBlobs[SVP_NNIE_MAX_OUTPUT_NUM];

		SVP_NNIE_FORWARD_CTRL_S nnieCtrl;
	}SVP_NNIE_ONE_SEG_S;

	class NetOneSeg
	{
	public:
		NetOneSeg(const char * wkfile, HI_U32 maxBatch);
		~NetOneSeg();
	public:
		HI_S32  setBlob(const HI_U8 * bgr, HI_U32 n, HI_U32 c, HI_U32 h, HI_U32 w, HI_U32 stride, SVP_BLOB_S * srcBlob);
		HI_S32  forword();

	private:
		HI_S32 init(const char * wkfile, HI_U32 maxBatch  =1);
		HI_S32 destory();

	private:
		SVP_NNIE_HANDLE		NNIEHandle;
		SVP_NNIE_ID_E		NNIEDevId = SVP_NNIE_ID_0;
	public:
		SVP_NNIE_ONE_SEG_S  NNIEOneSegNetParam;
		std::vector<std::pair<std::string, SVP_BLOB_S>>	srcBlobs;
		std::vector<std::pair<std::string, SVP_BLOB_S>>	dstBlobs;
	};




}

#endif
