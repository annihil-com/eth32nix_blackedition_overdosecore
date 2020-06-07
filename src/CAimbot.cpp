// ETH32nix - an Enemy Territory cheat for Linux
// Copyright (c) 2009 eth32 team
// www.nixcoders.org

#include "eth32.h"
#include <stdlib.h>

#ifndef min
#define min( x, y ) ( ( ( x ) < ( y ) ) ? ( x ) : ( y ) )
#endif
#ifndef max
#define max( x, y ) ( ( ( x ) > ( y ) ) ? ( x ) : ( y ) )
#endif

CAimbot Aimbot;

extern xcommand_t orig_cc_start_attack;
extern xcommand_t orig_cc_end_attack;
extern xcommand_t orig_cc_StartCrouch;
extern xcommand_t orig_cc_EndCrouch;
extern void SnapVectorTowards( vec3_t v, vec3_t to );

// There !!!HAS TO BE!!! a corresponding string for each select type (except SORT_MAX)
const char *sortTypeText[SORT_MAX] =
{
	"Off",
	"Crosshair",
	"Distance",
	"Attacker",
	"K/D ratio",
	"Accuracy",
	"Threat",
};

const char *humanAimTypeText[HUMAN_AIM_MAX]=
{
	"Lucky",
	"Full",
};

const char *aimprotectText[PROTECT_MAX] =
{
	"Aimprotect Off",
	"Aimprotect Specs",
	"Aimprotect All",
};

const char *selfPredictText[SPR_MAX] =
{
	"Off",
	"Manual",
	"Ping",
	"L337",
};

const char *traceTypeText[TRACE_MAX] =
{
	"Center",
	"Contour",
	"Random Volume",
	"Random Surface",
	"Capsule Volume",
	"Capsule Surface",
};

const char *hitboxText[HITBOX_MAX] =
{
	"Off",
	"etMain",
	"etPro",
	"Custom",
};

const char *priorityTypeText[AP_MAX] =
{
	"Body Only",
	"Head Only",
	"Body - Head",
	"Head - Body",
	"Head priority",
};

const char *aimTypeText[AIM_MAX] =
{
	"Off",
	"On Fire",
	"On Button",
	"Always",
	"Trigger",
};

const char *aimModeText[AIMMODE_MAX] =
{
	"Aimbot Off",
	"Normal Aimbot",
	"Human Aimbot",
};

int SortCrosshair(const void *p1, const void *p2)
{
	const player_t *player1 = *(const player_t**)p1;
	const player_t *player2 = *(const player_t**)p2;

	if (Aimbot.CrosshairDistance(player1) < Aimbot.CrosshairDistance(player2))
	return -1;
	else
	return 1;
}

CAimbot::CAimbot(void)
{
	numFrameTargets = 0;
	target = NULL;
	lastTarget = NULL;
	lastTargetValid = false;
	atkBtn = NULL;
	firing = false;
	stopAutoTargets = false;
}

void CAimbot::SetSelf( int clientNum ){ this->self = &eth32.cg.players[clientNum]; }

void CAimbot::PreFrame(void)
{
	numFrameTargets = 0;
	target = NULL;
	lastTargetValid = false;

	if (lastTarget && (!eth32.settings.lockTarget || IS_DEAD(lastTarget->clientNum) || !IS_INFOVALID(lastTarget->clientNum)))
		lastTarget = NULL;
}

void CAimbot::PostFrame(void)
{
	// important default actions, assume neither are set unless explicitly told to do so (safer)
	// these actions are very fast anyway
	LockSensitivity(false);

	if (!this->attackPressed)
		Autofire(false);

	if (((eth32.settings.aimprotect == PROTECT_ALL) && (eth32.cg.nspectators || eth32.cg.limbo)) ||
		((eth32.settings.aimprotect == PROTECT_SPECS) && eth32.cg.nspectators) )
		return;

	// master aim mode
	if (eth32.settings.aimMode == AIMMODE_OFF)
		return;

	// No need for aimbot on those conditions
	if (IS_SPECTATOR(eth32.cg.snap->ps.clientNum) || IS_DEAD(eth32.cg.clientNum)){
		Autocrouch(false, true);
		return;
	}
	if (eth32.cg.currentWeapon->attribs & WA_USER_DEFINED){
		Aimbot.DoBulletBot();
	if (!target)
		Autocrouch(false, false);
	} else
		Autocrouch(false, false);
}

	// respawn so reset the aimbot
void CAimbot::Respawn(void)
{
	#ifdef ETH32_DEBUG
		Debug.Log("Aimbot Respawn()");
	#endif
		Autofire(false);
		LockSensitivity(false);
		Autocrouch(false, true);
		lastTargetValid = false;
		stopAutoTargets = false;
}

void CAimbot::AddTarget(player_t *player)
{
	if (player->friendly || player->invulnerable)
		return;

	// - solcrushr: fov check added before adding to our target array
	if ((eth32.cg.currentWeapon->attribs & WA_USER_DEFINED) && player->distance < eth32.cg.currentWeapon->range && CheckFov(player->orHead.origin)) {
		// sol: if last target is valid, then don't add him to normal targetlist
		//      we can check him independently
		if (lastTarget && player == lastTarget) {
			lastTargetValid = true;
			return;
		}

		frameTargets[numFrameTargets] = player;
		numFrameTargets++;
		// sol: weapdefs look safe, but to prevent overwriting array
		return;
	}
}

// these are callbacks for qsort, not part of aimbot object
int distance_test(const void *elm1, const void *elm2)
{
	const trace_point *a = (trace_point *) elm1;
	const trace_point *b = (trace_point *) elm2;

	if( a->d > b->d )
		return 1;
	else if( a->d < b->d )
		return -1;
	else
		return 0;
}

int angle_test(const void *elm1, const void *elm2)
{
	const trace_point *a = (trace_point *) elm1;
	const trace_point *b = (trace_point *) elm2;

	// elements are dotproducts of normalized vecs so bigger value, smaller angle
	if( a->d > b->d )
		return -1;
	else if( a->d < b->d )
		return 1;
	else
		return 0;
}

