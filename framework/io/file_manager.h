#pragma once

class FileManager
{
public:
	bool init();
	bool config_preload();
private:
	bool _find_all_folder = false;
};
