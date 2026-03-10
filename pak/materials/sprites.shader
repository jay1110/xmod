// Sprite shaders for objective indicators.
// Using materials/ folder (ET:Legacy approach) so these definitions take
// priority over the vanilla scripts/sprites.shader without overwriting it.

sprites/waypoint_attack
{
	nocompress
	nopicmip
	{
		map sprites/attack.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

sprites/waypoint_attack_compass
{
	nocompress
	nopicmip
	{
		map sprites/attack.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphagen wave sin .6 .5 .25 .25
	}
}

sprites/waypoint_defend
{
	nocompress
	nopicmip
	{
		map sprites/defend.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

sprites/waypoint_defend_compass
{
	nocompress
	nopicmip
	{
		map sprites/defend.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphagen wave sin .6 .5 .25 .25
	}
}

sprites/waypoint_regroup
{
	nocompress
	nopicmip
	{
		map sprites/regroup.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

sprites/waypoint_regroup_compass
{
	nocompress
	nopicmip
	{
		map sprites/regroup.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphagen wave sin .6 .5 .25 .25
	}
}

sprites/construct
{
	nocompress
	nopicmip
	{
		map sprites/construct.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/destroy
{
	nocompress
	nopicmip
	{
		map sprites/destroy.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/escort
{
	nocompress
	nopicmip
	{
		map sprites/escort.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}
