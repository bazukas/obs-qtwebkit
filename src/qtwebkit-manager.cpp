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
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>

#include "qtwebkit-manager.hpp"

#define blog(level, msg, ...) blog(level, "qtwebkit-source: " msg, ##__VA_ARGS__)

#define SHM_NAME "/obsqtwebkit"

QtWebkitManager::QtWebkitManager(char *url, uint32_t width, uint32_t height, uint32_t fps, char *css):
	width(width), height(height), fps(fps)
{
	pthread_mutexattr_t attrmutex;

	uid = rand();
	char shm_name[50];
	snprintf(shm_name, 50, "%s%d", SHM_NAME, uid);
	fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		blog(LOG_ERROR, "shm_open error");
		return;
	}
	
	size_t data_size = width * height * 4;
	if (ftruncate(fd, sizeof(struct shared_data) + data_size) == -1) {
		blog(LOG_ERROR, "ftruncate error");
		return;
	}

	data = (struct shared_data *) mmap(NULL, sizeof(struct shared_data) + data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		blog(LOG_ERROR, "mmap error");
		return;
	}

	pthread_mutexattr_init(&attrmutex);
	pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&data->mutex, &attrmutex);

	SpawnRenderer(url, css);
}

QtWebkitManager::~QtWebkitManager()
{
	size_t data_size = width * height * 4;
	KillRenderer();
	pthread_mutex_destroy(&data->mutex);
	if (data != NULL && data != MAP_FAILED)
		munmap(data, sizeof(struct shared_data) + data_size);
	if (fd != -1) {
		char shm_name[50];
		snprintf(shm_name, 50, "%s%d", SHM_NAME, uid);
		shm_unlink(shm_name);
	}
}

void QtWebkitManager::SetUrl(char *url, char *css)
{
	KillRenderer();
	SpawnRenderer(url, css);
}

void QtWebkitManager::SpawnRenderer(char *url, char *css)
{
	char renderer[512];
	char width_buf[32];
	char height_buf[32];
	char fps_buf[32];
	char uid_buf[32];
	char *s;
	const char *file = obs_get_module_binary_path(obs_current_module());
	strncpy(renderer, file, 512);
	s = strrchr(renderer, '/');
	if (s)
		*(s+1) = '\0';
	strcat(renderer, "renderer");
	snprintf(width_buf, 32, "%d", width);
	snprintf(height_buf, 32, "%d", height);
	snprintf(fps_buf, 32, "%d", fps);
	snprintf(uid_buf, 32, "%d", uid);
	if (!css) {
		css = (char *) "";
	}
	char *argv[] = { renderer, url, width_buf, height_buf, fps_buf, uid_buf, css, NULL };
	pid = fork();
	if (pid == 0)
		execv(renderer, argv);
}

void QtWebkitManager::KillRenderer()
{
	if (pid > 0) {
		kill(pid, SIGTERM);
		waitpid(pid, NULL, 0);
	}
}

void QtWebkitManager::Lock()
{
	pthread_mutex_lock(&data->mutex);
}

void QtWebkitManager::UnLock()
{
	pthread_mutex_unlock(&data->mutex);
}