void CAimbot::DrawGrenadelauncherTrace()
{
	vec3_t angles, forward, viewpos, lastPos;
	bool once = false;

	if (!eth32.settings.rifleTracer)
		return;

	if (!(eth32.cg.currentWeapon->attribs & WA_RIFLE_GRENADE))
		return;

	if (eth32.cg.snap->ps.weaponTime != 0 )
		return;

	if (Engine.forward == NULL)
		return;

	trajectory_t rifleTrajectory;
	rifleTrajectory.trType = TR_GRAVITY;
	rifleTrajectory.trTime = eth32.cg.time;
	VectorCopy(eth32.cg.muzzle, rifleTrajectory.trBase);
	VectorCopy(rifleTrajectory.trBase, viewpos);

	VectorCopy(eth32.cg.refdefViewAngles, angles);
	VectorCopy(Engine.forward, forward);
	viewpos[2] += eth32.cg.snap->ps.viewheight;
	VectorMA(viewpos, 32, forward, viewpos);
	VectorScale(forward, 2000, forward);

	VectorCopy(forward, rifleTrajectory.trDelta);
	SnapVector(rifleTrajectory.trDelta);

	// Calculate rifle impact
	int timeOffset = 0;
	trace_t rifleTrace;
	vec3_t rifleImpact;
	VectorCopy(rifleTrajectory.trBase, rifleImpact);
	#define TIME_STEPP 50
	int maxTime = 3250;
	int totalFly = 0;

	while (timeOffset < maxTime) {
		vec3_t nextPos;
		timeOffset += TIME_STEPP;
		totalFly += TIME_STEPP;

		BG_EvaluateTrajectory(&rifleTrajectory, eth32.cg.time + timeOffset, nextPos, qfalse, 0);
		orig_CG_Trace(&rifleTrace, rifleImpact, 0, 0, nextPos, eth32.cg.snap->ps.clientNum, MASK_MISSILESHOT);

		// check for hit
		if (rifleTrace.startsolid || rifleTrace.fraction != 1) {
			// When a nade flies for over 750ms and hits somnexing, it'll explode
			if (totalFly > 750)
				break;

			// calculate reflect angle (forward axis)
			int hitTime = eth32.cg.time + totalFly - TIME_STEPP + TIME_STEPP * rifleTrace.fraction;
			BG_EvaluateTrajectoryDelta(&rifleTrajectory, hitTime, rifleTrajectory.trDelta, qfalse, 0);
			float dot = DotProduct(rifleTrajectory.trDelta, rifleTrace.plane.normal);
			VectorMA(rifleTrajectory.trDelta, -2*dot, rifleTrace.plane.normal, rifleTrajectory.trDelta);

			VectorScale(rifleTrajectory.trDelta, 0.35, rifleTrajectory.trDelta);

			if (rifleTrace.surfaceFlags == 0)
				VectorScale(rifleTrajectory.trDelta, 0.5, rifleTrajectory.trDelta);

			// calc new max time and reset trTime
			maxTime -= timeOffset;
			timeOffset = 0;
			rifleTrajectory.trTime = eth32.cg.time;

			// new base origins
			VectorCopy(rifleTrace.endpos, rifleTrajectory.trBase);

			SnapVector(rifleTrajectory.trDelta);
			SnapVector(rifleTrajectory.trBase);
		} else {
			VectorCopy(nextPos, rifleImpact);
       		if(!once)
			{
				VectorCopy(nextPos, lastPos);
				once = true;
			}
			if(Engine.IsVisible(eth32.cg.refdef->vieworg, nextPos, 0))
       			Engine.MakeRailTrail( lastPos, nextPos, false, colorGreen, eth32.cg.frametime*3 );
			else
				Engine.MakeRailTrail( lastPos, nextPos, false, colorBlue, eth32.cg.frametime*4 );

			VectorCopy(nextPos, lastPos);
		}
	}

	// copy the results for the cams
	VectorCopy( lastPos, lastImpact );
	Aimbot.flyTime = totalFly;
}

void CAimbot::DrawGrenadeTrace()
{
	vec3_t forward, tosspos, viewpos, lastPos, right;

	float pitch, upangle;
	bool once = false;

	if (!eth32.settings.grenadeTracer)
		return;

	if (!(eth32.cg.currentWeapon->attribs & WA_GRENADE))
		return;

	if (Engine.forward == NULL)
		return;

	trajectory_t grenadeTrajectory;
	grenadeTrajectory.trType = TR_GRAVITY;
	grenadeTrajectory.trTime = eth32.cg.time;


	AngleVectors(eth32.cg.refdefViewAngles, NULL, right, NULL);
	VectorCopy(Engine.forward, forward);
	VectorMA(eth32.cg.muzzle, 8, forward, tosspos);
	VectorMA(tosspos, 20, right, tosspos);
	tosspos[2] -= 8;
	SnapVector( tosspos );

	pitch = eth32.cg.refdefViewAngles[0];
	if( pitch >= 0 ) {
		forward[2] += 0.5f;
		pitch = 1.3f;
	}
	else {
		pitch = -pitch;
		pitch = min( pitch, 30 );
		pitch /= 30.f;
		pitch = 1 - pitch;
		forward[2] += (pitch * 0.5f);
		pitch *= 0.3f;
		pitch += 1.f;
	}

	upangle = -eth32.cg.refdefViewAngles[0];
	upangle = min(upangle, 50);
	upangle = max(upangle, -50);
	upangle = upangle/100.0f;
	upangle += 0.5f;

	if(upangle < .1)
		upangle = .1;
	upangle *= 900.0f*pitch;

	VectorNormalizeFast( forward );

	// check for valid start spot (so you don't throw through or get stuck in a wall)
	VectorCopy( self->lerpOrigin, viewpos );
	viewpos[2] += eth32.cg.snap->ps.viewheight;

	trace_t tr;
	orig_CG_Trace(&tr, viewpos, tv(-4.f, -4.f, 0.f), tv(4.f, 4.f, 6.f), tosspos, self->currentState->number, MASK_MISSILESHOT);

	if( tr.startsolid ) {
		VectorCopy( forward, viewpos );
		VectorMA( self->lerpOrigin, -24.f, viewpos, viewpos );

		orig_CG_Trace(&tr, viewpos, tv(-4.f, -4.f, 0.f), tv(4.f, 4.f, 6.f), tosspos, self->currentState->number, MASK_MISSILESHOT);
		VectorCopy( tr.endpos, tosspos );
	} else if( tr.fraction < 1 ) {	// oops, bad launch spot
		VectorCopy( tr.endpos, tosspos );
		SnapVectorTowards( tosspos, viewpos );
	}

	VectorScale(forward, upangle, forward);
	VectorCopy(tosspos, grenadeTrajectory.trBase);
	VectorCopy(forward, grenadeTrajectory.trDelta);
	SnapVector(grenadeTrajectory.trDelta);

	// Calculate grenade impact
	int timeOffset = 0;
	trace_t grenadeTrace;
	vec3_t grenadeImpact;
	VectorCopy(grenadeTrajectory.trBase, grenadeImpact);
	#define TIME_STEPP 50
	int maxTime = 4000;
	int totalFly = 0;
	int bounces = 0;

	while (timeOffset < maxTime) {
		vec3_t nextPos;
		timeOffset += TIME_STEPP;
		totalFly += TIME_STEPP;

		BG_EvaluateTrajectory(&grenadeTrajectory, eth32.cg.time + timeOffset, nextPos, qfalse, 0);
		orig_CG_Trace(&grenadeTrace, grenadeImpact, 0, 0, nextPos, eth32.cg.snap->ps.clientNum, MASK_MISSILESHOT);

		// check for hit
		if (grenadeTrace.startsolid || grenadeTrace.fraction != 1) {
			// calculate reflect angle (forward axis)
			int hitTime = eth32.cg.time + totalFly - TIME_STEPP + TIME_STEPP * grenadeTrace.fraction;
			BG_EvaluateTrajectoryDelta(&grenadeTrajectory, hitTime, grenadeTrajectory.trDelta, qfalse, 0);
			float dot = DotProduct(grenadeTrajectory.trDelta, grenadeTrace.plane.normal);
			VectorMA(grenadeTrajectory.trDelta, -2*dot, grenadeTrace.plane.normal, grenadeTrajectory.trDelta);

			bounces++;
			VectorScale(grenadeTrajectory.trDelta, powf(0.35, bounces), grenadeTrajectory.trDelta);

			if (VectorLength(grenadeTrajectory.trDelta) < 30.0f)
				return;

			if (grenadeTrace.surfaceFlags == 0)
				VectorScale(grenadeTrajectory.trDelta, 0.5, grenadeTrajectory.trDelta);

			// calc new max time and reset trTime
			maxTime -= timeOffset;
			timeOffset = 0;
			grenadeTrajectory.trTime = eth32.cg.time;

			// new base origins
			VectorCopy(grenadeTrace.endpos, grenadeTrajectory.trBase);

			SnapVector(grenadeTrajectory.trDelta);
			SnapVector(grenadeTrajectory.trBase);
		} else {
			VectorCopy(nextPos, grenadeImpact);
       		if(!once)
			{
				VectorCopy(nextPos, lastPos);
				once = true;
			}
			if(Engine.IsVisible(eth32.cg.refdef->vieworg, nextPos, 0))
       			Engine.MakeRailTrail( lastPos, nextPos, false, colorGreen, eth32.cg.frametime*3 );
			else
				Engine.MakeRailTrail( lastPos, nextPos, false, colorBlue, eth32.cg.frametime*4 );

			VectorCopy(nextPos, lastPos);
		}
	}

	// copy the results for the cams
	VectorCopy( lastPos, lastImpact );
	Aimbot.flyTime = totalFly;
}

