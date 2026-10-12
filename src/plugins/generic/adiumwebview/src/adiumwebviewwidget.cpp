/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2012 Ruslan Nigmatullin <euroelessar@yandex.ru>
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

#include "adiumwebviewwidget.h"
#include "adiumwebviewcontroller.h"
#include <qutim/config.h>
#include <qutim/servicemanager.h>
#include <QWebEngineContextMenuRequest>
#include <QApplication>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QUrlQuery>
#include <qutim/icon.h>

namespace Adium {

using namespace qutim_sdk_0_3;

WebViewWidget::WebViewWidget(QWidget *parent)
	: QWebEngineView(parent)
{
	if (QObject *scroller = ServiceManager::getByName("Scroller"))
		QMetaObject::invokeMethod(scroller,
								  "enableScrolling",
								  Q_ARG(QObject*, this));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showCustomContextMenu(QPoint)));
	Config cfg = Config("appearance");
	cfg.beginGroup("chat");
	m_searcher = cfg.value("defaultSearch", "Yandex");
	cfg.endGroup();
}

QSize WebViewWidget::sizeHint() const
{
	// QWebEngineView has no size hint until a page is shown, the chat splitter
	// collapsed the view to nothing. QWebView used to ask for this much.
	return QSize(800, 600);
}

void WebViewWidget::setViewController(QObject* object)
{
	if (m_controller.data() == object)
		return;

	if (!m_controller.isNull())
		m_controller.data()->disconnect(this);

	WebViewController *controller = qobject_cast<WebViewController*>(object);
	m_controller = controller;
	if (controller) {
		setPage(controller);
		controller->scrollToBottom();
	}
}

void WebViewWidget::showCustomContextMenu(const QPoint &point)
{
	QWebEngineContextMenuRequest *request = lastContextMenuRequest();
	if (!request || !m_controller)
		return;

	QMenu *menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose, true);
	bool linkUnderMouse = !request->linkUrl().isEmpty();

	if (linkUnderMouse) {
		m_urlForOpen = request->linkUrl();

		if(m_urlForOpen.toString().startsWith("mailto:")) { // && endsWith("?message")
			QAction *xmppChat = new QAction(tr("&Open chat"), menu);
			menu->addAction(xmppChat);
			connect(xmppChat, SIGNAL(triggered()), SLOT(openChat()));

			QAction *xmppConf = new QAction(tr("&Join conference"), menu);
			menu->addAction(xmppConf);
			connect(xmppConf, SIGNAL(triggered()), SLOT(openConference()));
		}

		// The page's own OpenLink action would navigate the chat away
		QAction *openLink = new QAction(qutim_sdk_0_3::Icon("document-open"), tr("Open link"), menu);
		QAction *copyLink = pageAction(QWebEnginePage::CopyLinkToClipboard);
		copyLink->setIcon(qutim_sdk_0_3::Icon("edit-copy"));
		menu->addAction(openLink);
		menu->addAction(copyLink);
		connect(openLink, SIGNAL(triggered()), SLOT(openLinkFromContextMenu()));
	}
	if(!request->selectedText().isEmpty()) {
		QAction *copy = pageAction(QWebEnginePage::Copy);
		copy->setIcon(qutim_sdk_0_3::Icon("edit-copy"));
		QAction *quote = new QAction(tr("&Quote"), menu);
		quote->setIcon(qutim_sdk_0_3::Icon("insert-text"));
		QAction *search = new QAction(tr("&Search in %1").arg(m_searcher), menu);
		search->setIcon(qutim_sdk_0_3::Icon("edit-find"));
		if (!linkUnderMouse)
			menu->addAction(copy);
		menu->addAction(quote);
		if(!linkUnderMouse) {
			menu->addAction(search);
			connect(search, SIGNAL(triggered()), SLOT(searchSelectedText()));
		}
		connect(quote, SIGNAL(triggered()), SLOT(insertQuoteText()));
	}

	menu->addSeparator();

	QAction *inspect = new QAction(qutim_sdk_0_3::Icon("document-properties"), tr("Inspect"), menu);
	menu->addAction(inspect);
	connect(inspect, SIGNAL(triggered()), SLOT(inspectElement()));
	connect(menu, SIGNAL(destroyed(QObject*)), SLOT(setPrevFocus(QObject*)));
	menu->popup(mapToGlobal(point));
}

void WebViewWidget::inspectElement()
{
	QWebEnginePage *chatPage = page();
	if (!chatPage->devToolsPage()) {
		QWebEngineView *devTools = new QWebEngineView(chatPage->profile());
		devTools->setAttribute(Qt::WA_DeleteOnClose, true);
		devTools->setWindowTitle(tr("Inspect"));
		devTools->resize(800, 600);
		chatPage->setDevToolsPage(devTools->page());
		devTools->show();
	}
	chatPage->triggerAction(QWebEnginePage::InspectElement);
}

void WebViewWidget::insertQuoteText()
{
	QString text;
	const QString newLine = QLatin1String("\n> ");
	QString quote = m_controller->quote();
	quote.prepend("> ");
	for (int i = 0; i < quote.size(); ++i) {
		if (quote[i] == QLatin1Char('\n') || quote[i].unicode() == QChar::ParagraphSeparator)
			text += newLine;
		else
			text += quote[i];
	}
	text.reserve(text.size() + quote.size() * 1.2);
	text += QLatin1Char('\n');
	m_controller->appendText(text);
}

void WebViewWidget::setPrevFocus(QObject *)
{
	if (!m_controller)
		return;
	QObject *form = ServiceManager::getByName("ChatForm");
	QObject *obj = 0;
	if (QMetaObject::invokeMethod(form, "textEdit", Q_RETURN_ARG(QObject*, obj),
									  Q_ARG(qutim_sdk_0_3::ChatSession*, m_controller->getSession())) && obj) {
	if (QWidget *widget = qobject_cast<QWidget*>(obj))
	widget->setFocus();
	}
}

void WebViewWidget::searchSelectedText()
{
	QString text = m_controller->quote().trimmed();
	QUrl url;
	QUrlQuery query;
	if (m_searcher == "Yandex") {
		url = QUrl(m_yandexDomainDefine.getDomain() + "search/");
		query.addQueryItem(QStringLiteral("text"), text);
	} else if (m_searcher == "Google") {
		url = QUrl(QStringLiteral("https://www.google.com/search"));
		query.addQueryItem(QStringLiteral("q"), text);
	} else {
		return;
	}
	url.setQuery(query);
	QDesktopServices::openUrl(url);
}

void WebViewWidget::openLinkFromContextMenu()
{
	QDesktopServices::openUrl(m_urlForOpen);
	m_urlForOpen.clear();
}

void WebViewWidget::openChat()
{
	QString url = m_urlForOpen.toString();
	url.replace("mailto:", "xmpp:");
	url += "?message";
	m_urlForOpen.setUrl(url);
	QDesktopServices::openUrl(m_urlForOpen);
	m_urlForOpen.clear();
}

void WebViewWidget::openConference()
{
	QString url = m_urlForOpen.toString();
	url.replace("mailto:", "xmpp:");
	url += "?join";
	m_urlForOpen.setUrl(url);
	QDesktopServices::openUrl(m_urlForOpen);
	m_urlForOpen.clear();
}

} // namespace Adium

