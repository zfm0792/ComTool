#pragma once
#define COMMON_NAME			"ComTool"
#define COMMON_VERSION		"1.0"

#ifdef _DEBUG 
#define COMMON_NAME_AND_VERSION COMMON_NAME " " COMMON_VERSION " - Debug Mode"
#else
#define COMMON_NAME_AND_VERSION COMMON_NAME " " COMMON_VERSION " "
#endif