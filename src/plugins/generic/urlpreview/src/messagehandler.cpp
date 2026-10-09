/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Alexander Kazarin <boiler@co.ru>
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


#include "messagehandler.h"

#include <qutim/debug.h>
#include <qutim/config.h>
#include <qutim/chatsession.h>
#include <qutim/utils.h>
#include <qutim/json.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextDocument>
#include <QStringBuilder>

#include <QUrlQuery>

namespace UrlPreview {

using namespace qutim_sdk_0_3;

UrlHandler::UrlHandler() :
	m_netman(new QNetworkAccessManager(this))
{
	connect(m_netman, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
			SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*))
			);
	connect(m_netman, SIGNAL(finished(QNetworkReply*)),
			SLOT(netmanFinished(QNetworkReply*))
			);
	connect(m_netman, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
			SLOT(netmanSslErrors(QNetworkReply*,QList<QSslError>))
			);

	Config cfg;
	cfg.beginGroup("urlPreview");
	m_flags = cfg.value(QLatin1String("flags"), PreviewImages | PreviewYoutube);
	m_maxImageHeight = cfg.value(QLatin1String("maxHeight"), (quint64)600);
	m_maxImageWidth = cfg.value(QLatin1String("maxWidth"), (quint64)800);

	m_maxFileSize = cfg.value(QLatin1String("maxFileSize"), (quint64)100000);
	m_template = "<br><b>" % tr("URL Preview") % "</b>: <i>%TYPE%, %SIZE% " % tr("bytes") % "</i><br>";
	m_imageTemplate = "<img class=\"urlpreview-image\" src=\"%URL%\" style=\"display: none;\" "
								 "onload=\"if (this.width>%MAXW%) this.style.maxWidth='%MAXW%px';"
								 "if (this.height>%MAXH%) { this.style.maxWidth=''; this.style.maxHeight='%MAXH%px'; } "
								 "this.style.display=''; if(nearBottom() || this.parentNode.getAttribute('data-wasnearbottom') == 'true' ){scrollToBottom();} \"><br>";
	m_youtubeTemplate =	"<img src=\"https://img.youtube.com/vi/%YTID%/1.jpg\">"
								   "<img src=\"https://img.youtube.com/vi/%YTID%/2.jpg\">"
								   "<img onload=\"if(nearBottom() || this.parentNode.getAttribute('data-wasnearbottom') == 'true'){scrollToBottom();}\" src=\"https://img.youtube.com/vi/%YTID%/3.jpg\"><br>";

	m_html5AudioTemplate = "<audio controls=\"controls\" preload=\"none\"><source src=\"%AUDIOURL%\" type=\"%FILETYPE%\"/>" % tr("Something went wrong.") % "</audio>";

	m_html5VideoTemplate = "<video controls=\"controls\" preload=\"none\"><source src=\"%VIDEOURL%\" type=\"%VIDEOTYPE%\" />" % tr("Something went wrong.") % "</video>";
	m_enableYoutubePreview = cfg.value("youtubePreview", true);
	m_enableImagesPreview = cfg.value("imagesPreview", true);
	m_enableHTML5Audio = cfg.value("HTML5Audio", true);
	m_enableHTML5Video = cfg.value("HTML5Video", true);
	m_exceptionList = cfg.value("exceptionList", QStringList());

	cfg.endGroup();
}

MessageHandlerAsyncResult UrlHandler::doHandle(Message &message)
{
	ChatSession *session = ChatLayer::get(message.chatUnit(), false);
	if (!session || !session->property("supportJavaScript").toBool()) {
		return makeAsyncResult(Accept, QString());
	}
	const QString originalHtml = message.html();
	QString html;
	foreach (const UrlParser::UrlToken &token,
			 UrlParser::tokenize(originalHtml, UrlParser::Html)) {
		if (token.url.isEmpty()) {
			html += token.text.toString();
		} else {
			static int uid = 1;
			QString link = token.url;
			checkLink(token.text, link, message.chatUnit(), uid++);
			html += link;
		}
	}
	message.setHtml(html);

	return makeAsyncResult(Accept, QString());
}

