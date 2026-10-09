/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2026 qutIM developers
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

#include "pasterspaster.h"
#include <QCoreApplication>

// paste.rs: POST the raw text, the reply body is the paste URL
QString PasteRsPaster::name()
{
	return QStringLiteral("paste.rs");
}

QNetworkReply *PasteRsPaster::send(QNetworkAccessManager *manager, const QString &content, const QString &syntax)
{
	Q_UNUSED(syntax);
	QNetworkRequest request(QUrl(QStringLiteral("https://paste.rs/")));
	request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("text/plain; charset=utf-8"));
	return manager->post(request, content.toUtf8());
}

QUrl PasteRsPaster::handle(QNetworkReply *reply, QString *error)
{
	const QUrl url(QString::fromUtf8(reply->readAll()).trimmed());
	if (!url.isValid() || url.scheme() != QLatin1String("https"))
		*error = QCoreApplication::translate("AutoPaster", "Unexpected reply from %1").arg(name());
	return url;
}
