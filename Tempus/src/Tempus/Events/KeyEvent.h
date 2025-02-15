#pragma once

#include "Event.h"

#include <sstream>

namespace Tempus {

	class TEMPUS_API KeyEvent : public Event
	{

	public:

		inline int GetKeyCode() const { return m_KeyCode; }

	protected:

		int m_KeyCode;

	};


}