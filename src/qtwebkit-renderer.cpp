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

void term(int signum)
{
	done = 1;
}

void init_shared_data(int width, int height)
{
	size_t data_size = width * height * 4;
	fd = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR);
	data = (struct shared_data *) mmap(NULL, sizeof(struct shared_data) + data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

int main(int argc, char *argv[])
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = term;
	sigaction(SIGTERM, &action, NULL);

	int fps = atoi(argv[4]);
	int width = atoi(argv[2]);
	int height = atoi(argv[3]);

	init_shared_data(atoi(argv[2]), atoi(argv[3]));

	QApplication app(argc, argv);
	QWebPage page;

	page.mainFrame()->setUrl(QUrl::fromUserInput(argv[1]));
	page.setViewportSize(QSize(width, height));
	page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

	pthread_mutex_lock(&data->mutex);
	QImage image(&data->data, width, height, QImage::Format_RGBA8888);
	image.fill(0);
	QPainter painter(&image);
	pthread_mutex_unlock(&data->mutex);

	while (!done) {
		app.processEvents();

		pthread_mutex_lock(&data->mutex);
		page.mainFrame()->render(&painter, QWebFrame::ContentsLayer);
		pthread_mutex_unlock(&data->mutex);

		usleep(1000000 / fps);
	}

	return 0;
}
