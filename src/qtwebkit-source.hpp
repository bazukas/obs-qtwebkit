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

#include "qtwebkit-manager.hpp"

class QtWebkitSource {
public:
	QtWebkitSource(obs_source_t *source);
	~QtWebkitSource();
public:
	uint32_t GetWidth() const { return width; }
	uint32_t GetHeight() const { return height; }
	uint32_t GetFps() const { return fps; }
	void UpdateSettings(obs_data_t *settings);
	void RenderTexture(gs_effect_t *effect);
	void PrepareTexture();
	void LockTexture();
	void UnlockTexture();
	void Reload();
private:
	void UpdatePage(bool resize, bool fps_change);

public:
	obs_hotkey_id reload_key;
private:
	bool isLocalFile;
	char *url;
	uint32_t width;
	uint32_t height;
	uint32_t fps;
	char *css;

	obs_source_t *source;

	gs_texture_t *activeTexture;
	pthread_mutex_t textureLock;

	QtWebkitManager *manager;
};
