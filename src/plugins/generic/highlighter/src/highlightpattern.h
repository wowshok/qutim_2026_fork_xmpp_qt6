/****************************************************************************
**
** qutIM - instant messenger
**
** Copyright © 2011 Alexander Kazarin <boiler@co.ru>
** Copyright © 2011 Aleksey Sidorov <gorthauer87@yandex.ru>
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

#ifndef HIGHLIGHTPATTERN_H
#define HIGHLIGHTPATTERN_H

#include <QCoreApplication>
#include <QRegularExpression>
#include <QString>

// A user-defined highlight rule. Qt 5 versions stored a QRegExp in the
// config; Qt 6 has no QRegExp, so the rule is kept as pattern + syntax and
// compiled to a QRegularExpression.
class HighlightPattern
{
	Q_DECLARE_TR_FUNCTIONS(HighlightPattern)
public:
	enum Syntax
	{
		RegExp,      // Perl-like regular expression
		Wildcard,    // Shell-like: * and ?
		FixedString
	};

	HighlightPattern() = default;
	HighlightPattern(const QString &pattern, Syntax syntax) : m_pattern(pattern), m_syntax(syntax) {}

	QString pattern() const { return m_pattern; }
	Syntax syntax() const { return m_syntax; }
	bool isEmpty() const { return m_pattern.isEmpty(); }
	bool isValid() const { return !isEmpty() && toRegularExpression().isValid(); }

	QRegularExpression toRegularExpression() const
	{
		QString expression;
		switch (m_syntax) {
		case RegExp:
			expression = m_pattern;
			break;
		case Wildcard:
			expression = QRegularExpression::wildcardToRegularExpression(
						m_pattern, QRegularExpression::UnanchoredWildcardConversion);
			break;
		case FixedString:
			expression = QRegularExpression::escape(m_pattern);
			break;
		}
		return QRegularExpression(expression, QRegularExpression::UseUnicodePropertiesOption);
	}

	static QString syntaxTitle(Syntax syntax)
	{
		switch (syntax) {
		case RegExp:
			return tr("Perl-like");
		case Wildcard:
			return tr("Shell-like");
		case FixedString:
			return tr("Fixed string");
		}
		return tr("Perl-like");
	}

	static QList<Syntax> allSyntaxes() { return { RegExp, Wildcard, FixedString }; }

	// Config representation: a map with "pattern" and "syntax" keys
	static QString syntaxKey(Syntax syntax)
	{
		switch (syntax) {
		case Wildcard: return QStringLiteral("wildcard");
		case FixedString: return QStringLiteral("fixed");
		case RegExp: break;
		}
		return QStringLiteral("regexp");
	}
	static Syntax syntaxFromKey(const QString &key)
	{
		if (key == QLatin1String("wildcard"))
			return Wildcard;
		if (key == QLatin1String("fixed"))
			return FixedString;
		return RegExp;
	}

private:
	QString m_pattern;
	Syntax m_syntax = RegExp;
};

#endif // HIGHLIGHTPATTERN_H
