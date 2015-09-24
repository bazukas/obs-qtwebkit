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

#include <unistd.h>
#include <sys/syscall.h>

#include <obs-module.h>
#include "qtwebkit-source.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("qtwebkit-source", "en-US")

static const char *qtwebkit_get_name(void*)
{
	return obs_module_text("QtWebKitBrowser");
}

static void reload_hotkey_pressed(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed)
{
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(key);
	UNUSED_PARAMETER(pressed);

	QtWebkitSource *ws = static_cast<QtWebkitSource *>(data);
	ws->Reload();
}

static bool is_local_file_modified(obs_properties_t *props, obs_property_t *prop,
		obs_data_t *settings)
{
	UNUSED_PARAMETER(prop);
	
	bool enabled = obs_data_get_bool(settings, "is_local_file");
	obs_property_t *url = obs_properties_get(props, "url");
	obs_property_t *local_file = obs_properties_get(props, "local_file");
	obs_property_set_visible(url, !enabled);
	obs_property_set_visible(local_file, enabled);

	return true;
}

static bool reload_button_clicked(obs_properties_t *props,
		obs_property_t *property, void *data)
{
	UNUSED_PARAMETER(props);
	UNUSED_PARAMETER(property);

	QtWebkitSource *ws = static_cast<QtWebkitSource *>(data);
	ws->Reload();
	return true;
}

static obs_properties_t *qtwebkit_get_properties(void *)
{
	obs_properties_t *props = obs_properties_create();

	obs_property_t *prop = obs_properties_add_bool(props, "is_local_file",
			obs_module_text("LocalFile"));

	obs_property_set_modified_callback(prop, is_local_file_modified);
	obs_properties_add_path(props, "local_file", obs_module_text("LocalFile"),
			OBS_PATH_FILE, "*.*", nullptr);
	obs_properties_add_text(props, "url", obs_module_text("URL"), OBS_TEXT_DEFAULT);

	obs_properties_add_int(props, "width", obs_module_text("Width"), 1, 4096, 1);
	obs_properties_add_int(props, "height", obs_module_text("Height"), 1, 4096, 1);
	obs_properties_add_int(props, "fps", obs_module_text("FPS"), 1, 60, 1);
	obs_properties_add_path(props, "css_file", obs_module_text("CustomCSS"),
			OBS_PATH_FILE, "*.css", nullptr);

	obs_properties_add_button(props, "reload", obs_module_text("Reload"), reload_button_clicked);

	return props;
}

static void qtwebkit_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, "url", "http://www.obsproject.com");
	obs_data_set_default_int(settings, "width", 800);
	obs_data_set_default_int(settings, "height", 600);
	obs_data_set_default_int(settings, "fps", 30);
}

static void qtwebkit_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);
	QtWebkitSource *ws = static_cast<QtWebkitSource *>(data);
	ws->PrepareTexture();
}

static void qtwebkit_render(void *data, gs_effect_t *effect)
{
	QtWebkitSource *ws = static_cast<QtWebkitSource *>(data);
	ws->RenderTexture(effect);
}

static void *qtwebkit_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);
	QtWebkitSource *ws = new QtWebkitSource(source);
	ws->UpdateSettings(settings);
	ws->reload_key = obs_hotkey_register_source(source, "qtwebkit.reload", obs_module_text("Reload"),
			reload_hotkey_pressed, ws);
	return ws;
}

static void qtwebkit_destroy(void *data)
{
	QtWebkitSource *ws = static_cast<QtWebkitSource *>(data);
	obs_hotkey_unregister(ws->reload_key);
	delete ws;
}

static void qtwebkit_update(void *data, obs_data_t *settings)
{
	QtWebkitSource *ws = static_cast<QtWebkitSource *>(data);
	ws->UpdateSettings(settings);
}

static uint32_t qtwebkit_get_width(void *data)
{
	QtWebkitSource *ws = static_cast<QtWebkitSource *>(data);
	return ws->GetWidth();
}

static uint32_t qtwebkit_get_height(void *data)
{
	QtWebkitSource *ws = static_cast<QtWebkitSource *>(data);
	return ws->GetHeight();
}

bool obs_module_load(void)
{
	struct obs_source_info info = {};
	info.id             = "qtwebkit-source";
	info.type           = OBS_SOURCE_TYPE_INPUT;
	info.output_flags   = OBS_SOURCE_VIDEO;

	info.get_name       = qtwebkit_get_name;
	info.create         = qtwebkit_create;
	info.destroy        = qtwebkit_destroy;
	info.update         = qtwebkit_update;
	info.get_width      = qtwebkit_get_width;
	info.get_height     = qtwebkit_get_height;
	info.get_properties = qtwebkit_get_properties;
	info.get_defaults   = qtwebkit_get_defaults;
	info.video_tick     = qtwebkit_tick;
	info.video_render   = qtwebkit_render;

	obs_register_source(&info);
	return true;
}