void CAimbot::DrawBodyBox( int clientNum, bool axes )
{
	player_t	*player = &eth32.cg.players[clientNum];
	vec3_t		size;
	vec3_t		boxOrigin;

	// i suppose body box size does not vary much (if at all)
	size[0] = size[1] = eth32.settings.bodybox;
	size[2] = 24.0;

	VectorCopy( player->lerpOrigin, boxOrigin );

	if (player->currentState->eFlags & EF_PRONE)
		size[2] += PRONE_VIEWHEIGHT+12.0;
	else if (player->currentState->eFlags & EF_CROUCHING)
		size[2] += CROUCH_VIEWHEIGHT+8.0;
	else
		size[2] += DEFAULT_VIEWHEIGHT-4.0;

	boxOrigin[2] += -24.0 + size[2]*0.5;

	vec3_t min,max;
	VectorCopy( boxOrigin, min );
	VectorCopy( boxOrigin, max );

	VectorMA( boxOrigin, -size[0]*0.5, xAxis, min );
	VectorMA( min, -size[1]*0.5, yAxis, min );
	VectorMA( min, -size[2]*0.5, zAxis, min );

	VectorMA( boxOrigin, size[0]*0.5, xAxis, max );
	VectorMA( max, size[1]*0.5, yAxis, max );
	VectorMA( max, size[2]*0.5, zAxis, max );

	Engine.MakeRailTrail( min, max, true, eth32.settings.colorBodyHitbox, eth32.settings.bodyRailTime );
}

void CAimbot::DrawHeadBox( int clientNum, bool axes )
{
	vec3_t		cv;
	vec3_t		p;
	hitbox_t 	*hbox;
	bool		moving;
	int 		eFlags;
	vec3_t		vel;
	vec3_t		size;
	float		speed;

	if( eth32.settings.hitboxType == HITBOX_CUSTOM )
		hbox = &customHitbox;
	else
		hbox = &head_hitboxes[eth32.settings.hitboxType];

	eFlags = eth32.cg.players[clientNum].currentState->eFlags;

	VectorCopy( eth32.cg.players[clientNum].currentState->pos.trDelta, vel );
	/* this is not really movement detector, but animation detector
		only use pos.trDelta since that handles user-intended velocities */
	if( VectorCompare(vel, vec3_origin) )
		moving = false;
	else
		moving = true;

	if( (eFlags & EF_PRONE) || (eFlags & EF_PRONE_MOVING) )
		VectorCopy( hbox->prone_offset, cv );
	else {
		if( !moving ){
			if( eFlags & EF_CROUCHING )
				VectorCopy( hbox->crouch_offset, cv );
			else
				VectorCopy( hbox->stand_offset, cv );
		} else {
			if( eFlags & EF_CROUCHING )
				VectorCopy( hbox->crouch_offset_moving, cv );
			else
				VectorCopy( hbox->stand_offset_moving, cv );
		}
	}

	VectorCopy( hbox->size, size );

	/* Dynamic Hitbox - adjust X,Y,Z size based on speed perpendicular to our viewdirection
		This is gives the aimbot 'fastness' of aim when guy corners */
	if (eth32.settings.dynamicHitboxScale > 0){
		speed = VectorLength(vel) - fabs(DotProduct(vel,eth32.cg.refdef->viewaxis[0]));

		if( speed > 0 ){
			size[0] *= (1.0+eth32.settings.dynamicHitboxScale*speed/eth32.cg.snap->ps.speed);		// ps.speed is g_speed
			size[1] *= (1.0+eth32.settings.dynamicHitboxScale*speed/eth32.cg.snap->ps.speed);		// ps.speed is g_speed
		}
	}

	orientation_t *head = &eth32.cg.players[clientNum].orHead;
	// rotate hitbox offset vector with tag (hitboxes themselves dont rotate)
	VectorMA( head->origin, cv[2], head->axis[2], p );
	VectorMA( p, cv[1], head->axis[1], p );
	VectorMA( p, cv[0], head->axis[0], p );

	vec3_t min,max;

	VectorMA( p, -0.5* size[0], xAxis, min );
	VectorMA( min, -0.5* size[1], yAxis, min );
	VectorMA( min, -0.5* size[2], zAxis, min );

	VectorMA( p, 0.5* size[0], xAxis, max );
	VectorMA( max, 0.5* size[1], yAxis, max );
	VectorMA( max, 0.5* size[2], zAxis, max );

	Engine.MakeRailTrail( min, max, true, eth32.settings.colorHeadHitbox, eth32.settings.headRailTime );
	if (axes){
	vec3_t ex1,ex2,ey1,ey2,ez1,ez2;
	VectorMA( p, 0, head->axis[0], ex1 );
	VectorMA( p, 25, head->axis[0], ex2 );

	VectorMA( p, 0, head->axis[1], ey1 );
	VectorMA( p, 25, head->axis[1], ey2 );

	VectorMA( p, 0, head->axis[2], ez1 );
	VectorMA( p, 25, head->axis[2], ez2 );

	Engine.MakeRailTrail( ex1, ex2, false, eth32.settings.colorXAxis, eth32.settings.headRailTime );
	Engine.MakeRailTrail( ey1, ey2, false, eth32.settings.colorYAxis, eth32.settings.headRailTime );
	Engine.MakeRailTrail( ez1, ez2, false, eth32.settings.colorZAxis, eth32.settings.headRailTime );
	}
}

/* trace a userdefined box, set the visible vector, and return visibility
	size is world coordinates, X*Y*Z */
