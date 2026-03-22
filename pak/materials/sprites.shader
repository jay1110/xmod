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

sprites/voiceAmmo
{
	nocompress
	nopicmip
	{
		map sprites/voiceAmmo.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/objective_blue
{
	nocompress
	nopicmip
	{
		map sprites/objective_blue.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/objective_red
{
	nocompress
	nopicmip
	{
		map sprites/objective_red.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/undercover
{
	nocompress
	nopicmip
	{
		map sprites/undercover.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

sprites/greentick
{
nocompress
nopicmip
{
clampmap sprites/greentick.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/redcross
{
nocompress
nopicmip
{
clampmap sprites/redcross.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/voicechat_orange
{
nocompress
nopicmip
{
clampmap sprites/voicechat_orange.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/medic_revive2
{
nocompress
nopicmip
{
clampmap sprites/medicrevive2.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/objective_team
{
nocompress
nopicmip
{
clampmap sprites/objective_team.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/objective_dropped
{
nocompress
nopicmip
{
clampmap sprites/objective_dropped.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/objective_enemy
{
nocompress
nopicmip
{
clampmap sprites/objective_enemy.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/objective_both_te
{
nocompress
nopicmip
{
clampmap sprites/objective_both_te.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/objective_both_td
{
nocompress
nopicmip
{
clampmap sprites/objective_both_td.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/objective_both_de
{
nocompress
nopicmip
{
clampmap sprites/objective_both_de.tga
blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
rgbGen vertex
}
}

sprites/cm_medic_icon
{
nopicmip
nocompress
nomipmaps
{
clampmap sprites/voicemedic.tga
depthFunc equal
blendfunc blend
rgbGen vertex
alphaGen vertex
}
}

sprites/cm_medic_revive
{
nopicmip
nocompress
nomipmaps
{
clampmap sprites/medicrevive2.tga
depthFunc equal
blendfunc blend
rgbGen vertex
alphaGen vertex
}
}

sprites/cm_voicechat_icon
{
nopicmip
nocompress
nomipmaps
{
clampmap sprites/voicechat.tga
depthFunc equal
blendfunc blend
rgbGen vertex
alphaGen vertex
}
}

sprites/cm_voicechat_orange_icon
{
nopicmip
nocompress
nomipmaps
{
clampmap sprites/voicechat_orange.tga
depthFunc equal
blendfunc blend
rgbGen vertex
alphaGen vertex
}
}

sprites/cm_friendlycross
{
nopicmip
nocompress
nomipmaps
{
clampmap gfx/2d/friendlycross.tga
depthFunc equal
blendfunc blend
rgbGen vertex
alphaGen vertex
}
}
