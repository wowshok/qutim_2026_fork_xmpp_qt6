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

#ifndef DPASTEPASTER_H
#define DPASTEPASTER_H

#include "pasterinterface.h"

class DpastePaster : public PasterInterface
{
public:
	QString name() override;
	QNetworkReply *send(QNetworkAccessManager *manager, const QString &content, const QString &syntax) override;
	QUrl handle(QNetworkReply *reply, QString *error) override;
};

#endif // DPASTEPASTER_H