bool CAimbot::traceBox( vec3_t boxOrigin, float dist, vec3_t size, vec3_t trOrigin, int skipEnt, player_t *player, boxtrace_t trType,  vec3_t visible, int maxTraces )
{
	trace_point *p;
	VectorCopy( vec3_origin, visible );
	float boxVolume = size[0]*size[1]*size[2];
	// need this, because if we are modifying the origin with prediction
	// false points (0,0,0) will seem valid after prediction is applied
	if (VectorCompare(boxOrigin, vec3_origin))
		return false;
	vec3_t velocity;
   
      	VectorMA(boxOrigin, eth32.settings.predTarget, player->currentState->pos.trDelta, boxOrigin);

      	maxTraces = 24;


	// for now don't allow center trace for body in this fashion
	if (trType != TRACE_BODY){
		if (IsPointVisible(trOrigin, boxOrigin, skipEnt)){
			VectorCopy( boxOrigin, visible );
			return true;
		}
	else if( trType == TRACE_CENTER )
			return false;
	} 
	else
		maxTraces += 6;

	// check for a too small volume
	if( boxVolume < 1.0 )
		return false;

	int k = 0;
	p = (trace_point *)malloc(sizeof(trace_point)*maxTraces);
	memset(p, 0, sizeof(trace_point)*maxTraces);

	switch( trType ){
		case TRACE_RANDOM_SURFACE: {
			/* get the 1, 2, or 3 plane(s) facing us */
			vec3_t dir,ndir, dr;
			vec3_t n, a, b;
			int i,j,N;
			float frac;
			VectorSubtract( eth32.cg.refdef->vieworg, boxOrigin, dir );
			VectorCopy( dir, ndir );
			VectorNormalizeFast( ndir );
			for( i=0; i<6; i++ ){
				n[(i+1)%3] = n[(i+2)%3] = 0;
				n[(i % 3)] = (i > 2) ? 1.0 : -1.0;
				frac = DotProduct( n, ndir );
				if( frac > 0 ){
					/* this plane is visible, fill it up with points, we have MAX_TR to distribute
					   so allocate by visible surface area. Note: max( sum(DotProduct(ndir, n)) ) = sqrt(3)
					   by axiom of spherical geometry */

					N = (int)((frac/sqrtf(3))*maxTraces);
					/* construct orthonormal plane vecs */
					a[i%3] = a[(i+2)%3] = 0;
					a[((i+1) % 3)] = 1.0;
					b[i%3] = b[(i+1)%3] = 0;
					b[((i+2) % 3)] = 1.0;
					for( j=0;j<N; j++ ){
						VectorMA( boxOrigin, size[i % 3]*0.5, n, p[k].pt );
						VectorMA( boxOrigin, size[i % 2]*0.5, n, p[k].pt );
						VectorMA( p[k].pt, size[(i+1) % 3]*crandom()*0.5, a, p[k].pt );
						VectorMA( p[k].pt, size[(i+2) % 3]*crandom()*0.5, b, p[k].pt );
						VectorMA( p[k].pt, size[(i+3) % 3]*crandom()*0.7, b, p[k].pt );

						/* sort based on angular distance from center based on our viewdir (to maximize acc) */
						VectorSubtract( eth32.cg.refdef->vieworg, p[k].pt, dr );
						VectorNormalizeFast( dr );
						p[k].d = DotProduct( dr, ndir );
						k++;
					}
				}
			}
			qsort( p, maxTraces, sizeof(trace_point), angle_test );
			break;
		}
		case TRACE_RANDOM_VOLUME: {
			// trace random points within hbox volume (faster but less accurate) 
			for( k=0; k < maxTraces; k++ ){
				VectorMA( p[k].pt, size[0]*crandom()*-0.5, xAxis, p[k].pt );
				VectorMA( p[k].pt, size[1]*crandom()*-0.5, yAxis, p[k].pt );
				VectorMA( p[k].pt, size[2]*crandom()*-0.5, zAxis, p[k].pt );
				p[k].d = VectorLengthSquared( p[k].pt );
			}
			qsort( p, maxTraces, sizeof(trace_point), distance_test );

			for( k=0; k < maxTraces; k++ )
				VectorAdd( boxOrigin, p[k].pt, p[k].pt );

			break;
		}
		case TRACE_CONTOUR: {
			size[0] -= 0.7;
			size[1] -= 0.7;
			//trace periphery - prioritize plane centers, more chance of hitting
			VectorMA( boxOrigin, size[0]*0.5, xAxis, p[k].pt ); k++;
			VectorMA( boxOrigin, -size[0]*0.5, xAxis, p[k].pt ); k++;
			VectorMA( boxOrigin, size[1]*0.5, yAxis, p[k].pt ); k++;
			VectorMA( boxOrigin, -size[1]*0.5, yAxis, p[k].pt ); k++;
			VectorMA( boxOrigin, size[2]*0.5, zAxis, p[k].pt ); k++;

			//corners
			VectorMA( boxOrigin, size[0]*0.5, xAxis, p[k].pt );
			VectorMA( p[k].pt, size[2]*0.5, zAxis, p[k].pt );
			k++;

			VectorMA( boxOrigin, -size[0]*0.5, xAxis, p[k].pt );
			VectorMA( p[k].pt, size[2]*0.5, zAxis, p[k].pt );
			k++;

			VectorMA( boxOrigin, size[1]*0.5, yAxis, p[k].pt );
			VectorMA( p[k].pt, size[2]*0.5, zAxis, p[k].pt );
			k++;

			VectorMA( boxOrigin, -size[1]*0.5, yAxis, p[k].pt );
			VectorMA( p[k].pt, size[2]*0.5, zAxis, p[k].pt );
			k++;
			for(k=4; k<maxTraces; k++){
				VectorMA( boxOrigin, size[0]*crandom()*0.5, xAxis, p[k].pt );
				VectorMA( p[k].pt, size[1]*crandom()*0.5, yAxis, p[k].pt );
				VectorMA( p[k].pt, size[2]*crandom()*0.5, zAxis, p[k].pt );
			}

			break;
		}

		// for capsules size[0] is the radious and size[2] the height
		case TRACE_CAPSULE_VOLUME: {
			float phi, r;
			// trace random points within capsule volume
			for( k=0; k < maxTraces; k++ ){
				phi = 2.0*M_PI*random();
				r = 0.5*random()*size[0];
				p[k].pt[0] = cosf(phi)*r;
				p[k].pt[1] = sinf(phi)*r;
				p[k].pt[2] = 0.5*crandom()*size[2];
				p[k].d = r*r+p[k].pt[2]*p[k].pt[2];
			}
			qsort( p, maxTraces, sizeof(trace_point), distance_test );

			for( k=0; k < maxTraces; k++ )
				VectorAdd( boxOrigin, p[k].pt, p[k].pt );

			break;
		}
		// for capsules size[0] is the radious and size[2] the height
		case TRACE_CAPSULE_SURFACE: {
			vec3_t dir, dr;
			float phi;

			VectorSubtract( eth32.cg.refdef->vieworg, boxOrigin, dir );
			VectorNormalizeFast( dir );

			for( k=0; k < maxTraces; k++ ){
				phi = 2.0*M_PI*random();
				p[k].pt[0] = cosf(phi)*0.5*size[0];
				p[k].pt[1] = sinf(phi)*0.5*size[0];
				p[k].pt[2] = 0.5*crandom()*size[2];

				VectorAdd( boxOrigin, p[k].pt, p[k].pt );

				VectorSubtract( eth32.cg.refdef->vieworg, p[k].pt, dr );
				VectorNormalizeFast( dr );
				p[k].d = DotProduct( dr, dir );

			}
			qsort( p, maxTraces, sizeof(trace_point), angle_test );

			break;
		}
		// First a few fixed points on the upper torso, when nothing is visible, do so random points
		case TRACE_BODY: {
			size[0] -= 0.7;
			size[1] -= 0.7;
			size[2] -= 0.7;
			// create 4 fixed points on the vertices of the top square
			VectorMA( boxOrigin, 0.5*size[2]-5.0, zAxis, p[0].pt );
			VectorMA( p[0].pt, 0.5*size[0], xAxis, p[0].pt );
			VectorMA( p[0].pt, 0.5*size[1], yAxis, p[0].pt );

			VectorMA( boxOrigin, 0.5*size[2]-5.0, zAxis, p[1].pt );
			VectorMA( p[1].pt, -0.5*size[0], xAxis, p[1].pt );
			VectorMA( p[1].pt, 0.5*size[1], yAxis, p[1].pt );

			VectorMA( boxOrigin, 0.5*size[2]-5.0, zAxis, p[2].pt );
			VectorMA( p[2].pt, -0.5*size[0], xAxis, p[2].pt );
			VectorMA( p[2].pt, -0.5*size[1], yAxis, p[2].pt );

			VectorMA( boxOrigin, 0.5*size[2]-5.0, zAxis, p[3].pt );
			VectorMA( p[3].pt, 0.5*size[0], xAxis, p[3].pt );
			VectorMA( p[3].pt, -0.5*size[1], yAxis, p[3].pt );

			// if these fail to be visible (enemy behind complex object with cracks/holes
			// then create a user-requested amount of random points
			for( k=4; k < maxTraces; k++ ){
				VectorMA( boxOrigin, size[0]*crandom()*0.5, xAxis, p[k].pt );
				VectorMA( p[k].pt, size[1]*crandom()*0.5, yAxis, p[k].pt );
				VectorMA( p[k].pt, size[2]*crandom()*0.5, zAxis, p[k].pt );
			}
			break;
		}
		default:
			/* unknown trace type */
			free(p);
			return false;
	}

	// trace them all
	for( k=0; k<maxTraces; k++ ){
		if(IsPointVisible(trOrigin, p[k].pt, skipEnt)){
			VectorCopy( p[k].pt, visible );
			free(p);
			return true;
		}
	}

	free(p);
	return false;
}

bool CAimbot::traceHead( orientation_t *head, vec3_t hitPoint, int clientNum )
{
	return this->traceHead(head, hitPoint, eth32.cg.muzzle, clientNum);
}

