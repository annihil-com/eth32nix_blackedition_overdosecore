// ETH32 - an Enemy Territory cheat for linux
// Copyright (c) 2007 eth32 team
// www.nixcoders.org

#pragma once

#include "CWindow.h"

class CSpecWindow : public CWindow
{
	public:
		CSpecWindow(char *wlabel, int wx, int wy, int ww, int wh);
		void Display(void);
};
