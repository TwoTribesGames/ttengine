#pragma once
#ifndef GWEN_WINDOW_PROVIDER_H
#define GWEN_WINDOW_PROVIDER_H

namespace Gwen
{
	class WindowProvider
	{
		public:
			virtual ~WindowProvider() { } // Two Tribes addition
			virtual void* GetWindow() = 0;
	};

}

#endif
