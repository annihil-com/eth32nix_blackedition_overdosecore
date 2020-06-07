// ETH32nix - an Enemy Territory cheat for Linux
// Copyright (c) 2009 eth32 team
// www.nixcoders.org

#pragma once

#include <vector>

extern const char *camTypeText[];

class CCam
{
public:

	void drawCam(float x, float y, float width, float height, refdef_t *refDef, qboolean crosshair);
	void mortarCam(camInfo_t *ci);
	void followCam(camInfo_t *ci);
	void resetInfo(void);
	int nearestEntity(void);
	int CheckFov(vec3_t origin, camInfo_t *drop);
	void Point(camInfo_t *ci);
	void camForOrigin( vec3_t origin, float range, vec3_t angles );

	int			closestCam;
	int			framePlayers[MAX_CLIENTS]; //hack
	int			numFramePlayers;

	vec3_t		gentityOrigin;
	vec3_t		gentityAxis;
	vec3_t		dropAngles;
	int			enemyNum;
	int			targetClientNum;

private:

	refdef_t	camRefDef;
	char		str[128];
	

	//dropcams
	int			*camCount;
	vec3_t		adjustAngle;
	vec3_t		adjustOrigin;
	float		camDistance;
	int			distance;
};

extern CCam Cam;
extern bool drawingCam;
extern std::vector<refEntity_t> refEntities;
