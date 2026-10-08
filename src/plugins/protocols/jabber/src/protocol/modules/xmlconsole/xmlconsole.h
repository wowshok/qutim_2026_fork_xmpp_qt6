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

#ifndef XMLCONSOLE_H
#define XMLCONSOLE_H

#include <QWidget>
#include "../../../sdk/jabber.h"
#include <jreen/client.h>
#include <jreen/jid.h>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTextBlock>

namespace Ui {
class XmlConsole;
}

namespace Jabber
{
class XmlConsole : public QWidget, public JabberExtension, public Jreen::XmlStreamHandler
{
	Q_OBJECT
	Q_INTERFACES(Jabber::JabberExtension)
public:
	XmlConsole(QWidget *parent = 0);
	~XmlConsole();

	void init(qutim_sdk_0_3::Account *account);
	void init(Jreen::Client *client);
	virtual void handleStreamBegin();
	virtual void handleStreamEnd();
	virtual void handleIncomingData(const char *data, qint64 size);
	virtual void handleOutgoingData(const char *data, qint64 size);
public slots:
	void show();
protected:
	void changeEvent(QEvent *e);
private:
	void stackProcess(const QByteArray &data, bool incoming);

	struct XmlNode
	{
		enum Type
		{
			Iq = 1,
			Presence = 2,
			Message = 4,
			Custom = 8
		};
		QDateTime time;
		Type type;
		bool incoming;
		QSet<QString> xmlns;
		Jreen::JID jid;
		QSet<QString> attributes;
		QTextBlock block;
		int lineCount;
	};
	enum FilterType
	{
		Disabled = 0x10,
		ByJid = 0x20,
		ByXmlns = 0x30,
		ByAllAttributes = 0x40
	};

	enum State
	{
		WaitingForStanza,
		ReadFeatures,
		ReadStanza,
		ReadCustom
	};

	struct StackToken
	{
		StackToken(QXmlStreamReader &reader)
		{
			type = reader.tokenType();
			if (type == QXmlStreamReader::StartElement) {
				startTag.name = new QString(reader.name().toString());
				startTag.xmlns = new QString(reader.namespaceUri().toString());
				startTag.attributes = new QXmlStreamAttributes(reader.attributes());
				startTag.empty = false;
			} else if (type == QXmlStreamReader::Characters) {
				charachters.text = new QString(reader.text().toString());
			} else if (type == QXmlStreamReader::EndElement) {
				endTag.name = new QString(reader.name().toString());
			}
		}
		StackToken(const QString &name)
		{
			type = QXmlStreamReader::Characters;
			charachters.text = new QString(name);
		}

		~StackToken()
		{
			if (type == QXmlStreamReader::StartElement) {
				delete startTag.name;
				delete startTag.xmlns;
				delete startTag.attributes;
			} else if (type == QXmlStreamReader::Characters) {
				delete charachters.text;
			} else if (type == QXmlStreamReader::EndElement) {
				delete endTag.name;
			}
		}

		QXmlStreamReader::TokenType type;
		union {
			struct {
				QString *name;
				QString *xmlns;
				QXmlStreamAttributes *attributes;
				bool empty;
			} startTag;
			struct {
				QString *text;
			} charachters;
			struct {
				QString *name;
			} endTag;
		};
	};

	struct StackEnvironment
	{
		QXmlStreamReader reader;
		State state;
		int depth;
		QList<StackToken*> tokens;
		QColor commentColor;
		QColor bodyColor;
		QColor tagColor;
		QColor attributeColor;
		QColor paramColor;
	};

	::Ui::XmlConsole *m_ui;
	Jreen::Client *m_client;
	QList<XmlNode> m_nodes;
	StackEnvironment m_stackIncoming;
	StackEnvironment m_stackOutgoing;
	QColor m_stackBracketsColor;
	int m_filter;

private slots:
	void on_lineEdit_textChanged(const QString &);
	void onActionGroupTriggered(QAction *action);
	void on_saveButton_clicked();
};
}

#endif // XMLCONSOLE_H

