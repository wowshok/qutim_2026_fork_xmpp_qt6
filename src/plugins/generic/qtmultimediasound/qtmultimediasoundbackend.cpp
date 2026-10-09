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

#include "qtmultimediasoundbackend.h"
#include <qutim/debug.h>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QUrl>

QtMultimediaSoundBackend::QtMultimediaSoundBackend()
{
}

void QtMultimediaSoundBackend::playSound(const QString &filename)
{
	// One player per sound, so overlapping events do not cut each other off
	auto player = new QMediaPlayer(this);
	auto output = new QAudioOutput(player);
	player->setAudioOutput(output);

	connect(player, &QMediaPlayer::mediaStatusChanged, this, [this, player] (QMediaPlayer::MediaStatus status) {
		if (status == QMediaPlayer::EndOfMedia || status == QMediaPlayer::InvalidMedia)
			release(player);
	});
	connect(player, &QMediaPlayer::errorOccurred, this, [this, player] (QMediaPlayer::Error, const QString &error) {
		qWarning() << "Failed to play sound:" << error;
		release(player);
	});

	player->setSource(QUrl::fromLocalFile(filename));
	player->play();
}

QStringList QtMultimediaSoundBackend::supportedFormats()
{
	// Decoded by the FFmpeg backend of Qt Multimedia
	return QStringList() << QStringLiteral("wav") << QStringLiteral("ogg") << QStringLiteral("oga")
						 << QStringLiteral("mp3") << QStringLiteral("flac");
}

void QtMultimediaSoundBackend::release(QMediaPlayer *player)
{
	// Both signals may fire for the same player
	if (player->property("released").toBool())
		return;
	player->setProperty("released", true);
	player->deleteLater();
}
