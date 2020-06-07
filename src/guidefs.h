// ETH32nix - an Enemy Territory cheat for Linux
// Copyright (c) 2009 eth32 team
// www.nixcoders.org

#pragma once

#include "CAimbot.h"

#define SETFLOAT(x)			((int)(x * 1000))		// only keeping up to 0.001
#define GETFLOAT(x)			(x / 1000.f)

// Control definition reference: (update this if you add new control types)

// Type					Arg0			Arg1			Arg2			Arg3			Arg4
// CTRL_CHECKBOX		bool *target	N/A				N/A				N/A				N/A
// CTRL_INTDISPLAY		int *target		N/A				N/A				N/A				N/A
// CTRL_INTSLIDER		int	min			int	max			int *target		N/A				N/A
// CTRL_FLOATSLIDER		float min		float max		float *target	N/A				N/A
// CTRL_COLORPICKER		BYTE *color		N/A				N/A				N/A				N/A

// NOTE: cast arg0-arg4 to int if used, use SETFLOAT/GETFLOAT for floats

static const windef_t windows[] =
{
	{
		"Status",			// title
		WIN_STATUS,			// type
		GUI_MAINVIEW,			// view
		5, 422, 150, 53,		// x, y, w, h
		0,				// num controls
	},
	{
		"Weapon",			// title
		WIN_WEAPON,			// type
		GUI_MAINVIEW,			// view
		6, 365, 150, 53,		// x, y, w, h
		0,				// num controls
	},
	{
		"mainchat",			// title
		WIN_CHAT,			// type
		GUI_MAINVIEW,			// view
		289, 379, 350, 100,		// x, y, w, h
		0,				// num controls
	},
	{
		"Spectators",			// title
  		WIN_SPECTATOR,			// type
		GUI_MAINVIEW,			// view
		50, 193, 100, 80,		// x, y, w, h
		0,				// num controls
	},
	{
		"Respawn",			// title
		WIN_RESPAWN,			// type
		GUI_MAINVIEW,			// view
		280, 5, 40, 20,			// x, y, w, h
		0,				// num controls
	},
	{
		"Radar",			// title
		WIN_RADAR,			// type
		GUI_MAINVIEW,			// view
		515, 5, 120, 120,		// x, y, w, h
		0,				// num controls
	},
	{
		"Cameras",			// title
		WIN_CAMERA,			// type
		GUI_MAINVIEW,			// view
		5, 5, 5, 5,			// x, y, w, h
		0,				// num controls
	},
	{
		"Weapon Config",		// title
		WIN_WEAPCONFIG,			// type
		GUI_MENUVIEW,			// view
		157, 55, 150, 225,		// x, y, w, h
		0,				// num controls
	},
	{
		"Custom Hitbox",		// title
		WIN_HITBOX,			// type
		GUI_MENUVIEW,			// view
		395, 59, 200, 195,		// x, y, w, h
		0,				// num controls
	},
	{
		"Camera Settings",		//title
		WIN_CAMCONFIG,			//type
		GUI_MENUVIEW,			//view
		218, 48, 150, 150,		// x, y, w, h
		0,				// num controls
	},
	{
		"Banner",
		WIN_BANNER,			// type
		GUI_MAINVIEW,			// view
		20, 20, 1, 1,			// x, y, w, h
		0,				// num controls
	},
	/** *******************************************************************
					AIMBOT
	******************************************************************* **/
	{
		"Main",				// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view

		7, 58, 150, 280,		// x, y, w, h
		11,				// num controls
		{
			// Type				Label				X	Y		W		H		Arg0 ... Arg4
			{ CTRL_DROPBOX,		"Aim",				5,	5,		140,	23,		AIM_OFF, AIM_MAX-1,				(int)&eth32.settings.aimType,		(int)aimTypeText },
			{ CTRL_DROPBOX,		"Aim Mode",			5,	30,		140,	23,		0, 	AIMMODE_MAX-1,				(int)&eth32.settings.aimMode,		(int)aimModeText },
			{ CTRL_DROPBOX,		"Aim Priority",		5,	55,		140,	23,		0, 	AP_MAX-1, 					(int)&eth32.settings.headbody,		(int)priorityTypeText },
			{ CTRL_DROPBOX,		"Aim Protect",		5,	80,		140,	23,		0, 	PROTECT_MAX-1,				(int)&eth32.settings.aimprotect,	(int)aimprotectText },
			{ CTRL_DROPBOX,		"Target Sort",		5,	105,	140,	23,		SORT_OFF, SORT_MAX-1,			(int)&eth32.settings.aimSort,		(int)sortTypeText },
			{ CTRL_CHECKBOX,	"Autofire",			5,	135,	140,	8,										(int)&eth32.settings.autofire },
			{ CTRL_CHECKBOX,	"Validate Attack",	5,	155,	140,	8,										(int)&eth32.settings.atkValidate },
			{ CTRL_CHECKBOX,	"Auto Crouch",		5,	175,	140,	8,										(int)&eth32.settings.autoCrouch },
			{ CTRL_CHECKBOX,	"Target Lock",		5,	195,	140,	8,										(int)&eth32.settings.lockTarget },
			{ CTRL_CHECKBOX,	"Lock Sensitivity",	5,	215,	140,	8,										(int)&eth32.settings.lockSensitivity },
			{ CTRL_FLOATSLIDER, "Fov",				5,	235,	140,	20,		SETFLOAT(0.0), SETFLOAT(360.0), (int)&eth32.settings.fov },
		},
	},
	{
		"Humanaim",		// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		325, 60, 150, 200,		// x, y, w, h
		5,				// num controls
		{
			// Type				Label				X		Y		W		H		Arg0 ... Arg4
			{ CTRL_FLOATSLIDER,	"Humanaim Value",			5,	5,	140,	20,		SETFLOAT(0.0),	SETFLOAT(1.0),	(int)&eth32.settings.humanValue },
			{ CTRL_FLOATSLIDER,	"Divmax",					5,	30,	140,	20,		SETFLOAT(0),	SETFLOAT(10), 	(int)&eth32.settings.divmax },
			{ CTRL_FLOATSLIDER,	"Divmin",					5,	55,	140,	20,		SETFLOAT(0),	SETFLOAT(10), 	(int)&eth32.settings.divmin },
			{ CTRL_DROPBOX,		"Human Aim", 				5,	80,	140,	23,		0, 	HUMAN_AIM_MAX-1,	(int)&eth32.settings.humanAimType,(int)humanAimTypeText },
            { CTRL_CHECKBOX,	"Lock mouse while aiming",	5,	110,140,	8,				(int)&eth32.settings.lockMouse },
		},
	},
	/** *******************************************************************
					AIMBOT EXTRA
	******************************************************************* **/
	{
		"Corrections",		// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		7, 55, 150, 233,		// x, y, w, h
		9,				// num controls
		{
			{ CTRL_FLOATSLIDER,	"Head Hitbox Size",				5,	5, 		140,	20,		SETFLOAT(1), SETFLOAT(15), 	(int)&eth32.settings.headBoxSize },
			{ CTRL_FLOATSLIDER,	"Body Hitbox Size",	    		5,	30, 	140,	20,		SETFLOAT(1), SETFLOAT(40), 	(int)&eth32.settings.bodybox },
			{ CTRL_INTSLIDER,	"Aimpoints Head",     			5,	55, 	140,	20,		0,64, 						(int)&eth32.settings.pointshead },
			{ CTRL_INTSLIDER,	"Aimpoints Body",       		5,	80, 	140,	20,		0,64, 						(int)&eth32.settings.pointsbody },
			{ CTRL_CHECKBOX,	"Auto weapon Delay",   			5,	105,	140,	8,									(int)&eth32.settings.autoDelay },
			{ CTRL_INTSLIDER,	"Weapon Delay Close",       	5,	118, 	140,	20,		0,50, 						(int)&eth32.settings.delayClose },
			{ CTRL_INTSLIDER,	"Weapon Delay Med",       		5,	143, 	140,	20,		0,50, 						(int)&eth32.settings.delayMed },
			{ CTRL_INTSLIDER,	"Weapon Delay Far",       		5,	168, 	140,	20,		0,50, 						(int)&eth32.settings.delayFar },
			{ CTRL_CHECKBOX,	"Auto Z Vecs Calculation",		5,	193,	140,	8,									(int)&eth32.settings.autoVecZ },
		}
	},
	{
		"Aim Prediction",	// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		395, 59, 150, 290,		// x, y, w, h
		10,				// num controls
		{
			// Type				Label				 		X		Y		W		H		Arg0 ... Arg4
			{ CTRL_CHECKBOX,	"Preshoot",					5,		5,		140,	8,										(int)&eth32.settings.preShoot },
			{ CTRL_FLOATSLIDER,	"Preshoot Time",			5,		30,		140,	20,		SETFLOAT(0), SETFLOAT(300),		(int)&eth32.settings.preShootTime },
			{ CTRL_CHECKBOX,	"Preaim",					5,		60,		140,	8,										(int)&eth32.settings.preAim },
			{ CTRL_FLOATSLIDER,	"Preaim Time",		        5,		85,		140,	20,		SETFLOAT(0), SETFLOAT(1000),	(int)&eth32.settings.preAimTime },
			{ CTRL_CHECKBOX,	"Apply Target Prediction",	5,		115,	140,	8,										(int)&eth32.settings.predTargzor},
			{ CTRL_DROPBOX,		"Self Predict Type",		5,		140,	140,	23,		SPR_OFF, SPR_MAX-1, 			(int)&eth32.settings.predSelfType, (int)selfPredictText },
			{ CTRL_FLOATSLIDER,	"Self Predict",				5,		165,	140,	20,		SETFLOAT(-0.1), SETFLOAT(0.1), 	(int)&eth32.settings.predSelf },
			{ CTRL_DROPBOX,		"Target Predict Type",		5,		190,	140,	23,		PRED_OFF, PRED_MAX-1, 			(int)&eth32.settings.predictionType, (int)predictionTypeText },
			{ CTRL_FLOATSLIDER,	"Target Predict",	        5,     	215,	140,	20,		SETFLOAT(-0.1), SETFLOAT(0.1), 	(int)&eth32.settings.pred },
			{ CTRL_FLOATSLIDER,	"Bot Predict",	            5,      240,	140,	20,		SETFLOAT(-0.1), SETFLOAT(0.1), 	(int)&eth32.settings.predbot },
		},
	},
	{
		"Hitbox & Trace",	// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		7, 58, 150, 180,		// x, y, w, h
		4,				// num controls
		{
			// Type				Label						X		Y		W		H		Arg0 ... Arg4
			{ CTRL_DROPBOX,		"Hitbox Style",				5,		5,		140,	23,		HITBOX_OFF, HITBOX_MAX-1,		(int)&eth32.settings.hitboxType,		(int)hitboxText },
			{ CTRL_DROPBOX,		"Trace Style",				5,		30,		140,	23,		TRACE_CENTER, TRACE_MAX-1, 		(int)&eth32.settings.traceType, (int)traceTypeText },
			{ CTRL_FLOATSLIDER,	"Animation Correction",		5,		60,		140,	20,		SETFLOAT(-10.0), SETFLOAT(10.0),(int)&eth32.settings.animCorrection },
			{ CTRL_FLOATSLIDER, "Dynamic Hitbox",			5,		80,		140,	20,     SETFLOAT(0.0), SETFLOAT(10.0),	(int)&eth32.settings.dynamicHitboxScale },	
		},
	},
	/** *******************************************************************
					VISUALS
	******************************************************************* **/
	{
		"Visuals",		// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		60, 46, 150, 290,		// x, y, w, h
		13,				// num controls
		{
			// Type				Label					X	Y	W	H		Arg0 ... Arg4
			{ CTRL_CHECKBOX,	"Wallhack",				5,		5,		140,	8,											(int)&eth32.settings.wallhack },
			{ CTRL_CHECKBOX,	"Do Distance Wallhack",	5,		25,		140,	8,											(int)&eth32.settings.doDistWallhack },
			{ CTRL_FLOATSLIDER,	"Wallhack Distance",	5,		45,		140,	20,		SETFLOAT(0.0), SETFLOAT(10000),		(int)&eth32.settings.wallhackDistance },
			{ CTRL_CHECKBOX,	"Draw Blackout",		5,		75,		140,	8,											(int)&eth32.settings.blackout },
			{ CTRL_CHECKBOX,	"Weapon Zoom",			5,		95,		140,	8,											(int)&eth32.settings.weaponZoom },
			{ CTRL_FLOATSLIDER,	"Scoped Turning",		5,		115,	140,	20,		SETFLOAT(0.1), SETFLOAT(1.0), 		(int)&eth32.settings.scopedTurnSpeed },
			{ CTRL_INTSLIDER,	"Smoke Visibility",		5,		135,	140,	20,		0, 100, 							(int)&eth32.settings.smoketrnsp },
			{ CTRL_FLOATSLIDER,	"Radar Range",			5,		155,	140,	20,		SETFLOAT(100.0), SETFLOAT(10000.0),	(int)&eth32.settings.radarRange },
			{ CTRL_CHECKBOX,	"Disguised ESP",		5,		185,	140,	8,											(int)&eth32.settings.drawDisguised },
			{ CTRL_CHECKBOX,	"Missile Blast Light",	5,		205,	140,	8,											(int)&eth32.settings.grenadeDlight },
			{ CTRL_CHECKBOX,	"Mortar Blast Light",	5,		225,	140,	8,											(int)&eth32.settings.mortarDlight },
			{ CTRL_CHECKBOX,	"Mortar Trace and Esp",	5,		245,	140,	8,											(int)&eth32.settings.mortarTrace },
			{ CTRL_CHECKBOX,	"Arty Markers",			5,		265,	140,	8,											(int)&eth32.settings.artyMarkers },
		},
	},
	{
		"Hitbox Display",	// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		377, 151, 150, 110,		// x, y, w, h
		5,				// num controls
		{
			// Type				Label				X	Y	W	H		Arg0 ... Arg4
			{ CTRL_CHECKBOX,	"Head",				5,		5,		140,	8,						(int)&eth32.settings.drawHeadHitbox },
			{ CTRL_CHECKBOX,	"Head Axes",		5,		25,		140,	8,						(int)&eth32.settings.drawHeadAxes },
			{ CTRL_CHECKBOX,	"Body",				5,		45,		140,	8,						(int)&eth32.settings.drawBodyHitbox },
			{ CTRL_CHECKBOX,	"Bullet Rail",		5,		65,		140,	8,						(int)&eth32.settings.drawBulletRail },
			{ CTRL_CHECKBOX,	"Rail Wallhack",	5,		85,		140,	8,						(int)&eth32.settings.railWallhack }
		},
	},
	{
		"Colors",			// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		109, 208, 260, 90,		// x, y, w, h
		1,				// num controls
		{
			// Type			Label				X	Y	W	H		Arg0 ... Arg4
			{ CTRL_COLORPICKER,    "Picker",	5,	5,	250,	65 },
		},
	},
	/** *******************************************************************
					MISC
	******************************************************************* **/
	{
		"Misc",				// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		119, 47, 150, 250,		// x, y, w, h
		11,				// num controls
		{
			// Type				Label				X		Y		W		H		Arg0 ... Arg4
			{ CTRL_CHECKBOX,	"Players Names",	5,		5,		140,	8,								(int)&eth32.settings.espName },
			{ CTRL_CHECKBOX,	"Spec list", 		5,	 	25, 	140, 	8,								(int)&eth32.settings.getSpeclist },
			{ CTRL_CHECKBOX,	"Respawn Timers",	5,		45,		140,	8,								(int)&eth32.settings.respawnTimers },
			{ CTRL_CHECKBOX,	"Auto Tapout",		5,		65,		140,	8,								(int)&eth32.settings.autoTapout },
			{ CTRL_CHECKBOX,	"Remove Foliage",	5,		85,		140,	8,								(int)&eth32.settings.removeFoliage },
			{ CTRL_CHECKBOX,	"Grenade Trace",	5,		105,	140,	8,								(int)&eth32.settings.grenadeTracer },
			{ CTRL_CHECKBOX,	"Riflenade Trace",	5,		125,	140,	8,								(int)&eth32.settings.rifleTracer},
			{ CTRL_DROPBOX,		"PB Screenshot",	5,		145,	140,	23,     	0,   PB_SS_MAX-1,	(int)&eth32.settings.pbScreenShot,	(int)pbssText },
			{ CTRL_CHECKBOX,	"Auto Vote",		5,		170,	140,	8,								(int)&eth32.settings.autoVote },
			{ CTRL_CHECKBOX,	"Anti Teamkill",	5,		195,	140,	8,								(int)&eth32.settings.antiTk },
			{ CTRL_CHECKBOX,	"Timenudge Hack",	5,		210,	140,	8,					(int)&eth32.settings.nudgeHack },
		},
	},
	{
		"Gui Options",			// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		319, 47, 150, 100,		// x, y, w, h
		4,				// num controls
		{
			// Type				Label				X	Y	W		H		Arg0 ... Arg4
			{ CTRL_CHECKBOX,	"Banner",			5,	5,	140,	8,										(int)&eth32.settings.guiBanner },
			{ CTRL_FLOATSLIDER,	"Banner size",		5,	25,	140,	20,		SETFLOAT(0.0), SETFLOAT(3.0),	(int)&eth32.settings.BannerScale },
			{ CTRL_CHECKBOX,	"Original hud",		5,	55,	140,	8,										(int)&eth32.settings.guiOriginal },
			{ CTRL_CHECKBOX,	"Chatbox",			5,	75,	140,	8,										(int)&eth32.settings.guiChat },
		},
	},
	{
		"Sound",			// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		119, 280, 150, 90,		// x, y, w, h
		4,				// num controls
		{
			// Type				Label				X	Y	W	H		Arg0 ... Arg4
			{ CTRL_CHECKBOX, 	"Hit Sound",		5,	5,	140,	8,						(int)&eth32.settings.hitsounds },
			{ CTRL_CHECKBOX,	"Custom Sounds",	5,	25,	140,	8,						(int)&eth32.settings.customSounds },
			{ CTRL_CHECKBOX,	"Pure only",		5,	45,	140,	8,						(int)&eth32.settings.pureSounds },
			{ CTRL_CHECKBOX,	"HQ sounds",		5,	65,	140,	8,						(int)&eth32.settings.hqSounds },
		},
	},
	{
		"Namestealer",			// title
		WIN_STANDARD,			// type
		GUI_MENUVIEW,			// view
		325, 58, 150, 145,		// x, y, w, h
		5,				// num controls
		{
			// Type					Label				X	Y	W	H		Arg0 ... Arg4
			{ CTRL_CHECKBOX,	"Name Steal",			5,	5,	140,	8,							(int)&eth32.settings.doNamesteal },
			{ CTRL_INTSLIDER,	"Delay",				5,	25,	140,	20,		0, 20000,			(int)&eth32.settings.NamestealDelay },
			{ CTRL_INTSLIDER,	"Init Grace",			5,	45,	140,	20,		0, 20000,			(int)&eth32.settings.NamestealGrace },
			{ CTRL_DROPBOX,		"Steal type",			5,	75,	140,	23,		0, NAMESTEAL_MAX-1,	(int)&eth32.settings.NamestealMode,	(int)namestealText },
			{ CTRL_CHECKBOX,	"PB Exact Namesteal",	5,	105,140,	8,							(int)&eth32.settings.nsSmartMode },
		},
	},
};

