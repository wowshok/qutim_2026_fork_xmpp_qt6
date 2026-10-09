/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2013 Roman Tretyakov <roman@trett.ru>
**
*****************************************************************************
**
** $QUTIM_BEGIN_LICENSE$
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/.
** $QUTIM_END_LICENSE$
**
****************************************************************************/

#include "idle.h"

#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include <xcb/xcb.h>
#include <xcb/screensaver.h>

#include <cstdlib>

// Idle time on Linux, tried in this order:
//  - X11 (XFCE and other X sessions): the MIT-SCREEN-SAVER extension;
//  - GNOME/Mutter, also on Wayland: org.gnome.Mutter.IdleMonitor;
//  - KDE and others: org.freedesktop.ScreenSaver.GetSessionIdleTime.
// If none of them works, Idle falls back to watching the mouse position.

namespace Psi {

class IdlePlatform::Private
{
public:
	enum Backend { None, X11, Mutter, FreedesktopScreenSaver };

	Backend backend = None;
	xcb_connection_t *connection = nullptr;
	xcb_window_t root = 0;

	bool initX11()
	{
		auto x11 = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
		if (!x11 || !x11->connection())
			return false;
		connection = x11->connection();
		const xcb_query_extension_reply_t *extension = xcb_get_extension_data(connection, &xcb_screensaver_id);
		if (!extension || !extension->present)
			return false;
		xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
		if (!screen)
			return false;
		root = screen->root;
		return x11Idle() >= 0;
	}

	qint64 x11Idle() const
	{
		xcb_screensaver_query_info_cookie_t cookie = xcb_screensaver_query_info(connection, root);
		xcb_screensaver_query_info_reply_t *info = xcb_screensaver_query_info_reply(connection, cookie, nullptr);
		if (!info)
			return -1;
		const qint64 ms = info->ms_since_user_input;
		std::free(info);
		return ms;
	}

	qint64 mutterIdle() const
	{
		QDBusInterface monitor(QStringLiteral("org.gnome.Mutter.IdleMonitor"),
							   QStringLiteral("/org/gnome/Mutter/IdleMonitor/Core"),
							   QStringLiteral("org.gnome.Mutter.IdleMonitor"),
							   QDBusConnection::sessionBus());
		QDBusReply<quint64> reply = monitor.call(QStringLiteral("GetIdletime"));
		return reply.isValid() ? qint64(reply.value()) : -1;
	}

	qint64 screenSaverIdle() const
	{
		QDBusInterface saver(QStringLiteral("org.freedesktop.ScreenSaver"),
							 QStringLiteral("/org/freedesktop/ScreenSaver"),
							 QStringLiteral("org.freedesktop.ScreenSaver"),
							 QDBusConnection::sessionBus());
		QDBusReply<uint> reply = saver.call(QStringLiteral("GetSessionIdleTime"));
		// Reported in seconds
		return reply.isValid() ? qint64(reply.value()) * 1000 : -1;
	}

	qint64 idleMs() const
	{
		switch (backend) {
		case X11: return x11Idle();
		case Mutter: return mutterIdle();
		case FreedesktopScreenSaver: return screenSaverIdle();
		case None: break;
		}
		return -1;
	}
};

IdlePlatform::IdlePlatform() : d(new Private)
{
}

IdlePlatform::~IdlePlatform()
{
	delete d;
}

bool IdlePlatform::init()
{
	if (d->initX11())
		d->backend = Private::X11;
	else if (d->mutterIdle() >= 0)
		d->backend = Private::Mutter;
	else if (d->screenSaverIdle() >= 0)
		d->backend = Private::FreedesktopScreenSaver;
	return d->backend != Private::None;
}

int IdlePlatform::secondsIdle()
{
	const qint64 ms = d->idleMs();
	return ms < 0 ? 0 : int(ms / 1000);
}

} // namespace Psi
