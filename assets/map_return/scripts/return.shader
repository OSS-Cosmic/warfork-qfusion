textures/return/manholecover_decal
{
	qer_editorimage textures/return/manholecover_decal.png
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
	qer_alphaFunc gequal 0.5
	polygonoffset

	{
		map textures/return/manholecover_decal.png
		alphaFunc GE128
		depthWrite
	}

	{
		map $lightmap
		blendFunc filter
		depthFunc equal
	}
}

textures/return/manholecover_rect
{
	qer_editorimage textures/return/manholecover_rect.png
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
	qer_alphaFunc gequal 0.5
	polygonoffset

	{
		map textures/return/manholecover_rect.png
		alphaFunc GE128
		depthWrite
	}

	{
		map $lightmap
		blendFunc filter
		depthFunc equal
	}
}

textures/return/manholecover_small
{
	qer_editorimage textures/return/manholecover_small.png
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
	qer_alphaFunc gequal 0.5
	polygonoffset

	{
		map textures/return/manholecover_small.png
		alphaFunc GE128
		depthWrite
	}

	{
		map $lightmap
		blendFunc filter
		depthFunc equal
	}
}

textures/return/hurryup
{
	qer_editorimage textures/return/hurryup.png
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
	qer_alphaFunc gequal 0.5
	polygonoffset

	{
		map textures/return/hurryup.png
		alphaFunc GE128
		depthWrite
	}

	{
		map $lightmap
		blendFunc filter
		depthFunc equal
	}
}

textures/return/wth1
{	
	qer_editorimage textures/return/wth1.png
	q3map_lightimage textures/return/wth1.png
  	q3map_surfacelight 500
	q3map_lightsubdivide 128
	surfaceparm nomarks
	surfaceparm nodlight
	q3map_forcemeta
	surfaceparm nolightmap

	{
		map textures/return/wth1.png
	}
}

textures/return/paint-2023
{
	qer_editorimage textures/return/paint-2023.png
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
	qer_alphaFunc gequal 0.5
	polygonoffset

	{
		map textures/return/paint-2023.png
		alphaFunc GE128
		depthWrite
	}

	{
		map $lightmap
		blendFunc filter
		depthFunc equal
	}
}

textures/return/tooslow
{
	qer_editorimage textures/return/tooslow.png
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
	qer_alphaFunc gequal 0.5
	polygonoffset

	{
		map textures/return/tooslow.png
		alphaFunc GE128
		depthWrite
	}

	{
		map $lightmap
		blendFunc filter
		depthFunc equal
	}
}


