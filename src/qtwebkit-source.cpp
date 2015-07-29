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

#include <pthread.h>

#include "qtwebkit-source.hpp"

#define blog(level, msg, ...) blog(level, "qtwebkit-source: " msg, ##__VA_ARGS__)

QtWebkitSource::QtWebkitSource(obs_source_t *s)
{
	source = s;
	activeTexture = nullptr;
	manager = nullptr;
	pthread_mutex_init(&textureLock, nullptr);
}

QtWebkitSource::~QtWebkitSource()
{
	LockTexture();
	if (activeTexture) {
		obs_enter_graphics();
		gs_texture_destroy(activeTexture);
		activeTexture = nullptr;
		obs_leave_graphics();
	}
	UnlockTexture();
	pthread_mutex_destroy(&textureLock);

	if (manager)
		delete manager;
}

void QtWebkitSource::LockTexture()
{
	pthread_mutex_lock(&textureLock);
}

void QtWebkitSource::UnlockTexture()
{
	pthread_mutex_unlock(&textureLock);
}

void QtWebkitSource::UpdatePage(bool resize, bool fps_change)
{
	LockTexture();
	obs_enter_graphics();
	if (resize || !activeTexture) {
		if (activeTexture)
			gs_texture_destroy(activeTexture);
		activeTexture = gs_texture_create(width, height, GS_RGBA, 1, nullptr, GS_DYNAMIC);
	}
	obs_leave_graphics();
	UnlockTexture();
	if (!manager || resize || fps_change) {
		if (manager)
			delete manager;
		manager = new QtWebkitManager(url, width, height, fps, css);
	} else {
		manager->SetUrl(url, css);
	}
}

void QtWebkitSource::UpdateSettings(obs_data_t *settings)
{
	uint32_t old_width = width;
	uint32_t old_height = height;
	uint32_t old_fps = fps;
	isLocalFile = obs_data_get_bool(settings, "is_local_file");
	url = (char *) obs_data_get_string(settings, isLocalFile ? "local_file" : "url");
	width = (uint32_t) obs_data_get_int(settings, "width");
	height = (uint32_t) obs_data_get_int(settings, "height");
	fps = (uint32_t) obs_data_get_int(settings, "fps");
	css = (char *) obs_data_get_string(settings, "css_file");
	UpdatePage(old_width != width || old_height != height, old_fps != fps);
}

void QtWebkitSource::PrepareTexture()
{
	LockTexture();

	if (!activeTexture || !obs_source_showing(source)) {
		UnlockTexture();
		return;
	}

	manager->Lock();
	obs_enter_graphics();
	gs_texture_set_image(activeTexture, manager->GetData(), width * 4, false);
	obs_leave_graphics();
	manager->UnLock();

	UnlockTexture();
}

void QtWebkitSource::RenderTexture(gs_effect_t *effect)
{
	LockTexture();

	if (!activeTexture) {
		UnlockTexture();
		return;
	}

	gs_reset_blend_state();
	gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), activeTexture);
	gs_draw_sprite(activeTexture, 0, width, height);

	UnlockTexture();
}

void QtWebkitSource::Reload()
{
	if (manager)
		delete manager;
	manager = new QtWebkitManager(url, width, height, fps, css);
}
