// Sprite shaders — placed in materials/ folder (ET:Legacy approach) so these
// definitions take priority over the vanilla scripts/sprites.shader without
// overwriting it.  Only shaders whose names are absent from, or differ in
// important ways from, vanilla ET pak0.pk3 are listed here.

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

// -------------------------------------------------------------------------
// General-purpose sprites that vanilla ET defines in scripts/ but that we
// re-define here to add rgbgen vertex / animation support, matching
// ET:Legacy's materials/sprites.shader approach.
// -------------------------------------------------------------------------

sprites/buddy
{
	nocompress
	nopicmip
	{
		map sprites/buddy.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphagen wave sin .6 .5 .75 .25
	}
}

sprites/medic_revive
{
	nocompress
	nopicmip
	{
		map sprites/medicrevive.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/objective
{
	nocompress
	nopicmip
	{
		map sprites/objective.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/voiceChat
{
	nocompress
	nopicmip
	{
		map sprites/voiceChat.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/voiceMedic
{
	nocompress
	nopicmip
	{
		map sprites/voiceMedic.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}
