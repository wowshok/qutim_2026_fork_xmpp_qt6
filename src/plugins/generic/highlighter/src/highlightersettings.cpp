/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2012 Nicolay Izoderov <nico-izo@ya.ru>
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

#include "highlightersettings.h"
#include <qutim/config.h>

using namespace qutim_sdk_0_3;

HighlighterSettings::HighlighterSettings()
{
	ui.setupUi(this);
	lookForWidgetState(ui.enableAutoHighlights);

	connect(ui.regexp, SIGNAL(textChanged(const QString &)), this,  SLOT(validateInputRegexp()));
	connect(ui.regexptype, SIGNAL(currentIndexChanged(int)), this,  SLOT(validateInputRegexp()));


	for (HighlightPattern::Syntax syntax : HighlightPattern::allSyntaxes())
		ui.regexptype->addItem(HighlightPattern::syntaxTitle(syntax), int(syntax));
}

HighlighterSettings::~HighlighterSettings()
{
	clearItems();
}

void HighlighterSettings::loadImpl()
{
	clearItems();

	Config cfg;

	cfg.beginGroup(QLatin1String("highlighter"));
	ui.enableAutoHighlights->setChecked(cfg.value("enableAutoHighlights", true));

	int count = cfg.beginArray(QLatin1String("regexps"));
	for (int i = 0; i < count; i++) {
		cfg.setArrayIndex(i);
		const HighlightPattern pattern(cfg.value(QLatin1String("pattern"), QString()),
									   HighlightPattern::syntaxFromKey(cfg.value(QLatin1String("syntax"), QString())));
		if (pattern.isEmpty())
			continue;

		HighlighterItemList *item = new HighlighterItemList(pattern, ui.regexpsList);
		connect(item, SIGNAL(buttonClicked()), this, SLOT(onRemoveButtonClicked()));
		m_items << item;
	}
	cfg.endArray();

	cfg.endGroup();
}

void HighlighterSettings::saveImpl()
{
	Config cfg;
	cfg.beginGroup(QLatin1String("highlighter"));
	cfg.setValue("enableAutoHighlights", ui.enableAutoHighlights->isChecked());

	int count = cfg.beginArray(QLatin1String("regexps"));
	for (int i = 0; i < m_items.size(); i++) {
		cfg.setArrayIndex(i);
		const HighlightPattern pattern = m_items.at(i)->pattern();
		cfg.setValue(QLatin1String("pattern"), pattern.pattern());
		cfg.setValue(QLatin1String("syntax"), HighlightPattern::syntaxKey(pattern.syntax()));
	}
	for (int i = count - 1; i >= m_items.size(); --i)
		cfg.remove(i);
	cfg.endArray();
	cfg.endGroup();
}

void HighlighterSettings::cancelImpl()
{
	loadImpl();
}

void HighlighterSettings::clearItems()
{
	ui.regexpsList->clear();
	for (QPointer<HighlighterItemList> item : m_items) {
		if (item)
			delete item.data();
	}
	m_items.clear();
}

void HighlighterSettings::onRemoveButtonClicked()
{
	HighlighterItemList *item = qobject_cast<HighlighterItemList*>(sender());
	Q_ASSERT(item);
	m_items.removeOne(item);
	delete item->item();
	setModified(true);
}

void HighlighterSettings::on_addRegexp_clicked()
{
	const HighlightPattern pattern = inputPattern();
	if (!pattern.isValid())
		return;

	HighlighterItemList *item = new HighlighterItemList(pattern, ui.regexpsList);
	connect(item, SIGNAL(buttonClicked()), this, SLOT(onRemoveButtonClicked()));
	m_items << item;

	setModified(true);
}

HighlightPattern HighlighterSettings::inputPattern() const
{
	const int index = ui.regexptype->currentIndex();
	return HighlightPattern(ui.regexp->text(),
							static_cast<HighlightPattern::Syntax>(ui.regexptype->itemData(index).toInt()));
}

void HighlighterSettings::validateInputRegexp()
{
	if (!inputPattern().isValid()) {
		//ui.regexp->setStyleSheet(QLatin1String("background: rgb(252, 190, 189);"));
		ui.addRegexp->setDisabled(true);
	} else {
		//ui.regexp->setStyleSheet(QLatin1String("background: #FFF;"));
		ui.addRegexp->setDisabled(false);
	}

}

void HighlighterSettings::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type())
	{
	case QEvent::LanguageChange:
		for(int i = ui.regexptype->count() - 1; i >= 0; --i) {
			const auto itemSyntax = static_cast<HighlightPattern::Syntax>(ui.regexptype->itemData(i).toInt());
			ui.regexptype->setItemText(i, HighlightPattern::syntaxTitle(itemSyntax));
		}
		break;
	default:
		break;
	}
}
