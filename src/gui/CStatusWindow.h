// ETH32 - an Enemy Territory cheat for linux
// Copyright (c) 2007 eth32 team
// www.nixcoders.org

#pragma once

#include "CWindow.h"

class CStatusWindow : public CWindow
{
public:
	CStatusWindow(char *wlabel, int wx, int wy, int ww, int wh);
	void Display(void);
private:
	int statbarSize;
};
