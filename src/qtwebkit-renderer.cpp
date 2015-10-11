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

#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <pthread.h>

#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include <QPainter>

#define SHM_NAME "/obsqtwebkit"

struct shared_data {
	pthread_mutex_t mutex;
	uint8_t data;
};

static volatile sig_atomic_t done = 0;
static int fd;
static struct shared_data *data;

static int in_fd;
static char buf[sizeof(struct inotify_event)];
static volatile sig_atomic_t refresh = 0;

void term(int signum)
{
	done = 1;
}

void file_changed(int signum)
{
	read(in_fd, (void *) buf, sizeof(buf)); // right now don't really care what we get
	refresh = 1;
}

void init_shared_data(int width, int height, char *suffix)
{
	char shm_name[50];
	snprintf(shm_name, 50, "%s%s", SHM_NAME, suffix);

	size_t data_size = width * height * 4;
	fd = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
	data = (struct shared_data *) mmap(NULL, sizeof(struct shared_data) + data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

void uninit_shared_data(int width, int height)
{
	size_t data_size = width * height * 4;
	munmap(data, sizeof(struct shared_data) + data_size);
}

void init_inotify(char *file)
{
	in_fd = inotify_init1(IN_NONBLOCK);
	inotify_add_watch(in_fd, file, IN_MODIFY);
	fcntl(in_fd, F_SETFL, O_ASYNC);
	fcntl(in_fd, F_SETOWN, getpid());
	struct sigaction action;

	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = file_changed;
	sigaction(SIGIO, &action, NULL);
}

void uninit_inotify()
{
	close(in_fd);
}

int main(int argc, char *argv[])
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = term;
	sigaction(SIGTERM, &action, NULL);

	// shutdown if parent process dies
	prctl(PR_SET_PDEATHSIG, SIGTERM);

	int width = atoi(argv[2]);
	int height = atoi(argv[3]);
	int fps = atoi(argv[4]);
	char *suffix = argv[5];
	init_shared_data(atoi(argv[2]), atoi(argv[3]), suffix);

	QApplication app(argc, argv);
	QWebPage page;

	QPalette palette = page.palette();
	palette.setBrush(QPalette::Base, Qt::transparent);
	page.setPalette(palette);
	page.settings()->setUserStyleSheetUrl(QUrl::fromUserInput(argv[6]));
	page.settings()->setObjectCacheCapacities(0, 0, 0);

	const QUrl url = QUrl::fromUserInput(argv[1]);
	page.setViewportSize(QSize(width, height));
	page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
	page.mainFrame()->setUrl(url);

	pthread_mutex_lock(&data->mutex);
	QImage image(&data->data, width, height, QImage::Format_RGBA8888);
	image.fill(0);
	QPainter painter(&image);
	pthread_mutex_unlock(&data->mutex);

	if (url.isLocalFile()) {
		init_inotify(argv[1]);
	}

	while (!done) {
		app.processEvents();

		pthread_mutex_lock(&data->mutex);
		image.fill(0);
		page.mainFrame()->render(&painter, QWebFrame::ContentsLayer);
		pthread_mutex_unlock(&data->mutex);

		// reload file if changed
		if (refresh) {
			refresh = 0;
			page.mainFrame()->setUrl(url);
		}

		usleep(1000000 / fps);
	}

	if (url.isLocalFile()) {
		uninit_inotify();
	}

	uninit_shared_data(atoi(argv[2]), atoi(argv[3]));

	return 0;
}