static const assetdef_t assetDefs[] =
{
//	  Key				Type					Target
	{ "titlecolor",		ASSET_VEC4,				(void*)eth32.guiAssets.titleColor },
	{ "textcolor1",		ASSET_VEC4,				(void*)eth32.guiAssets.textColor1 },
	{ "textcolor2",		ASSET_VEC4,				(void*)eth32.guiAssets.textColor2 },
	{ "titleleft",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.titleLeft },
	{ "titlecenter",	ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.titleCenter },
	{ "titleright",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.titleRight },
	{ "winleft",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.winLeft },
	{ "wintop",			ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.winTop },
	{ "wintopl",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.winTopLeft },
	{ "wincenter",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.winCenter },
	{ "txtinputl",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.txtinputLeft },
	{ "txtinputc",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.txtinputCenter },
	{ "txtinputr",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.txtinputRight },
	{ "btnl",			ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.btnLeft },
	{ "btnc",			ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.btnCenter },
	{ "btnr",			ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.btnRight },
	{ "btnsell",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.btnselLeft },
	{ "btnselc",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.btnselCenter },
	{ "btnselr",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.btnselRight },
	{ "check",			ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.check },
	{ "checkbox",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.checkBox },
	{ "mouse",			ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.mousePtr },
	{ "dropboxarrow",	ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.dropboxArrow },
	{ "scrollarrow",	ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.scrollbarArrow },
	{ "scrollbtn",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.scrollbarBtn },
	{ "scrolltrack",	ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.scrollbarTrack },
	{ "sliderbtn",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.sliderBtn },
	{ "slidertrack",	ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.sliderTrack },
	{ "camcorner",		ASSET_SHADERNOMIP,		(void*)&eth32.guiAssets.camCorner },
};
