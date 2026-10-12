/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Aleksey Sidorov <gorthauer87@yandex.ru>
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

#ifndef QUICKCHATVIEWCONTROLLER_H
#define QUICKCHATVIEWCONTROLLER_H

#include "../lib/webkitmessageviewstyle.h"
#include <qutim/adiumchat/chatviewfactory.h>
#include <qutim/message.h>
#include <QWebEnginePage>
#include <QVariant>
#include <QPointer>

class QWebEngineProfile;

namespace Adium {

class WebViewController;

// Published to the page as "client" through QWebChannel, see data/webview/client.js
class WebViewClient : public QObject
{
	Q_OBJECT
public:
	explicit WebViewClient(WebViewController *controller);

public slots:
	void debugLog(const QString &message);
	void appendNick(const QVariant &nick);
	void contextMenu(const QVariant &nick);
	void appendText(const QVariant &text);
	void setTopic(const QString &topic);

private:
	WebViewController *m_controller;
};

class WebViewController : public QWebEnginePage, public Core::AdiumChat::ChatViewController
{
	Q_OBJECT
	Q_INTERFACES(Core::AdiumChat::ChatViewController)
public:
	WebViewController(bool isPreview = false);
	virtual ~WebViewController();

	static QWebEngineProfile *chatProfile();

	virtual void setChatSession(qutim_sdk_0_3::ChatSession *session);
	virtual qutim_sdk_0_3::ChatSession *getSession() const;
	virtual void appendMessage(const qutim_sdk_0_3::Message &msg);
	virtual void clearChat();
	virtual QString quote();
	WebKitMessageViewStyle *style();
	void setDefaultFont(const QString &family, int size);
	QString defaultFontFamily() const;
	int defaultFontSize() const;
	bool eventFilter(QObject *obj, QEvent *);
	void scrollToBottom();

public slots:
	// The result is not available, scripts run asynchronously
	QVariant evaluateJavaScript(const QString &script);
	void debugLog(const QString &message);
	void appendNick(const QVariant &nick);
	void contextMenu(const QVariant &nickVar);
	void appendText(const QVariant &text);
	void setTopic(const QString &topic);

protected:
	void clearFocusClass();
	bool isContentSimiliar(const qutim_sdk_0_3::Message &a, const qutim_sdk_0_3::Message &b);
	void loadSettings(bool onFly);
	bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
	QWebEnginePage *createWindow(WebWindowType type) override;
	void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
								  int lineNumber, const QString &sourceID) override;

private slots:
	void onSettingsSaved();
	void onLoadFinished(bool ok);
	void onTopicChanged(const QString &topic);
	void updateTopic();

private:
	QPointer<qutim_sdk_0_3::ChatSession> m_session;
	QString m_styleName;
	WebKitMessageViewStyle m_style;
	bool m_isLoading;
	bool m_isPreview;
	QStringList m_pendingScripts;
	qutim_sdk_0_3::Message m_last;
	qutim_sdk_0_3::Message m_topic;
};

} // namespace Adium

#endif // QUICKCHATVIEWCONTROLLER_H
