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

#include "modulemanagerimpl.h"
#include <qutim/jsonfile.h>
#include <QVariant>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <qutim/protocol.h>
#include "profiledialog.h"
#include "profilecreationwizard.h"
#include <qutim/config.h>
#include <qutim/notification.h>
#include <qutim/systemintegration.h>
#include <QApplication>
#include <qutim/debug.h>
#include <qutim/systeminfo.h>

namespace Core
{
ModuleManagerImpl::ModuleManagerImpl()
{
	ModuleManager::loadPlugins();
	Config config = ProfileDialog::profilesInfo();
#ifdef QUTIM_SINGLE_PROFILE
	bool singleProfile = true;
#else
	bool singleProfile = false;
#endif
	singleProfile = config.value("singleProfile", singleProfile);

	QWizard *wizard = 0;
	if (singleProfile) {
		if (!config.hasChildGroup("profile")) {
			wizard = new ProfileCreationWizard(this, QString(), QString(), true);
		} else {
			config.beginGroup("profile");
			// Usage statistics were sent to qutim.org, which is gone

			bool systemProfiles = false;
			ProfileDialog::profilesConfigPath(&systemProfiles);
			if(ProfileDialog::acceptProfileInfo(config, QString(), !systemProfiles)) {
				QTimer::singleShot(0, this, SLOT(initExtensions()));
			} else {
				qWarning() << "Can't login";
				QDialog *dialog = new ProfileDialog(config, this);
				SystemIntegration::show(dialog);
			}
			config.endGroup();
		}
	} else {
		QDialog *dialog = new ProfileDialog(config, this);
		SystemIntegration::show(dialog);
	}
	if (wizard) {
		wizard->setAttribute(Qt::WA_DeleteOnClose, true);
		wizard->setAttribute(Qt::WA_QuitOnClose, false);
		SystemIntegration::show(wizard);
	}
}

ExtensionInfoList ModuleManagerImpl::coreExtensions() const
{
	return ExtensionInfoList();
}

void ModuleManagerImpl::initExtensions()
{
	QString path = SystemInfo::getPath(SystemInfo::SystemShareDir);
	path += QLatin1String("/ca-certs/*.pem");
	QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
	sslConfig.addCaCertificates(path, QSsl::Pem, QSslCertificate::PatternSyntax::Wildcard);
	path.chop(3);
	path += QLatin1String("crt");
	sslConfig.addCaCertificates(path, QSsl::Pem, QSslCertificate::PatternSyntax::Wildcard);
	QSslConfiguration::setDefaultConfiguration(sslConfig);

	ModuleManager::initExtensions();

	NotificationRequest request(Notification::AppStartup);
	request.send();
}
}

