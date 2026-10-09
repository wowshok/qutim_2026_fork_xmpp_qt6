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

#ifndef QTMULTIMEDIASOUNDBACKEND_H
#define QTMULTIMEDIASOUNDBACKEND_H

#include <qutim/sound.h>
#include <QStringList>

class QMediaPlayer;

// Plays notification sounds through Qt Multimedia, replacing the Phonon backend
class QtMultimediaSoundBackend : public qutim_sdk_0_3::SoundBackend
{
	Q_OBJECT
public:
	QtMultimediaSoundBackend();
	void playSound(const QString &filename) override;
	QStringList supportedFormats() override;

private:
	void release(QMediaPlayer *player);
};

#endif // QTMULTIMEDIASOUNDBACKEND_H
