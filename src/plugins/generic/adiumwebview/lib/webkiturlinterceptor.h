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

#ifndef WEBKITURLINTERCEPTOR_H
#define WEBKITURLINTERCEPTOR_H

#include <QWebEngineUrlRequestInterceptor>

class QUrl;

// Adium styles come from case-insensitive file systems and often refer
// to their resources with the wrong case, find the real files for them
class WebKitUrlInterceptor : public QWebEngineUrlRequestInterceptor
{
	Q_OBJECT
public:
	explicit WebKitUrlInterceptor(QObject *parent = 0);

	void interceptRequest(QWebEngineUrlRequestInfo &info) override;
private:
	void fixLocalUrl(QUrl &url);
};

#endif // WEBKITURLINTERCEPTOR_H
