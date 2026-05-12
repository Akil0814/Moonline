#pragma once
#include "../base/singleton.h"

class FileManager : public Singleton<FileManager>
{
	friend Singleton<FileManager>;
public:
	bool init();
	bool config_preload();
private:
	bool _find_all_folder = false;
};