void UrlHandler::checkLink(QStringView originalLink, QString &link, ChatUnit *from, qint64 id)
{
	const char *entitiesIn[] = { "&quot;", "&gt;", "&lt;", "&amp;" };
	const char *entitiesOut[] = { "\"", ">", "<", "&" };
	const int entitiesCount = sizeof(entitiesIn) / sizeof(entitiesIn[0]);

	for (int i = 0; i < entitiesCount; ++i) {
		link.replace(QLatin1String(entitiesIn[i]),
					 QLatin1String(entitiesOut[i]),
					 Qt::CaseInsensitive);
	}

	foreach (QString key, m_exceptionList.value()) {
		// TODO: We have strange thing: after config update we have one empty string in list
		// Of course every string contains empty string. And here is workaround
		if(!key.isEmpty() && link.contains(key))
			return;
	}

	const QUrl url = QUrl::fromUserInput(link);

	if (m_flags & PreviewYoutube) {
		QString urlquery = QUrlQuery(url.query()).queryItemValue(QLatin1String("v"));
		const QString youtubeId = (url.host() == QLatin1String("youtube.com")
								   || url.host() == QLatin1String("www.youtube.com"))
								  ? (url.path().startsWith(QLatin1String("/v/"))
									 ? url.path().mid(3)
									 : (url.path().startsWith(QLatin1String("/embed/"))
										? url.path().mid(7)
										: urlquery))
								  : (url.host() == QLatin1String("youtu.be")
									 ? url.path().mid(1)
									 : QString());

		if (!youtubeId.isEmpty()) {
			QString html = m_template;
			html.replace("%TYPE%", tr("YouTube video"));
			html += m_youtubeTemplate;
			html.replace("%YTID%", QString::fromLatin1(QUrl::toPercentEncoding(youtubeId)));
			html.replace("%SIZE%", tr("Unknown"));
			html.prepend(originalLink.toString() + QLatin1String(" "));
			link = html;
			return;
		}
	}

	const QString uid = QString::number(id);

	QNetworkRequest request;
	request.setUrl(QUrl(link));
	request.setRawHeader("Ranges", "bytes=0-0");
	QNetworkReply *reply = m_netman->head(request);
	reply->setProperty("uid", uid);
	reply->setProperty("unit", QVariant::fromValue<ChatUnit *>(from));

	ChatSession *session = ChatLayer::get(from);

	QVariant val;
	QMetaObject::invokeMethod(session, "evaluateJavaScript", Q_RETURN_ARG(QVariant, val), Q_ARG(QString, "nearBottom();"));
	qDebug() << val;

	link = QString::fromLatin1("%1 <span class='urlpreview' id='urlpreview%2' data-wasnearbottom='%3'></span> ")
		   .arg(originalLink.toString(), uid, val.toString());
}

void UrlHandler::netmanFinished(QNetworkReply *reply)
{
	reply->deleteLater();

	// Everything below ends up in HTML: escape what comes from the network
	const QString url = reply->url().toString(QUrl::FullyEncoded).toHtmlEscaped();
	// "text/html; charset=utf-8" -> "text/html"
	QString type = reply->header(QNetworkRequest::ContentTypeHeader).toString()
			.section(QLatin1Char(';'), 0, 0).trimmed();
	// We asked for one byte: the full size is after the slash in
	// "Content-Range: bytes 0-0/12345" (the old code took the first number, 0)
	quint64 size = 0;
	const QByteArray range = reply->rawHeader("Content-Range");
	const int slash = range.lastIndexOf('/');
	if (slash >= 0)
		size = range.mid(slash + 1).toULongLong();
	if (!size)
		size = reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();

	if (type.isNull())
		return;

	QString uid = reply->property("uid").toString();

	QString pstr;
	bool showPreviewHead = true;
	if (type.startsWith(QLatin1String("text/html"))) {
		showPreviewHead = false;
	}

	if (m_enableHTML5Audio &&
			(type == QLatin1String("audio/ogg")
			 || type == QLatin1String("audio/mpeg")
			 || type == QLatin1String("application/ogg")
			 || type == QLatin1String("audio/x-wav"))) {
		pstr = m_template;
		showPreviewHead = false;
		pstr.replace("%TYPE%", tr("HTML5 Audio"));
		pstr += m_html5AudioTemplate;
		if (type == QLatin1String("application/ogg")) {
			pstr.replace("%FILETYPE%", "audio/ogg");
		} else {
			pstr.replace("%FILETYPE%", type.toHtmlEscaped());
		}
		pstr.replace("%AUDIOURL%", url);
		pstr.replace("%SIZE%", QString::number(size));
	}

	if (m_enableHTML5Video &&
			(type == QLatin1String("video/webm")
			 || type == QLatin1String("video/ogg")
			 || type == QLatin1String("video/mp4"))) {
		pstr = m_template;
		showPreviewHead = false;
		pstr.replace("%TYPE%", tr("HTML5 Video"));
		pstr += m_html5VideoTemplate;
		pstr.replace("%VIDEOTYPE%", type.toHtmlEscaped());
		pstr.replace("%VIDEOURL%", url);
		pstr.replace("%SIZE%", QString::number(size));
	}

	if (showPreviewHead) {
		QString sizestr = size ? QString::number(size) : tr("Unknown");
		pstr = m_template;
		pstr.replace("%TYPE%", type.toHtmlEscaped());
		pstr.replace("%SIZE%", sizestr);
	}

	if (type.startsWith(QLatin1String("image/")) && 0 < size && size < m_maxFileSize && m_enableImagesPreview) {
		QString amsg = m_imageTemplate;
		amsg.replace("%URL%", url);
		amsg.replace("%UID%", uid);
		amsg.replace("%MAXW%", QString::number(m_maxImageWidth));
		amsg.replace("%MAXH%", QString::number(m_maxImageHeight));
		pstr += amsg;
	}

	updateData(reply->property("unit").value<ChatUnit *>(),
			   uid,
			   pstr);
}

void UrlHandler::updateData(ChatUnit *unit, const QString &uid, const QString &html)
{
	QString js = QLatin1String("urlpreview")
				 % uid
				 % QLatin1String(".innerHTML = \"")
				 % QString(html).replace("\"", "\\\"")
				 % QLatin1String("\";")
				 % QLatin1String("if(nearBottom() || urlpreview") % uid % QLatin1String(".getAttribute('data-wasnearbottom') == 'true'){scrollToBottom();}");
	ChatSession *session = ChatLayer::get(unit);

	QMetaObject::invokeMethod(session, "evaluateJavaScript", Q_ARG(QString, js));
}

void UrlHandler::authenticationRequired(QNetworkReply *, QAuthenticator *)
{

}

void UrlHandler::netmanSslErrors(QNetworkReply *, const QList<QSslError> &)
{

}

} // namespace UrlPreview