bool CAimbot::traceHead( orientation_t *head, vec3_t hitPoint, vec3_t start, int clientNum )
{
	player_t	*player = &eth32.cg.players[clientNum];
	vec3_t		cv;
	vec3_t		p;
	hitbox_t 	*hbox;
	bool		moving;
	int 		eFlags;
	centity_t	*cent;
	vec3_t		vel;
	vec3_t		angles, origin;
	vec3_t		size;
	float		speed;

	// allow users to disable head traces all together
	if (!eth32.cg.currentWeapon->headTraces)
		return false;

	if( eth32.settings.hitboxType == HITBOX_CUSTOM )
		hbox = &customHitbox;
	else
		hbox = &head_hitboxes[eth32.settings.hitboxType];

	eFlags = player->currentState->eFlags;

	VectorCopy( player->currentState->pos.trDelta, vel );

	/* this is not really movement detector, but animation detector
		only use pos.trDelta since that handles user-intended velocities */
	if( VectorCompare(vel, vec3_origin) )
		moving = false;
	else
		moving = true;

	if( (eFlags & EF_PRONE) || (eFlags & EF_PRONE_MOVING) )
		VectorCopy( hbox->prone_offset, cv );
	else {
		if( !moving ){
			if( eFlags & EF_CROUCHING )
				VectorCopy( hbox->crouch_offset, cv );
			else
				VectorCopy( hbox->stand_offset, cv );
		} else {
			if( eFlags & EF_CROUCHING )
				VectorCopy( hbox->crouch_offset_moving, cv );
			else
				VectorCopy( hbox->stand_offset_moving, cv );
		}
	}
	// Auto Aimpoints
	if (eth32.cg.currentWeapon->attribs & WA_USER_DEFINED) {
		eth32.cg.currentWeapon->headTraces = eth32.settings.pointshead;
       		eth32.cg.currentWeapon->bodyTraces = eth32.settings.pointsbody;
       	}

	
	switch(eth32.settings.predictionType) {
		case PRED_OFF:
			eth32.settings.predTarget = eth32.settings.pred;
			break;
		case PRED_PING: {			
			if (eth32.cgMod->type != MOD_ETPRO) {
				int slot, ping;
				static int lastUpdateTime = 0;
		
				if (eth32.cg.cgTime - lastUpdateTime >= 30000) {
					Syscall.SendClientCommand( "score" );
					lastUpdateTime = eth32.cg.cgTime;
				}

				// For now we'll use PLAYER_ENEMY as BOT
				for (int i=0; i<(eth32.cg.numScores); i++) {	// i is not the real player slot
					slot = eth32.ocg.scores[i].client;	// Translates i to real player slot (scoreboard based)
					ping = eth32.ocg.scores[i].ping;
					if (ping == 0)
						eth32.cg.players[slot].playerType = PLAYER_ENEMY;
					else if (ping != 0)
						eth32.cg.players[slot].playerType = PLAYER_NORMAL;
					// Do not select teammates as enemy.
					if (IS_FRIENDLY(slot))
						eth32.cg.players[slot].playerType = PLAYER_NORMAL;
				}
				
				if (player->playerType == PLAYER_ENEMY)
					eth32.settings.predTarget = eth32.settings.predbot;
				else if (player->playerType == PLAYER_NORMAL)
					eth32.settings.predTarget = eth32.settings.pred;
				
			} else
				eth32.settings.predTarget = eth32.settings.pred;
			break;
		}
		case PRED_MAN: {
			if (player->playerType == PLAYER_ENEMY)
				eth32.settings.predTarget = eth32.settings.predbot;
			else if (player->playerType == PLAYER_NORMAL)
				eth32.settings.predTarget = eth32.settings.pred;
			break;
		}
	}
		
	/** ----------------------------------------------------------------------------------
               	     		AUTO VECS & WEAPON DELAY BASED ON DISTANCE
	---------------------------------------------------------------------------------- **/		
	
	float close, far;
       	float f = player->distance/2999;
       	f = f > 1.0 ? 1.0 : f;
	
	// Head Hitbox Size		
	hbox->size[0] = eth32.settings.headBoxSize;
        hbox->size[1] = eth32.settings.headBoxSize;
        hbox->size[2] = eth32.settings.headBoxSize;
     
		VectorCopy( hbox->size, size );

        // Vec Z corrections
        if (eth32.settings.autoVecZ) {
       		if (player->distance >= 0 && player->distance < 3000) {			// Low dist       	
       			close = 6.5;
        		far = 5.5;
       
       			hbox->stand_offset[2] = close + (far-close)*f;
       			hbox->stand_offset_moving[2] = close + (far-close)*f;
       			hbox->crouch_offset[2] = close + (far-close)*f;
  			hbox->crouch_offset_moving[2] = close + (far-close)*f;
       		} else if (player->distance >= 3000 && player->distance < 6000) {	// Med dist
       			close = 5.5;
        		far = 4.5;
        	
        		hbox->stand_offset[2] = close + (far-close)*f;
       			hbox->stand_offset_moving[2] = close + (far-close)*f;
       			hbox->crouch_offset[2] = close + (far-close)*f;
  			hbox->crouch_offset_moving[2] = close + (far-close)*f;
        	} else if (player->distance >= 6000 && player->distance < 9000) {	// Long dist
        		close = 4.5;
        		far = 3.0;
        	
        		hbox->stand_offset[2] = close + (far-close)*f;
       			hbox->stand_offset_moving[2] = close + (far-close)*f;
       			hbox->crouch_offset[2] = close + (far-close)*f;
  			hbox->crouch_offset_moving[2] = close + (far-close)*f;
        	} else if (player->distance >= 9000) {								// Very long dist
        		close = 3.0;
        		far = 2.0;
        	
        		hbox->stand_offset[2] = close + (far-close)*f;
       			hbox->stand_offset_moving[2] = close + (far-close)*f;
       			hbox->crouch_offset[2] = close + (far-close)*f;
  			hbox->crouch_offset_moving[2] = close + (far-close)*f;
  		}
    	} else {		
        	if (player->distance >= 0 && player->distance < 3000) {
        		hbox->stand_offset[2] = eth32.settings.standlowZ;
       			hbox->stand_offset_moving[2] = eth32.settings.runlowZ;
       			hbox->crouch_offset[2] = eth32.settings.crouchlowZ;
  			hbox->crouch_offset_moving[2] = eth32.settings.crawllowZ;
       		} else if (player->distance >= 3000 && player->distance < 6000) {
       			hbox->stand_offset[2] = eth32.settings.standmedZ;
       			hbox->stand_offset_moving[2] = eth32.settings.runmedZ;
       			hbox->crouch_offset[2] = eth32.settings.crouchmedZ;
  			hbox->crouch_offset_moving[2] = eth32.settings.crawlmedZ;
       		} else if (player->distance >= 6000) {
       			hbox->stand_offset[2] = eth32.settings.standfarZ;
       			hbox->stand_offset_moving[2] = eth32.settings.runfarZ;
       			hbox->crouch_offset[2] = eth32.settings.crouchfarZ;
  			hbox->crouch_offset_moving[2] = eth32.settings.crawlfarZ;
       		}
       	}
       	
       	hbox->stand_offset[0] = eth32.settings.standX;
       	hbox->stand_offset_moving[0] = eth32.settings.runX;
       	hbox->stand_offset[1] = eth32.settings.standY;
       	hbox->stand_offset_moving[1] = eth32.settings.runY;
     
       	hbox->crouch_offset[0] = eth32.settings.crouchX;
  	hbox->crouch_offset_moving[0] = eth32.settings.crawlX;
  	hbox->crouch_offset[1] = eth32.settings.crouchY;
  	hbox->crouch_offset_moving[1] = eth32.settings.crawlY;
       	
       	hbox->prone_offset[0] = eth32.settings.proneX;
       	hbox->prone_offset[1] = eth32.settings.proneY;
       	hbox->prone_offset[2] = eth32.settings.proneZ;
    	
    	// Auto Weapon Delay
    	if (eth32.settings.autoDelay) {
    		if (eth32.cg.currentWeapon->attribs & WA_USER_DEFINED) {
    			if (player->distance > 0 && player->distance < 2500)
				eth32.cg.currentWeapon->delay = eth32.settings.delayClose;
			else if (player->distance > 2500 && player->distance < 5000)
				eth32.cg.currentWeapon->delay = eth32.settings.delayMed;
			else if (player->distance > 5000)
				eth32.cg.currentWeapon->delay = eth32.settings.delayFar;	
    		}    
        }

	/* Dynamic Hitbox - adjust X,Y,Z size based on speed perpendicular to our viewdirection
		This is gives the aimbot 'fastness' of aim when guy corners */
	if (eth32.settings.dynamicHitboxScale > 0){
		speed = VectorLength(vel) - fabs(DotProduct(vel,eth32.cg.refdef->viewaxis[0]));

		if( speed > 0 ){
			size[0] *= (1.0+eth32.settings.dynamicHitboxScale*speed/eth32.cg.snap->ps.speed);		// ps.speed is g_speed
			size[1] *= (1.0+eth32.settings.dynamicHitboxScale*speed/eth32.cg.snap->ps.speed);		// ps.speed is g_speed
		}
	}

	// rotate hitbox offset vector with tag (hitboxes themselves dont rotate)
	VectorMA( head->origin, cv[2], head->axis[2], p );
	VectorMA( p, cv[1], head->axis[1], p );
	VectorMA( p, cv[0], head->axis[0], p );

	return traceBox(p, player->distance, size, start, clientNum, player, (boxtrace_t) eth32.settings.traceType, hitPoint, eth32.cg.currentWeapon->headTraces);
}

