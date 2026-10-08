#include "lastactivitycomparator.h"

namespace Core {

LastActivityComparator::LastActivityComparator()
{
	Q_UNUSED(QT_TRANSLATE_NOOP("ContactList", "Sort by contact's last activity"));
}

int LastActivityComparator::compare(qutim_sdk_0_3::Contact *a, qutim_sdk_0_3::Contact *b)
{
	qint64 diff = b->lastActivity().toSecsSinceEpoch() - a->lastActivity().toSecsSinceEpoch();
	int result = diff > 0 ? 1 : (diff < 0 ? -1 : 0);
	if (result)
		return result;
	return StatusComparator::compare(a, b);
}

void LastActivityComparator::doStartListen(qutim_sdk_0_3::Contact *contact)
{
	StatusComparator::doStartListen(contact);
	connect(contact, SIGNAL(lastActivityChanged(QDateTime,QDateTime)), SLOT(onContactChanged()));
}

} // namespace Core
