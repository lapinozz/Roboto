#pragma once
#include "stdafx.h"

class IDPool {
public:
	IDPool(unsigned int t_preCalcCount, unsigned int t_calcOnUnderflow);

	unsigned int pullID();
	void pushID(unsigned int t_);

private:
	std::queue<unsigned int> m_openPool;
	unsigned int m_calcOnUnderflow;
	unsigned int m_curr = 1;
};