bool CAimbot::traceBody( vec3_t hitPoint, int clientNum )
{
	player_t	*player = &eth32.cg.players[clientNum];
	vec3_t		size;
	vec3_t		boxOrigin;
	vec3_t		vel;
	float		speed;

	// allow user to turn off body traces all together
	if (!eth32.cg.currentWeapon->bodyTraces)
		return false;

	// i suppose body box size does not vary much (if at all)
	size[0] = size[1] = eth32.settings.bodybox;
	size[2] = 24.0;

	VectorCopy( player->lerpOrigin, boxOrigin );

	if (player->currentState->eFlags & EF_PRONE)
		size[2] += PRONE_VIEWHEIGHT+12.0;
	else if (player->currentState->eFlags & EF_CROUCHING)
		size[2] += CROUCH_VIEWHEIGHT+8.0;
	else
		size[2] += DEFAULT_VIEWHEIGHT-4.0;

	boxOrigin[2] += -24.0 + size[2]*0.5;
	// sol: testing with using user selected trace type on body
	return traceBox( boxOrigin, eth32.cg.players[clientNum].distance, size, eth32.cg.muzzle, clientNum, player, (boxtrace_t)eth32.settings.traceType, hitPoint, eth32.cg.currentWeapon->bodyTraces );
}

int SortDistance(const void *p1, const void *p2)
{
	const player_t *player1 = *(const player_t**)p1;
	const player_t *player2 = *(const player_t**)p2;

	if (player1->distance < player2->distance)
		return -1;
	else
		return 1;
}

int SortThreat(const void *p1, const void *p2)
{
	const player_t *player1 = *(const player_t**)p1;
	const player_t *player2 = *(const player_t**)p2;

	if (player1->stats.threat == player2->stats.threat)
		return SortDistance( p1, p2 );

	if (player1->stats.threat > player2->stats.threat)
		return -1;
	else
		return 1;
}

int SortAccuracy(const void *p1, const void *p2)
{
	const player_t *player1 = *(const player_t**)p1;
	const player_t *player2 = *(const player_t**)p2;

	if (player1->stats.accuracy > player2->stats.accuracy || (player1->stats.accuracy == player2->stats.accuracy && player1->distance < player2->distance))
		return -1;
	else
		return 1;
}

int SortKdRatio(const void *p1, const void *p2)
{
	const player_t *player1 = *(const player_t**)p1;
	const player_t *player2 = *(const player_t**)p2;

	if (player1->stats.kdRatio == player2->stats.kdRatio)
		return SortDistance( p1, p2 );

	if (player1->stats.kdRatio > player2->stats.kdRatio)
		return -1;
	else
		return 1;
}

int SortAttacker(const void *p1, const void *p2) 
{
	const player_t *player1 = *(const player_t**)p1;
    	const player_t *player2 = *(const player_t**)p2;
    	player_t *player;

	if (player1->clientNum == eth32.cg.snap->ps.persistant[PERS_ATTACKER])
		return -1;
	else if (player2->clientNum == eth32.cg.snap->ps.persistant[PERS_ATTACKER])
		return 1;
	else
		return SortDistance( p1, p2 );
}

qboolean isSingleShootWeapon(){
	int weapon = eth32.cg.snap->ps.weapon;
	switch( weapon ){
		case WP_K43:
		case WP_GARAND:
		case WP_CARBINE:
		case WP_KAR98:
		case WP_LUGER:
		case WP_SILENCER:
		case WP_AKIMBO_LUGER:
		case WP_AKIMBO_SILENCEDLUGER:
		case WP_COLT:
		case WP_SILENCED_COLT:
		case WP_AKIMBO_COLT:
		case WP_AKIMBO_SILENCEDCOLT:
			return qtrue;
		default:
			return qfalse;
	}
}

