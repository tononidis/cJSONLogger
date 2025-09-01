workspace "cJSONLogger"
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	outputdir = "%{cfg.buildcfg}"
	
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

	language "C"
	cdialect "Default"

project "cJSONLogger"
	kind "SharedLib"

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
		defines "CJSONLOGGER_DEBUG"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		defines "CJSONLOGGER_RELEASE"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:Dist"
		defines "CJSONLOGGER_DIST"
		runtime "Release"
		symbols "off"
		optimize "on"

project "cJSONLoggerExample"
	kind "ConsoleApp"

	files
	{
		"examples/example.c"
	}

	includedirs
	{
		"include"
	}

	links
	{
		"cJSONLogger"
	}

project "cJSONLoggerTests"
	kind "ConsoleApp"

	files
	{
		"tests/*.c"
	}

	includedirs
	{
		"cJSON",
		"include"
	}

	links
	{
		"cJSONLogger",
		"pthread"
	}

	filter "configurations:Debug"
		defines "CJSONLOGGER_TEST_DEBUG"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		defines "CJSONLOGGER_TEST_RELEASE"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:Dist"
		defines "CJSONLOGGER_TEST_DIST"
		runtime "Release"
		symbols "off"
		optimize "on"
