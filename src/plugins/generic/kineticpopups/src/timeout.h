#pragma once
#include <QObject>
#include <qutim/config.h>

class Timeout : public QObject
{
	Q_OBJECT
	// Read from the config on every access: each new popup picks up the current
	// value, so QML never needs a change notification
	Q_PROPERTY(int timeout READ timeout CONSTANT)

public:
	Timeout(){}
	~Timeout(){}

	int timeout() const
	{
		qutim_sdk_0_3::Config cfg(QStringLiteral("behavior"));
		cfg.beginGroup(QStringLiteral("popup"));
		// The settings page stores seconds as a double (e.g. 2.5)
		int timeout = qRound(cfg.value(QStringLiteral("timeout"), 5.0) * 1000);
		cfg.endGroup();
		return timeout;
	}

};