// normal bullet aimbot at full efficiency
void CAimbot::DoBulletBot(void)
{
	// autofire
	autoMode = (eth32.cg.currentWeapon->autofire && eth32.settings.autofire && eth32.settings.aimType != AIM_ON_FIRE);

	// validate target
	autoMode |= (eth32.settings.aimType == AIM_ON_FIRE && eth32.settings.atkValidate);
	
	if (eth32.settings.aimType == AIM_TRIGGER) {
		// Set a far away end point
		vec3_t aheadPos;
		VectorMA(eth32.cg.refdef->vieworg, 8192, eth32.cg.refdef->viewaxis[0], aheadPos);
	
		// Get trace for this point
		trace_t t;
		orig_CG_Trace(&t, eth32.cg.refdef->vieworg, NULL, NULL, aheadPos, eth32.cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_ITEM);
		Trigger = IS_ENEMY(t.entityNum);
	} else 
		Trigger = false;

	// Weapon parameters not known or weapon set to not aim
	if (!eth32.cg.currentWeapon || !eth32.settings.aimSort || !eth32.settings.lockSensitivity || !(eth32.cg.currentWeapon->attribs & WA_USER_DEFINED))
		return;

	// global aim is off or set for on fire without fire button down
	if (eth32.settings.aimType == AIM_OFF ||
		(eth32.settings.aimType == AIM_ON_FIRE && !attackPressed) ||
		(eth32.settings.aimType == AIM_ON_BUTTON && !aimkeyPressed))
		return;

	// Don't shoot if we're speccing/following a player
	if (eth32.cg.clientNum != eth32.cg.snap->ps.clientNum)
		return;
		
	if (eth32.cg.cgTime == eth32.cg.snap->serverTime)
		return;	
		
	// Don't aim if we're reloading
	if (eth32.cg.snap->ps.weaponstate == WEAPON_RELOADING)
		return; 
		
	// Don't aim with knife
	if (eth32.cg.snap->ps.weapon == WP_KNIFE)
		return;		
	
	// Don't aim if we're in limbo menu	
	if (Gui.IsLimboPanelUp())
		return;

	player_t *player;

	// sol: lock target check here
	if (eth32.settings.lockTarget && lastTarget && lastTargetValid) { // redundant :P
		player = lastTarget;

		if (eth32.settings.headbody == HEAD_PRIORITY || eth32.settings.headbody == HEAD_BODY) {
			if (traceHead(&player->orHead, player->headPt, player->clientNum)) {
				VectorCopy(player->headPt, player->aimPt);
				target = player;
			} else if (traceBody(player->aimPt, player->clientNum ))
				target = player;
		} else if (eth32.settings.headbody == BODY_HEAD) {
			if (traceBody(player->aimPt, player->clientNum ))
				target = player;
		  else if (traceHead(&player->orHead, player->headPt, player->clientNum)) {
				VectorCopy(player->headPt, player->aimPt);
				target = player;
			}
		} else if (eth32.settings.headbody == HEAD_ONLY) {
			if (traceHead(&player->orHead, player->headPt, player->clientNum)) {
				VectorCopy(player->headPt, player->aimPt);
				target = player;
			}
		} else if (eth32.settings.headbody == BODY_ONLY) {
			if (traceBody(player->aimPt, player->clientNum ))
				target = player;
		}
	}

	// sol: locked target wasn't valid or isn't visible, check normal target list
	if (!target) {
		// run sort based on selection type
		if (eth32.settings.aimSort == SORT_CROSSHAIR)
			qsort(frameTargets, numFrameTargets, sizeof(player_t*), SortCrosshair);
		else if (eth32.settings.aimSort == SORT_DISTANCE)
			qsort(frameTargets, numFrameTargets, sizeof(player_t*), SortDistance);
		else if (eth32.settings.aimSort == SORT_ATTACKER)
			qsort(frameTargets, numFrameTargets, sizeof(player_t*), SortAttacker);
		else if (eth32.settings.aimSort == SORT_KDRATIO)
			qsort(frameTargets, numFrameTargets, sizeof(player_t*), SortKdRatio);
		else if (eth32.settings.aimSort == SORT_ACCURACY)
			qsort(frameTargets, numFrameTargets, sizeof(player_t*), SortAccuracy);
		else if (eth32.settings.aimSort == SORT_THREAT)
			qsort(frameTargets, numFrameTargets, sizeof(player_t*), SortThreat);
		else
			return;	// should never get here

		// sol: modified to allow user to choose priority, may need a seperate function to keep this clean
		if (eth32.settings.headbody == HEAD_PRIORITY) {
			for (int i=0 ; i<numFrameTargets ; i++)
			{
				player = frameTargets[i];

				if (traceHead(&player->orHead, player->headPt, player->clientNum)) {
					VectorCopy(player->headPt, player->aimPt);
					target = player;
					break;
				}
			}

			if (!target){
				for (int i=0 ; i<numFrameTargets ; i++)
				{
					player = frameTargets[i];

					if (traceBody(player->aimPt, player->clientNum )) {
						target = player;
						break;
					}
				}
			}
		}
		else if (eth32.settings.headbody == HEAD_BODY) {
			for (int i=0 ; i<numFrameTargets ; i++)
			{
				player = frameTargets[i];

				if (traceHead(&player->orHead, player->headPt, player->clientNum)) {
					VectorCopy(player->headPt, player->aimPt);
					target = player;
					break;
				}
				if (!target) {
					if (traceBody(player->aimPt, player->clientNum )) {
						target = player;
						break;
					}
				}
			}
		}
		else if (eth32.settings.headbody == BODY_HEAD) {
			for (int i=0 ; i<numFrameTargets ; i++)
			{
				player = frameTargets[i];

				if (traceBody(player->aimPt, player->clientNum )) {
					target = player;
					break;
				}

				if (!target) {
					if (traceHead(&player->orHead, player->headPt, player->clientNum)) {
						VectorCopy(player->headPt, player->aimPt);
						target = player;
						break;
					}
				}
			}
		}
		else if (eth32.settings.headbody == HEAD_ONLY) {
			for (int i=0 ; i<numFrameTargets ; i++)
			{
				player = frameTargets[i];

				if (traceHead(&player->orHead, player->headPt, player->clientNum)) {
					VectorCopy(player->headPt, player->aimPt);
					target = player;
					break;
				}
			}
		}
		else if (eth32.settings.headbody == BODY_ONLY) {
			for (int i=0 ; i<numFrameTargets ; i++)
			{
				player = frameTargets[i];

				if (traceBody(player->aimPt, player->clientNum )) {
					target = player;
					break;
				}
			}
		}
	}

	// set lastTarget here, we don't want preaim targets to lock
	lastTarget = (target) ? target : NULL;

	// do preaim and preshoot stuff if we do not have a valid target yet
	if (!target && (eth32.settings.preAim || eth32.settings.preShoot)) {
		vec3_t spos;
		vec3_t ppos;

		float preshoot;
		preshoot = eth32.settings.preShootTime*0.001;
		VectorMA(eth32.cg.muzzle, preshoot, eth32.cg.predictedps->velocity, spos);
		VectorMA(eth32.cg.muzzle, eth32.settings.preAimTime*0.001, eth32.cg.predictedps->velocity, ppos);

		if (eth32.settings.preShoot) {
			for (int i=0 ; i<numFrameTargets ; i++){
				player = frameTargets[i];

				VectorMA(player->orHead.origin, preshoot, player->cent->currentState.pos.trDelta, player->orHead.origin);
				if (traceHead(&player->orHead, player->headPt, spos, player->clientNum)) {
					VectorCopy(player->headPt, player->aimPt);
					target = player;
					break;
				}

				//restore (is this necessary?)
				VectorMA(player->orHead.origin, -1.0f*preshoot, player->cent->currentState.pos.trDelta, player->orHead.origin);
			}
		}

		if (eth32.settings.preAim){
			for (int i=0 ; i<numFrameTargets ; i++){
				player = frameTargets[i];

				// use head "origin", somewhere in the backpack. This distance from this
				// point to a future aimpoint is smallest, thus minimizing spread
				VectorMA(player->orHead.origin, eth32.settings.preAimTime*0.001, player->cent->currentState.pos.trDelta, player->preAimPt);

				// point and lock, but do not fire!
				if (IsPointVisible(ppos, player->preAimPt, player->clientNum)){
					target = player;
					VectorCopy(target->orHead.origin, target->aimPt);
					LockSensitivity(true);
					Autofire(true);
					Point(eth32.cg.muzzle);
					Cam.targetClientNum = player->clientNum;
					return;
				}
			}
		}
	}
	// we have a valid (i.e. real) target - fire away
	if (target) {
		applySelfPrediction();
		applyTargPrediction();
        LockSensitivity(true);
		if (target->lf && (target->lf->frame == 1947 || target->lf->frame == 1998)){
			VectorMA(target->aimPt, eth32.settings.animCorrection, zAxis, target->aimPt);
		}
		Autocrouch(true, false);

		// Speedshooting for single shot weapons
		if ( isSingleShootWeapon() ) {
			static int lastShotTime = 0;
		if  (eth32.cg.cgTime - lastShotTime >= 30) {
				Autofire(true);
				lastShotTime = eth32.cg.cgTime;
			} else
				Autofire(false);
		} else
			Autofire(true);

		Cam.targetClientNum = target->clientNum;

		// we can improve acc even more, when first bullet fires and AC is on,
		// locally we think we're standing but server already crouched, so adjust
		if (acFirstFrame){
			eth32.cg.muzzle[2] -= 24.0f;
			Point(eth32.cg.muzzle);
		} else
			Point(eth32.cg.muzzle);
	}
	else {
		if (eth32.settings.atkValidate)
			Autofire(false);
		Cam.targetClientNum = -1;
	}
}

inline void CAimbot::applyTargPrediction()
{
 	if (eth32.settings.predTargzor)
	VectorMA(target->bodyPt, (eth32.cg.ping/2) * 0.001f, target->currentState->pos.trDelta, target->bodyPt);
	VectorMA(target->headPt, (eth32.cg.ping/2) * 0.001f, target->currentState->pos.trDelta, target->headPt);
}

