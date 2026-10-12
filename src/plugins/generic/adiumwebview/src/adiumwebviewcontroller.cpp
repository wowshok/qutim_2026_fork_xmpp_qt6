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

#include "adiumwebviewcontroller.h"
#include "../lib/webkiturlinterceptor.h"
#include <qutim/utils.h>
#include <qutim/message.h>
#include <qutim/thememanager.h>
#include <qutim/debug.h>
#include <qutim/account.h>
#include <qutim/conference.h>
#include <qutim/history.h>
#include <qutim/emoticons.h>
#include <qutim/notification.h>
#include <qutim/servicemanager.h>
#include <qutim/chatsession.h>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QApplication>
#include <QDesktopServices>
#include <QWebChannel>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QFile>
#include <QDir>

namespace qutim_sdk_0_3 {
// Escapes a string for a double quoted JavaScript literal, from libqutim's utils.cpp
LIBQUTIM_EXPORT QString &validateCpp(QString &text);
}

namespace Adium {

using namespace qutim_sdk_0_3;
using namespace Core::AdiumChat;

static QString readFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << "Unable to read" << fileName << file.errorString();
		return QString();
	}
	return QString::fromUtf8(file.readAll());
}

// Opens links from target="_blank" and window.open() in the system browser
class ExternalLinkPage : public QWebEnginePage
{
public:
	ExternalLinkPage(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent) {}

protected:
	bool acceptNavigationRequest(const QUrl &url, NavigationType, bool) override
	{
		QDesktopServices::openUrl(url);
		deleteLater();
		return false;
	}
};

WebViewClient::WebViewClient(WebViewController *controller)
	: QObject(controller), m_controller(controller)
{
}

void WebViewClient::debugLog(const QString &message)
{
	m_controller->debugLog(message);
}

void WebViewClient::appendNick(const QVariant &nick)
{
	m_controller->appendNick(nick);
}

void WebViewClient::contextMenu(const QVariant &nick)
{
	m_controller->contextMenu(nick);
}

void WebViewClient::appendText(const QVariant &text)
{
	m_controller->appendText(text);
}

void WebViewClient::setTopic(const QString &topic)
{
	m_controller->setTopic(topic);
}

QWebEngineProfile *WebViewController::chatProfile()
{
	static QPointer<QWebEngineProfile> profile;
	if (!profile) {
		// Off the record: nothing of the chats is cached on disk
		profile = new QWebEngineProfile(qApp);
		profile->setUrlRequestInterceptor(new WebKitUrlInterceptor(profile));

		// The style's own scripts may call client.* while the page is still loading,
		// so the bridge is created before any of them runs
		QDir shareDir = ThemeManager::path(QStringLiteral("data"), QStringLiteral("webview"));
		QWebEngineScript script;
		script.setName(QStringLiteral("qutim-client"));
		script.setInjectionPoint(QWebEngineScript::DocumentCreation);
		script.setWorldId(QWebEngineScript::MainWorld);
		script.setRunsOnSubFrames(false);
		script.setSourceCode(readFile(QStringLiteral(":/qtwebchannel/qwebchannel.js"))
							 + QLatin1Char('\n')
							 + readFile(shareDir.filePath(QStringLiteral("client.js")))
							 + QLatin1String("\nclient.initWebChannel();\n"));
		profile->scripts()->insert(script);
	}
	return profile;
}

WebViewController::WebViewController(bool isPreview) :
	QWebEnginePage(chatProfile()), m_isLoading(false), m_isPreview(isPreview)
{
	m_topic.setProperty("topic", true);

	QWebEngineSettings *webSettings = settings();
	// Styles live in local files, but messages may show remote images (urlpreview)
	webSettings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
	webSettings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);

	QWebChannel *channel = new QWebChannel(this);
	channel->registerObject(QStringLiteral("client"), new WebViewClient(this));
	setWebChannel(channel);

	connect(this, &QWebEnginePage::loadFinished, this, &WebViewController::onLoadFinished);
}

