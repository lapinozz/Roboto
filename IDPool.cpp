#include "IDPool.h"
#include "Utility.h"

IDPool::IDPool(unsigned int t_preCalcCount, unsigned int t_calcOnUnderflow) : m_calcOnUnderflow(t_calcOnUnderflow)
{
	for (unsigned int i = 1; i < t_preCalcCount; i++)
		pushID(m_curr++);

}

unsigned int IDPool::pullID()
{
	if (m_openPool.empty()) {
		for (unsigned int i = 1; i < m_calcOnUnderflow; i++)
			pushID(m_curr++);
	}

	unsigned int _t = m_openPool.front();
	m_openPool.pop();
	return _t;
}

void IDPool::pushID(unsigned int t_) {
	m_openPool.emplace(t_);
}
