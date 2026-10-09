/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Ruslan Nigmatullin <euroelessar@yandex.ru>
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
#include "dbusbackend.h"
#include <qutim/message.h>
#include <qutim/buddy.h>
#include <qutim/debug.h>
#include <qutim/config.h>
#include <qutim/chatsession.h>
#include <qutim/emoticons.h>
#include <QTextDocument>
#include <QVariant>
#include <QBuffer>
#include <QDBusPendingReply>
#include <QDBusConnectionInterface>
#include <QDBusReply>

using namespace qutim_sdk_0_3;

struct DBusNotifyImageData
{
	QPixmap image;
};

Q_DECLARE_METATYPE(DBusNotifyImageData)

QDBusArgument& operator<< (QDBusArgument& arg, const DBusNotifyImageData &data) {
	if (data.image.isNull()) {
		// Sometimes this gets called with a null QImage for no obvious reason.
		// - There is one reason: Qt calls this method at first time to research it's structure
		arg.beginStructure();
		arg << 0 << 0 << 0 << false << 0 << 0 << QByteArray();
		arg.endStructure();
		return arg;
	}
	QImage scaled = data.image.scaledToHeight(qMin(100, qMin(data.image.height(), data.image.width())),
											  Qt::SmoothTransformation).toImage();
	QImage i = scaled.convertToFormat(QImage::Format_ARGB32).rgbSwapped();
	arg.beginStructure();
	arg << i.width();
	arg << i.height();
	arg << i.bytesPerLine();
	arg << i.hasAlphaChannel();
	int channels = i.isGrayscale() ? 1 : (i.hasAlphaChannel() ? 4 : 3);
	arg << i.depth() / channels;
	arg << channels;
	arg << QByteArray(reinterpret_cast<const char*>(i.bits()), i.sizeInBytes());
	arg.endStructure();
	return arg;
}

const QDBusArgument& operator>> (const QDBusArgument& arg, DBusNotifyImageData &data) {
	Q_ASSERT(!"This shouldn't ever be happened");
	Q_UNUSED(data);
	return arg;
}

DBusBackend::DBusBackend() :
	NotificationBackend("Popup"),
	interface(new org::freedesktop::Notifications(
			"org.freedesktop.Notifications",
			"/org/freedesktop/Notifications",
			QDBusConnection::sessionBus()))
{
	setDescription(QT_TR_NOOP("Show popup"));
	qDBusRegisterMetaType<DBusNotifyImageData>();
	// Notify() is waited for synchronously: never let a stuck daemon freeze
	// the UI for the default 25 seconds
	interface->setTimeout(3000);
	if (!isServiceAvailable())
		qWarning() << "No org.freedesktop.Notifications service, desktop notifications are disabled";
	QDBusMessage message = QDBusMessage::createMethodCall(
				QLatin1String("org.freedesktop.Notifications"),
				QLatin1String("/org/freedesktop/Notifications"),
				QLatin1String("org.freedesktop.Notifications"),
				QLatin1String("GetCapabilities"));
	QDBusPendingReply<QStringList> call = QDBusConnection::sessionBus().asyncCall(message);
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
	connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
			this, SLOT(capabilitiesCallFinished(QDBusPendingCallWatcher*)));

	QDBusConnection::sessionBus().connect(
				QString(),
				"/org/freedesktop/Notifications",
				"org.freedesktop.Notifications",
				"ActionInvoked",
				this, SLOT(onActionInvoked(quint32,QString)));

	QDBusConnection::sessionBus().connect(
				QString(),
				"/org/freedesktop/Notifications",
				"org.freedesktop.Notifications",
				"NotificationClosed",
				this, SLOT(onNotificationClosed(quint32,quint32)));
}



bool DBusBackend::isServiceAvailable() const
{
	QDBusConnectionInterface *bus = QDBusConnection::sessionBus().interface();
	if (!bus)
		return false;
	const QString service = QStringLiteral("org.freedesktop.Notifications");
	if (bus->isServiceRegistered(service))
		return true;
	// Daemons like xfce4-notifyd may be started on demand
	QDBusReply<QStringList> activatable = bus->call(QStringLiteral("ListActivatableNames"));
	return activatable.isValid() && activatable.value().contains(service);
}

