workspace "cJSONLogger"
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	outputdir = "%{cfg.buildcfg}"

project "cJSONLogger"
	kind "SharedLib"
	language "C"
	cdialect "Default"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/*.c",
		"cJSON/cJSON.c"
	}

	includedirs
	{
		"cJSON",
		"include"
	}

	links
	{
		"pthread"
	}

	filter "configurations:Debug"
		defines "DEBUG"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		defines "RELEASE"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:Dist"
		defines "DIST"
		runtime "Release"
		symbols "off"
		optimize "on"

project "cJSONLoggerExample"
	kind "ConsoleApp"
	language "C"
	cdialect "Default"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"example.c"
	}

	includedirs
	{
		"include"
	}

	links
	{
		"cJSONLogger"
	}