WebViewController::~WebViewController()
{
}

void WebViewController::setChatSession(ChatSession *session)
{
	if (m_session != session) {
		m_session = session;
		m_session.data()->installEventFilter(this);
		if (qobject_cast<Conference*>(m_session.data()->unit())) {
			connect(m_session.data()->unit(), SIGNAL(topicChanged(QString,QString)),
					this, SLOT(onTopicChanged(QString)));
		}
		if (!m_isPreview) {
			loadSettings(false);
			clearChat();
		}
	}
}

ChatSession *WebViewController::getSession() const
{
	return m_session.data();
}

bool WebViewController::isContentSimiliar(const Message &a, const Message &b)
{
	int flags = 0;
	if (!m_style.hasAction())
		flags |= Message::IgnoreActions;
	return a.isSimiliar(b, flags);
}

void WebViewController::appendMessage(const qutim_sdk_0_3::Message &msg)
{
	Message copy = msg;
	QString html = UrlParser::parseUrls(copy.html(), UrlParser::Html);
	copy.setProperty("messageId", msg.id());
	if (msg.property("topic", false)) {
		copy.setHtml(html);
		m_topic = copy;
		if (!m_isLoading)
			updateTopic();
		return;
	}
	if (msg.property("firstFocus", false))
		clearFocusClass();
	// We don't want emoticons in topic
	html = Emoticons::theme().parseEmoticons(html);
	copy.setHtml(html);
	bool similiar = isContentSimiliar(m_last, msg);
	QString script = m_style.scriptForAppendingContent(copy, similiar, false, false);
	m_last = msg;
	evaluateJavaScript(script);
}

void WebViewController::clearChat()
{
	Q_ASSERT(!m_session.isNull());
	m_last = Message();
	m_isLoading = true;
	// Whatever was waiting for the old document has nothing to apply to
	m_pendingScripts.clear();
	setHtml(m_style.baseTemplateForChat(m_session.data()), m_style.baseUrl());
	evaluateJavaScript(m_style.scriptForSettingCustomStyle());
}

QString WebViewController::quote()
{
	return selectedText();
}

WebKitMessageViewStyle *WebViewController::style()
{
	return &m_style;
}

void WebViewController::setDefaultFont(const QString &family, int size)
{
	QWebEngineSettings *settings = QWebEnginePage::settings();
	QFontInfo info(QFont(family, size));
	QWebEngineSettings::FontFamily families[] = {
		QWebEngineSettings::StandardFont,
		QWebEngineSettings::FixedFont,
		QWebEngineSettings::SerifFont,
		QWebEngineSettings::SansSerifFont
	};
	const int familiesSize = sizeof(families) / sizeof(families[0]);
	if (family.isEmpty()) {
		for (int i = 0; i < familiesSize; ++i)
			settings->resetFontFamily(families[i]);
	} else {
		for (int i = 0; i < familiesSize; ++i)
			settings->setFontFamily(families[i], family);
	}
	if (size < 0)
		settings->resetFontSize(QWebEngineSettings::DefaultFontSize);
	else
		settings->setFontSize(QWebEngineSettings::DefaultFontSize, info.pixelSize());
}

QString WebViewController::defaultFontFamily() const
{
	return QWebEnginePage::settings()->fontFamily(QWebEngineSettings::StandardFont);
}

int WebViewController::defaultFontSize() const
{
	QWebEngineSettings *settings = QWebEnginePage::settings();
	QString family = settings->fontFamily(QWebEngineSettings::StandardFont);
	int size = settings->fontSize(QWebEngineSettings::DefaultFontSize);
	QFont font(family);
	font.setPixelSize(size);
	QFontInfo info(font);
	return info.pointSize();
}

