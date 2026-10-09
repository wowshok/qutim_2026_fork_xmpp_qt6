/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Nikita Belov <null@deltaz.org>
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

#include "highlighteritemlist.h"
#include <QTextDocument>
#include <qutim/icon.h>
#include <qutim/debug.h>

HighlighterItemList::HighlighterItemList(const HighlightPattern &pattern, QListWidget *regexList)
	: m_pattern(pattern)
{
	m_label = new QLabel(title(), this);
	QSizePolicy policy = m_label->sizePolicy();
	policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	m_label->setSizePolicy(policy);
	m_button = new QPushButton(tr("Remove"), this);
	m_button->setIcon(qutim_sdk_0_3::Icon(QLatin1String("list-remove")));
	connect(m_button, SIGNAL(clicked(bool)), this, SIGNAL(buttonClicked()));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(m_label);
	layout->addWidget(m_button);

	m_item = new QListWidgetItem(regexList);
	m_item->setData(Qt::SizeHintRole, sizeHint());
	regexList->setItemWidget(m_item, this);
}

HighlighterItemList::~HighlighterItemList()
{
	qDebug() << this;
}

HighlightPattern HighlighterItemList::pattern() const
{
	return m_pattern;
}

QString HighlighterItemList::title() const
{
	return QString::fromLatin1("%1<br>%2")
			.arg(m_pattern.pattern().toHtmlEscaped(), HighlightPattern::syntaxTitle(m_pattern.syntax()));
}

QListWidgetItem *HighlighterItemList::item()
{
	return m_item;
}

void HighlighterItemList::setItem(QListWidgetItem *item)
{
	m_item = item;
}

void HighlighterItemList::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type())
	{
	case QEvent::LanguageChange:
		m_label->setText(title());
		m_button->setText(tr("Remove"));
		break;
	default:
		break;
	}
}
