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

#include "localizationsettings.h"
#include "ui_localizationsettings.h"
#include <qutim/configbase.h>
#include <QStringBuilder>
#include <QDebug>
#include <QDir>
#include <QTranslator>
#include <QLabel>
#include <qutim/icon.h>
#include <qutim/thememanager.h>

using namespace qutim_sdk_0_3;

namespace Core
{

LocalizationSettings::LocalizationSettings() :
	m_ui(new Ui::LocalizationSettings)
{
	m_ui->setupUi(this);
	// The open windows keep their texts, only new ones come up in the new language
	QLabel *restartLabel = new QLabel(tr("To take effect you must restart qutIM"), this);
	restartLabel->setWordWrap(true);
	m_ui->verticalLayout->addWidget(restartLabel);
	QStringList langs = ThemeManager::list("languages");
	Icon icon("preferences-desktop-locale");
	if (!langs.contains(QLatin1String("en_EN"))) {
		QListWidgetItem *item = new QListWidgetItem(icon, QLatin1String("English"),
													m_ui->languagesList);
		m_items.insert(QLatin1String("en_EN"), item);
		item->setData(Qt::UserRole, QLatin1String("en_EN"));
	}
	foreach (const QString &lang, langs) {
		QString text;
		QDir dir = ThemeManager::path("languages", lang);
		QTranslator translator;
		if (dir.exists("core.qm") && translator.load(QLatin1String("core.qm"), dir.absolutePath())) {
			QString language = translator.translate("Localization", "<Language>", "Localized language name");
			// Some translations copied the placeholders instead of translating them
			if (!language.isEmpty() && !language.startsWith(QLatin1Char('<'))) {
				text = language;
				QString country = translator.translate("Localization", "<Country>",
													   "Localized country name, empty if localization is country-independent");
				if (!country.isEmpty() && !country.startsWith(QLatin1Char('<'))) {
					text += QLatin1String(" (");
					text += country;
					text += QLatin1String(")");
				}
			}
		}
		if (text.isEmpty()) {
			QLocale locale(lang);
			text = locale.nativeLanguageName();
			if (!text.isEmpty())
				text[0] = text[0].toUpper();
			if (lang.contains(QLatin1Char('_')) && !locale.nativeTerritoryName().isEmpty())
				text += QLatin1String(" (") % locale.nativeTerritoryName() % QLatin1String(")");
			if (text.isEmpty())
				text = lang;
		}
		QListWidgetItem *item = new QListWidgetItem(icon, text, m_ui->languagesList);
		m_items.insert(lang, item);
		item->setData(Qt::UserRole, lang);
	}
	m_ui->languagesList->sortItems();
	// Empty value: follow the system (LANGUAGE/LANG) on every start
	QListWidgetItem *systemItem = new QListWidgetItem(icon, tr("System language"));
	systemItem->setData(Qt::UserRole, QString());
	m_ui->languagesList->insertItem(0, systemItem);
	m_items.insert(QString(), systemItem);
	listenChildrenStates();
}

LocalizationSettings::~LocalizationSettings()
{
	delete m_ui;
}

void LocalizationSettings::loadImpl()
{
	Config cfg;
	cfg.beginGroup("localization");
	QString lang = cfg.value("lang", QString());
	m_ui->languagesList->setCurrentItem(m_items.value(lang, m_items.value(QString())));
}

void LocalizationSettings::saveImpl()
{
	if (QListWidgetItem *item = m_ui->languagesList->currentItem()) {
		Config cfg;
		cfg.beginGroup("localization");
		cfg.setValue("lang", item->data(Qt::UserRole));
	}
}

void LocalizationSettings::cancelImpl()
{
	loadImpl();
}

void LocalizationSettings::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		m_ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
}

