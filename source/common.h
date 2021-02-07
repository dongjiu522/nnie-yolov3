#ifndef __COMMON_H_
#define __COMMON_H_

#include<stdint.h>
#include<string>


namespace nnie
{
	typedef std::pair<std::string, std::string> FILE_NAME_PAIR;

	FILE_NAME_PAIR getFileNameFromFullPath(std::string fileFullPath);

	FILE_NAME_PAIR getFullPathFileNameFromFullPath(std::string fileFullPath);

	float  getTimeMs();




}



#endif