inline void CAimbot::applySelfPrediction()
{
	vec3_t displ, v1, vp, p, vs;
	float dt;

	if (eth32.cg.predictedps)
		VectorCopy(eth32.cg.predictedps->velocity, vs);
	else
		VectorCopy(eth32.cg.snap->ps.velocity, vs);

	switch (eth32.settings.predSelfType)
	{
		case SPR_OFF:
			return;
		case SPR_MANUAL:
			VectorMA(target->aimPt, eth32.settings.predSelf, vs, target->aimPt);
			return;
		case SPR_PING:
			VectorMA(target->aimPt, (eth32.cg.snap->ping * -0.0001f), vs, target->aimPt);
			return;
		case SPR_LEET:
			dt = -1.0f * Engine.FrameCorrectionTime(-1);
			break;
		default:
			return;
	}

	VectorSubtract(target->aimPt, eth32.cg.muzzle, p);
	VectorNormalizeFast(p);

	VectorScale(p, DotProduct(vs, p), v1);
	VectorSubtract(vs, v1, vp);
	VectorScale(vp, dt, displ);

	VectorAdd(target->aimPt, displ, target->aimPt);
}

bool CAimbot::IsPointVisible(vec3_t start, vec3_t pt, int skipEntity)
{
	trace_t t;

	orig_CG_Trace(&t, start, NULL, NULL, pt, skipEntity, MASK_SHOT);

	return (t.fraction == 1.f);
}

void CAimbot::Point(vec3_t vieworg)
{
	vec3_t org, ang;

	// sol: this check can probably go away, since box trace is checking for invalid box origins
	if (VectorCompare(target->aimPt, vec3_origin))
		return;

	if (eth32.settings.drawBulletRail && eth32.cg.snap->ps.weaponTime < 40)
		Engine.MakeRailTrail( eth32.cg.muzzle, target->aimPt, false, eth32.settings.colorBulletRail, 40);

	// Get the vector difference and calc the angle form it
	VectorSubtract(target->aimPt, vieworg, org);
	Tools.VectorAngles(org, ang);

	// Clip & normalize the angle

	ang[PITCH] = -ang[PITCH];

	ANG_CLIP(ang[YAW]);
	ANG_CLIP(ang[PITCH]);
	AnglesToAxis(ang, eth32.cg.refdef->viewaxis);
	ang[YAW] -= eth32.cg.refdefViewAngles[YAW];
	ANG_CLIP(ang[YAW]);
	ang[PITCH] -= eth32.cg.refdefViewAngles[PITCH];
	ANG_CLIP(ang[PITCH]);
	if (ang[YAW] < -180.0 || ang[YAW] > 180.0 || ang[PITCH] < -180.0 || ang[PITCH] > 180.0)
		return;
	if (eth32.settings.aimMode == AIMMODE_HUMAN) {
		if (!eth32.settings.lockMouse) 
			LockSensitivity(false);
		while (ang[PITCH] < -360.0f)
			ang[PITCH] += 360.0f;
		while (ang[PITCH] > 360.0f)
			ang[PITCH] -= 360.0f;
		while (ang[YAW] < -360.0f)
			ang[YAW] += 360.0f;
		while (ang[YAW] > 360.0f)
			ang[YAW] -= 360.0f;
		if (ang[PITCH] > 180.0f)
			ang[PITCH] -= 360.0f;
		if (ang[PITCH] < -180.0f)
			ang[PITCH] += 360.0f;
		if (ang[YAW] > 180.0f)
			ang[YAW] -= 360.0f;
		if (ang[YAW] < -180.0f)
			ang[YAW] += 360.0f;

			float aimDiv = eth32.settings.divmax;
		if (aimDiv>(float)eth32.settings.divmin) {
			ang[PITCH] *= eth32.settings.humanValue/aimDiv;
			ang[YAW] *= eth32.settings.humanValue/aimDiv;
		}else{
			aimDiv = (float)eth32.settings.divmax;
		}
bool Moving = Syscall.isKeyActionDown("+forward")||Syscall.isKeyActionDown("+backward")||Syscall.isKeyActionDown("+moveleft")||Syscall.isKeyActionDown("+moveright");
	if(ang[PITCH] <0) ang[PITCH]*=0.02;
	if(Moving) ang[YAW]*=0.2;
	}

	// Add the angle difference to the view (i.e aim at the target)
	*(float *)eth32.game.viewAngleX += ang[YAW];
	*(float *)eth32.game.viewAngleY += ang[PITCH];
}

// best of both worlds ;)
void CAimbot::LockSensitivity(bool lock){ senslocked = lock; }
bool CAimbot::SensitivityLocked(){ return senslocked; }
void CAimbot::SetAttackButton( void *ptr ) { atkBtn = (kbutton_t *)ptr; }
void CAimbot::SetCrouchButton( void *ptr ) { crhBtn = (kbutton_t *)ptr; }

void CAimbot::Autocrouch(bool enable, bool force)
{
	static bool autoCrouching = false;
	acFirstFrame = false;

	// force release of crouch, else tapout wont work :(
	if (!enable && force) {
		crhBtn->active = crhBtn->down[0] = crhBtn->down[1] = 0;
		autoCrouching = false;
		return;
	}

	// dont bother if user is manually crouching
	if ((userCrouching && !autoCrouching) || !eth32.settings.autoCrouch){
		autoCrouching = false;
		return;
	}

	if (!autoCrouching && enable){
		// first check aimpt is still visible after crouch to prevent oscilation
		eth32.cg.muzzle[2] -= 24.0;
		if (PreCrouchTrace(eth32.cg.muzzle, target->aimPt, target->clientNum)){
			crhBtn->active = crhBtn->wasPressed = 1;
			crhBtn->down[0] = -1;
			crhBtn->downtime = *eth32.game.com_frameTime;
			autoCrouching = true;
			acFirstFrame = true;
		}
		eth32.cg.muzzle[2] += 24.0;
	} else if (autoCrouching && !enable) {
		crhBtn->active = crhBtn->down[0] = crhBtn->down[1] = 0;
		autoCrouching = false;
	}
}

void CAimbot::Autofire(bool enable)
{
	static int lastShotTime = 0;

	// don't mess with attack if not in control
	if (!autoMode)
		return;

	if (atkBtn->wasPressed || autoMode || Trigger) {
		// check for heat
		if (enable && (eth32.cg.currentWeapon->attribs & WA_OVERHEAT) && eth32.cg.snap->ps.curWeapHeat > 200)
			enable = false;

		// User has selected a delay, so do a fire and release
		if (eth32.cg.currentWeapon && eth32.cg.currentWeapon->delay && enable) {
			static int lastShotTime = 0;
			atkBtn->active = 0;
			if ((eth32.cg.cgTime - lastShotTime) >= eth32.cg.currentWeapon->delay) {
				atkBtn->wasPressed = 1;
				lastShotTime = eth32.cg.cgTime;
			}
			return;
		}

		if (enable)
			atkBtn->active = 1;			// set fire
		else
			atkBtn->active = 0;
	}
}

void CAimbot::SetUserCrouchStatus(bool crouched)  { userCrouching = crouched; }
void CAimbot::SetUserAttackStatus(bool atkPressed){ attackPressed = atkPressed; }

float CAimbot::CrosshairDistance(const player_t *player)
{
	int x_dif=0, y_dif=0;

	// Calculate obtuse & adjacent
	player->screenX > eth32.cg.centerX ? x_dif = player->screenX - eth32.cg.centerX : x_dif = eth32.cg.centerX - player->screenX;
	player->screenY > eth32.cg.centerY ? y_dif = player->screenY - eth32.cg.centerY : y_dif = eth32.cg.centerY - player->screenY;

	// Return hypotenuse
	float h2 = ((x_dif*x_dif)+(y_dif*y_dif));

	return h2;
}

int CAimbot::CheckFov(vec3_t origin)
{
	if (AIM_FOV_VALUE > 359.f)
		return 1;

	vec3_t dir;
	float ang;
	VectorSubtract( origin, eth32.cg.refdef->vieworg, dir );
	VectorNormalizeFast( dir );
	ang = 57.2957795131*acos(DotProduct( dir, eth32.cg.refdef->viewaxis[0] ));

	if (ang <= AIM_FOV_VALUE)
		return 1;
	else
		return 0;

}