bool WebViewController::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == m_session.data() && ev->type() == MessageReceiptEvent::eventType()) {
		MessageReceiptEvent *msgEvent = static_cast<MessageReceiptEvent *>(ev);
		evaluateJavaScript(QStringLiteral("client.markMessage(%1, %2);")
						   .arg(msgEvent->id())
						   .arg(msgEvent->success() ? QLatin1String("true") : QLatin1String("false")));
		return true;
	}
	return QWebEnginePage::eventFilter(obj, ev);
}

void WebViewController::scrollToBottom()
{
	evaluateJavaScript(QStringLiteral("window.scrollTo(0, document.body.scrollHeight);"));
}

QVariant WebViewController::evaluateJavaScript(const QString &script)
{
	if (!m_session || m_isLoading)
		m_pendingScripts << script;
	else
		runJavaScript(script);
	return QVariant();
}

void WebViewController::debugLog(const QString &message)
{
	qDebug() << "WebKit: " << message;
}

void WebViewController::appendNick(const QVariant &nick)
{
	QObject *form = ServiceManager::getByName("ChatForm");
	QObject *obj = 0;
	if (QMetaObject::invokeMethod(form, "textEdit", Q_RETURN_ARG(QObject*, obj),
								  Q_ARG(qutim_sdk_0_3::ChatSession*, m_session.data())) && obj) {
		QTextCursor cursor;
		if (QTextEdit *edit = qobject_cast<QTextEdit*>(obj))
			cursor = edit->textCursor();
		else if (QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(obj))
			cursor = edit->textCursor();
		else
			return;
		if(cursor.atStart())
			cursor.insertText(nick.toString() + ": ");
		else
			cursor.insertText(nick.toString() + " ");
		static_cast<QWidget*>(obj)->setFocus();
	}
}

void WebViewController::contextMenu(const QVariant &nickVar)
{
	if (!m_session)
		return;
	QString nick = nickVar.toString();
	foreach (ChatUnit *unit, m_session.data()->unit()->lowerUnits()) {
		if (Buddy *buddy = qobject_cast<Buddy*>(unit)) {
			if (buddy->name() == nick)
				buddy->showMenu(QCursor::pos());
		}
	}
}

void WebViewController::appendText(const QVariant &text)
{
	QObject *form = ServiceManager::getByName("ChatForm");
	QObject *obj = 0;
	if (QMetaObject::invokeMethod(form, "textEdit", Q_RETURN_ARG(QObject*, obj),
								  Q_ARG(qutim_sdk_0_3::ChatSession*, m_session.data())) && obj) {
		QTextCursor cursor;
		if (QTextEdit *edit = qobject_cast<QTextEdit*>(obj))
			cursor = edit->textCursor();
		else if (QPlainTextEdit *edit = qobject_cast<QPlainTextEdit*>(obj))
			cursor = edit->textCursor();
		else
			return;
		cursor.insertText(text.toString());
		cursor.insertText(" ");
		static_cast<QWidget*>(obj)->setFocus();
	}
}

void WebViewController::loadSettings(bool onFly)
{
	Config config(QLatin1String("appearance/adiumChat"));
	config.beginGroup(QLatin1String("style"));
	if (!onFly) {
		m_styleName = config.value(QLatin1String("name"), QLatin1String("default"));
		m_style.setStylePath(ThemeManager::path(QLatin1String("webkitstyle"), m_styleName));
		m_style.setShowUserIcons(config.value(QLatin1String("showUserIcons"), true));
		m_style.setShowHeader(config.value(QLatin1String("showHeader"), true));
	}
	config.beginGroup(m_styleName);
	QString variant = config.value(QLatin1String("variant"), m_style.defaultVariant());
	m_style.setActiveVariant(variant);
	m_style.setCustomBackgroundType(config.value(QLatin1String("backgroundType"),
												 WebKitMessageViewStyle::BackgroundNormal));
	if (config.value(QLatin1String("customBackground"), false)) {
		m_style.setCustomBackgroundPath(config.value(QLatin1String("backgroundPath"), QString()));
		m_style.setCustomBackgroundColor(config.value(QLatin1String("backgroundColor"), QColor()));
	}
	QString fontFamily = config.value(QLatin1String("fontFamily"), m_style.defaultFontFamily());
	int fontSize = config.value(QLatin1String("fontSize"), m_style.defaultFontSize());
	setDefaultFont(fontFamily, fontSize);

	QString css;
	QString customFile = m_style.pathForResource(QLatin1String("Custom.json"));
	if (!customFile.isEmpty()) {
		QVariantList values = config.value(QLatin1String("customStyle"), QVariantList());
		Config variables(customFile);
		const int count = variables.arraySize();
		for (int index = 0; index < count; index++) {
			variables.setArrayIndex(index);
			css.append(QString::fromLatin1("%1 { %2: %3; } ")
					   .arg(variables.value(QLatin1String("selector"), QString()))
					   .arg(variables.value(QLatin1String("parameter"), QString()))
					   .arg(values.value(index, variables.value(QLatin1String("value"))).toString()));
		}
	}
	m_style.setCustomStyle(css);
}

