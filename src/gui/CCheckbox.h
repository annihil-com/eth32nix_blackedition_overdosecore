// ETH32 - an Enemy Territory cheat for linux
// Copyright (c) 2007 eth32 team
// www.nixcoders.org

#pragma once

#include "CControl.h"

class CCheckBox : public CControl
{
protected:
	bool *target;
public:
	CCheckBox(char *clabel, int cx, int cy, int cw, int ch, bool *ctarget);
	void Display(void);
	virtual int ProcessMouse(int mx, int my, uint32 event, CControl **mhook, CControl **khook);
	void SetTarget(bool *ctarget);
};

