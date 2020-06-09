#define FUNC
//#define INST

#ifdef _MSC_VER
#include<Windows.h>
#if DEBUG
#ifdef FUNC
#pragma comment(lib, "nniefc1.2d.lib")
#endif
#ifdef INST
#pragma comment(lib, "nnieit1.2d.lib")
#endif
#pragma comment(lib, "opencv_world341d.lib")
#endif
#endif

#include<string>
#include<vector>
#include<iostream>
#include"opencv2/opencv.hpp"
#include"SvpNnieYolov3.h"
#include"common.h"
int main()
{
	std::string imagePath = "data/dog_bike_car.jpg";
	std::string wkPath;
#ifdef FUNC
	wkPath = "wk/yolov3/inst_yolov3_func.wk";
#else
	wkPath = "wk/yolov3/inst_yolov3_inst.wk";
#endif

	//std::string yolov3_wk_path = wkPath;
	std::cout << "########################################" << std::endl;
	std::string net = "yolov3";
	std::cout << net << ": began" << std::endl;
	std::cout << "[INFO]wkPath: " << wkPath << std::endl;
	nnie::NetYoloV3 yolov3((const char*)wkPath.c_str());
	cv::Mat src = cv::imread(imagePath);
	if (src.empty())
	{
		std::cout << "[ERROR]" << imagePath << "is not read\n" << std::endl;
		return -1;
	}
	nnie::FILE_NAME_PAIR file_name_pair = nnie::getFullPathFileNameFromFullPath(imagePath);
	std::string out_image_path = file_name_pair.first + "_" + net + ".jpg";
	float width = src.cols;
	float height = src.rows;
	float diff = std::abs(width - height);
	cv::Mat src_copyMakeBorder;
	if (diff)
	{
		if (width > height)
		{
			cv::copyMakeBorder(src, src_copyMakeBorder, 0, diff, 0, 0, cv::BORDER_CONSTANT, 0);
		}
		else
		{
			cv::copyMakeBorder(src, src_copyMakeBorder, 0, 0, 0, diff, cv::BORDER_CONSTANT, 0);
		}

	}

	float w = 416;
	float h = 416;
	cv::Mat src_copyMakeBorder_resize;
	cv::resize(src_copyMakeBorder, src_copyMakeBorder_resize, cv::Size(w, h));

	float width_crop = src_copyMakeBorder_resize.cols;
	float height_crop = src_copyMakeBorder_resize.rows;
	cv::Mat image;


	float mScaleW = width_crop / w;
	float mScaleH = height_crop / h;
	float threshold = 0.5;
	std::vector<nnie::TargetBox>ObjBoxs;
	yolov3.detect(src_copyMakeBorder_resize.data, 1, 3, height_crop, width_crop, width_crop * 3, threshold,ObjBoxs);
	float fFontSize = 0.5f;
	cv::Scalar fontColor(0, 0, 255);
	cv::Scalar lineColor(255, 0, 0);

	for (nnie::TargetBox stBox : ObjBoxs)
	{
		nnie::Rect stRect = stBox.stRect;

		stRect.x = stRect.x * mScaleW;
		stRect.w = stRect.w * mScaleW;

		stRect.y = stRect.y * mScaleH;
		stRect.h = stRect.h * mScaleH;

		if (stBox.u32Class < 0 || stBox.u32Class > nnie::Yolov3ClassName.size())
		{
			cv::putText(src_copyMakeBorder_resize, std::to_string(stBox.u32Class), cv::Point(stRect.x + 5, stRect.y + 12), cv::FONT_HERSHEY_SIMPLEX, fFontSize, fontColor, 1, 8);
		}
		else
		{
			std::string name = nnie::Yolov3ClassName[stBox.u32Class];
			std::cout << name << " :  " << stBox.fScore << std::endl;
			cv::putText(src_copyMakeBorder_resize, name, cv::Point(stRect.x + 5, stRect.y + 12), cv::FONT_HERSHEY_SIMPLEX, fFontSize, fontColor, 1, 8);
		}

		cv::rectangle(src_copyMakeBorder_resize, { (HI_S32)(stRect.x), (HI_S32)(stRect.y) }, { (HI_S32)(stRect.x + stRect.w), (HI_S32)(stRect.y + stRect.h) }, lineColor, 2, 1, 0);
		cv::putText(src_copyMakeBorder_resize, std::to_string(stBox.fScore), cv::Point(stRect.x + 5, stRect.y + 27), cv::FONT_HERSHEY_SIMPLEX, fFontSize, fontColor, 1, 8);
	}

	cv::imwrite(out_image_path, src_copyMakeBorder_resize);

#ifdef _MSC_VER
	system("pause");
#endif
	return 0;
}