void WebViewController::onSettingsSaved()
{
	loadSettings(true);
	evaluateJavaScript(m_style.scriptForChangingVariant());
	evaluateJavaScript(m_style.scriptForSettingCustomStyle());
}

void WebViewController::onLoadFinished(bool ok)
{
	// A load that was replaced by a newer clearChat() also finishes, unsuccessfully;
	// the scripts wait for the document they were meant for
	if (!ok) {
		qDebug() << "Adium: chat page load was interrupted";
		return;
	}
	m_isLoading = false;
	const QStringList scripts = m_pendingScripts;
	m_pendingScripts.clear();
	for (const QString &script : scripts)
		runJavaScript(script);
	if (m_session && qobject_cast<Conference*>(m_session.data()->unit()))
		updateTopic();
}

void WebViewController::onTopicChanged(const QString &topic)
{
	if (m_topic.text() == topic)
		return;
	m_topic.setText(QString());
	if (!m_isLoading)
		updateTopic();
}

void WebViewController::updateTopic()
{
	if (!m_session)
		return;
	if (m_topic.text().isEmpty()) {
		Conference *conference = qobject_cast<Conference*>(m_session.data()->unit());
		if (!conference) {
			qWarning() << "Called WebViewController::updateTopic for non-conference";
			m_topic.setText(QString());
			m_topic.setHtml(QString());
		} else {
			m_topic.setText(conference->topic());
			m_topic.setHtml(QString());
			m_topic.setHtml(UrlParser::parseUrls(m_topic.html()));
		}
		m_topic.setTime(QDateTime::currentDateTime());
	}
	QString html = m_style.templateForContent(m_topic, false);
	evaluateJavaScript(QStringLiteral("client.updateTopic(\"%1\");").arg(validateCpp(html)));
}

void WebViewController::setTopic(const QString &topic)
{
	Conference *conference = m_session ? qobject_cast<Conference*>(m_session.data()->unit()) : nullptr;
	if (!conference)
		return;
	conference->setTopic(topic);
	updateTopic();
}

void WebViewController::clearFocusClass()
{
	evaluateJavaScript(QStringLiteral("client.clearFocus();"));
}

bool WebViewController::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
	// Our own setHtml() and the style's frames load, links go to the browser
	if (type == NavigationTypeLinkClicked) {
		QDesktopServices::openUrl(url);
		return false;
	}
	return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

QWebEnginePage *WebViewController::createWindow(WebWindowType type)
{
	Q_UNUSED(type);
	return new ExternalLinkPage(profile(), this);
}

void WebViewController::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
												 const QString &message, int lineNumber,
												 const QString &sourceID)
{
	if (level == ErrorMessageLevel || qEnvironmentVariableIsSet("ADIUM_WEBKIT_DEBUG"))
		qDebug() << "Adium:" << sourceID << lineNumber << message;
}

} // namespace Adium
