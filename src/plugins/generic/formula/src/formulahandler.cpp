/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2012 Ruslan Nigmatullin <euroelessar@yandex.ru>
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

#include "formulahandler.h"
#include <qutim/debug.h>
#include <QUrl>
#include <QTextDocument>

using namespace qutim_sdk_0_3;

FormulaHandler::FormulaHandler()
	// $$...$$, shortest match (QRegExp::setMinimal in the Qt 5 version)
	: m_regexp(QStringLiteral("\\$\\$(.*?)\\$\\$"))
{
	Q_ASSERT(m_regexp.isValid());
}

MessageHandlerAsyncResult FormulaHandler::doHandle(Message &message)
{
	qsizetype lastIndex = 0;
	const QString html = message.html();
	QString newHtml;
	newHtml.reserve(html.size());
	QRegularExpressionMatchIterator it = m_regexp.globalMatch(html);
	while (it.hasNext()) {
		const QRegularExpressionMatch match = it.next();
		newHtml += QStringView(html).mid(lastIndex, match.capturedStart() - lastIndex);
		// html is already escaped
		const QString equation = match.captured(0);
		const QString url = QLatin1String("https://latex.codecogs.com/png.latex?")
							+ QString::fromLatin1(QUrl::toPercentEncoding(unescape(match.captured(1))));
		newHtml += QLatin1String("<img src=\"");
		newHtml += url;
		newHtml += QLatin1String("\" alt=\"");
		newHtml += equation;
		newHtml += QLatin1String("\" title=\"");
		newHtml += equation;
		newHtml += QLatin1String("\">");
		lastIndex = match.capturedEnd();
	}
	newHtml += QStringView(html).mid(lastIndex);
	message.setHtml(newHtml);
	return makeAsyncResult(Accept, QString());
}
