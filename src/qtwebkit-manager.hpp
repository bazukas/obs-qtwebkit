/*
Copyright (C) 2015 by Azat Khasanshin <akhasanshin3@gatech.edu>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <obs-module.h>

struct shared_data {
	pthread_mutex_t mutex;
	uint8_t data;
};

class QtWebkitManager {
public:
	QtWebkitManager(char *url, uint32_t width, uint32_t height, uint32_t fps, char *css);
	~QtWebkitManager();
	uint8_t *GetData() { return &data->data; }
	void Lock();
	void UnLock();
	void SetUrl(char *url, char *css);
private:
	void KillRenderer();
	void SpawnRenderer(char *url, char *css);
private:
	int fd = -1;
	int pid = 0;
	int uid = 0;

	uint32_t width, height, fps;

	struct shared_data *data = NULL;
};
