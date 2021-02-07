#include"common.h"

#ifdef _MSC_VER
#include <Windows.h>
#else

#endif


namespace nnie
{
	FILE_NAME_PAIR getFullPathFileNameFromFullPath(std::string fileFullPath)
	{
		std::string strFileNameFull = fileFullPath;
		std::string strBasic;
		std::string strSuffix;
		FILE_NAME_PAIR namePair;

		size_t suffixPos = strFileNameFull.find_last_of(".");
		if (suffixPos != std::string::npos) {
			strSuffix = strFileNameFull.substr(suffixPos + 1);
		}

		strBasic = strFileNameFull.substr(0, suffixPos);

		namePair.first = strBasic;
		namePair.second = strSuffix;
		return namePair;
	}

	FILE_NAME_PAIR getFileNameFromFullPath(std::string fileFullPath)
	{
		std::string strFileNameFull;
		std::string strBasic;
		std::string strSuffix;
		FILE_NAME_PAIR namePair;

		size_t fileNamePos = fileFullPath.find_last_of("/");
		/*  "/" not found and set pos to 0 */
		if (fileNamePos == std::string::npos) {
			fileNamePos = 0;
			strFileNameFull = fileFullPath;
		}
		else
		{
			strFileNameFull = fileFullPath.substr(fileNamePos + 1);
		}

		size_t suffixPos = strFileNameFull.find_last_of(".");
		if (suffixPos != std::string::npos) {
			strSuffix = strFileNameFull.substr(suffixPos + 1);
		}

		strBasic = strFileNameFull.substr(0, suffixPos);

		namePair.first = strBasic;
		namePair.second = strSuffix;
		return namePair;
	}



	float  getTimeMs()
	{
#ifdef _MSC_VER
		LARGE_INTEGER time, freq;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&time);
		uint64_t sec = time.QuadPart / freq.QuadPart;
		uint64_t usec = (time.QuadPart % freq.QuadPart) * 1000000 / freq.QuadPart;
		return sec * 1000 + usec / 1000;
#else
		float curTime;
		struct timespec tmpTime;
		clock_gettime(CLOCK_MONOTONIC, &tmpTime);
		curTime = tmpTime.tv_sec * 1000 + tmpTime.tv_nsec / (1000 * 1000);
		return curTime;
#endif

	}
}