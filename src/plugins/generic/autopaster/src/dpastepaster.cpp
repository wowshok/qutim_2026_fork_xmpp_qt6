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

#include "dpastepaster.h"
#include <QCoreApplication>

// dpaste.com API v2: POST a form, the reply body is the paste URL.
// The service rejects requests without a User-Agent.
QString DpastePaster::name()
{
	return QStringLiteral("dpaste.com");
}

QNetworkReply *DpastePaster::send(QNetworkAccessManager *manager, const QString &content, const QString &syntax)
{
	// Encode every value fully: QUrlQuery leaves '+' as is, which form
	// decoding turns into a space ("i++" would arrive as "i  ")
	const QByteArray form = "content=" + QUrl::toPercentEncoding(content)
			+ "&syntax=" + QUrl::toPercentEncoding(syntax.isEmpty() ? QStringLiteral("text") : syntax)
			+ "&expiry_days=30";

	QNetworkRequest request(QUrl(QStringLiteral("https://dpaste.com/api/v2/")));
	request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));
	request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName() + QLatin1String(" autopaster"));
	return manager->post(request, form);
}

QUrl DpastePaster::handle(QNetworkReply *reply, QString *error)
{
	const QUrl url(QString::fromUtf8(reply->readAll()).trimmed());
	if (!url.isValid() || url.scheme() != QLatin1String("https"))
		*error = QCoreApplication::translate("AutoPaster", "Unexpected reply from %1").arg(name());
	return url;
}
