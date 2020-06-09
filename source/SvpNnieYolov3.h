#ifndef __SVP_NNIE_YOLOV3_H_
#define __SVP_NNIE_YOLOV3_H_

#include"SvpNnieOneSegNet.h"

namespace nnie
{

	const std::vector<std::string> Yolov3ClassName =
	{
		"person",
		"bicycle",
		"car",
		"motorcycle",
		"airplane",
		"bus",
		"train",
		"truck",
		"boat",
		"traffic light",
		"fire hydrant",
		"stop sign",
		"parking meter",
		"bench",
		"bird",
		"cat",
		"dog"
	};

	typedef struct
	{
		HI_U32 x;
		HI_U32 y;
		HI_U32 w;
		HI_U32 h;
	}Rect;
	typedef struct
	{
		Rect stRect;
		HI_U32 u32Class;
		HI_FLOAT fScore;
	}TargetBox;

	class NetYoloV3 :public NetOneSeg
	{
	public:
		NetYoloV3(const char * wkfile, HI_U32 maxBatch = 1);
		~NetYoloV3();
	public:
		HI_S32 detect(const HI_U8 * bgr, HI_U32 n, HI_U32 c, HI_U32 h, HI_U32 w, HI_U32 stride, HI_FLOAT threshold, std::vector<TargetBox> &boxs);
	private:
		void init(void);
		void destory(void);
	private:
		void * data = NULL;
	};


}
#endif