void DBusBackend::handleNotification(qutim_sdk_0_3::Notification *notification)
{
	if (!isServiceAvailable())
		return;

	NotificationRequest request = notification->request();
	NotificationData data;

	QString text = request.text().toHtmlEscaped();

	QVariantMap hints;
	if (!request.image().isNull()) {
		DBusNotifyImageData imageData = { request.image() };
		hints["image_data"] = QVariant::fromValue(imageData);
	}
	hints["desktop-entry"] = QLatin1String("qutim");
	// No "sound-file" hint: qutim plays notification sounds itself

	// Further events from the same sender update its notification
	quint32 id = m_ids.value(request.object(), 0);
	if (id) {
		data = m_notifications.value(id);
		data.body = data.body + "\n" + text;
	} else {
		data.sender = request.object();
		data.body = text;
	}

	QStringList actions;
	if (m_capabilities.contains(QLatin1String("actions"))) {
		// Clicking the notification itself runs the first action (usually "Open chat")
		const QList<NotificationAction> requestActions = request.actions();
		if (!requestActions.isEmpty()) {
			data.actions.replace(QStringLiteral("default"), requestActions.first());
			actions << QStringLiteral("default") << QString();
		}
		foreach (const NotificationAction &action, requestActions) {
			QString name = action.title().original();
			data.actions.insert(name, action);
			actions << name << action.title();
		}
	}

	const int timeout = 5000;
	QDBusPendingReply<uint> reply = interface->Notify(
				QCoreApplication::applicationName(),
				id,
				QLatin1String("qutim"),
				request.title(),
				data.body,
				actions,
				hints,
				timeout);
	reply.waitForFinished();
	if (reply.isError()) {
		qWarning() << "Failed to show desktop notification:" << reply.error().message();
		return;
	}

	ref(notification);
	data.notifications << notification;
	const quint32 newId = reply.value();
	m_notifications.insert(newId, data);
	m_ids.insert(request.object(), newId);
	if (id && id != newId)
		m_notifications.remove(id);
}

DBusBackend::~DBusBackend()
{
}

void DBusBackend::callFinished(QDBusPendingCallWatcher *watcher)
{
	watcher->deleteLater();
}

void DBusBackend::capabilitiesCallFinished(QDBusPendingCallWatcher* watcher)
{
	watcher->deleteLater();
	QDBusPendingReply<QStringList> reply = *watcher;
	const QStringList capabilities = reply.argumentAt<0>();
	m_capabilities = QSet<QString>(capabilities.begin(), capabilities.end());
}

void DBusBackend::onActionInvoked(quint32 id, const QString &name)
{
	NotificationData data = m_notifications.value(id);
	foreach (const NotificationAction &action, data.actions.values(name))
		action.trigger();

#ifdef Q_WS_MAEMO_5
	//Maemo dbus implementation
	QPointer<QObject> sender = data.sender;
	if (name == "default" ) {
		ChatUnit *unit = qobject_cast<ChatUnit *>(sender.data());

		if (unit) {
			ChatUnit *metaContact = unit->metaContact();
			if (metaContact)
				unit = metaContact;
			ChatLayer::get(unit,true)->activate();
		}
	}
#endif
}

inline void DBusBackend::ignore(NotificationData &data)
{
	Q_UNUSED(data);
	foreach (const QPointer<Notification> &notification, data.notifications)
		if (notification)
			notification.data()->ignore();
}

void DBusBackend::onNotificationClosed(quint32 id, quint32 reason)
{
	/*
	  The reasons:
	  1 - The notification expired.
	  2 - The notification was dismissed by the user.
	  3 - The notification was closed by a call to CloseNotification.
	  4 - Undefined/reserved reasons.
	 */

	QHash<quint32, NotificationData>::iterator itr = m_notifications.find(id);
	if (itr != m_notifications.end()) {
		if (reason == 2)
			ignore(*itr);
		m_ids.remove(itr->sender.data());
		foreach (const QPointer<Notification> &notification, itr->notifications)
			if (notification)
				deref(notification.data());
		m_notifications.erase(itr);
	}
}

