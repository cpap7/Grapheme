workspace "Grapheme"
    configurations 
	{ 
		"Debug", 
		"Release", 
	--	"Dist"
	}
    platforms { "x64" }

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	startproject "Grapheme-App"

    filter {}
	
include "External.lua"

include "Grapheme-App"
include "Grapheme-Core"
