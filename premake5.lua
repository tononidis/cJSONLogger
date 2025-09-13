workspace "cJSONLogger"
	configurations
	{
		"debug",
		"release",
		"dist"
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

	buildoptions { "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Wuninitialized", "-Wunreachable-code", "-Wconversion", "-Wsign-conversion", "-fstack-protector-strong" }

	filter "configurations:debug"
		defines "CJSONLOGGER_DEBUG"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:release"
		defines "CJSONLOGGER_RELEASE"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:dist"
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
		"tests/test.c"
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

	filter "configurations:debug"
		defines "CJSONLOGGER_TEST_DEBUG"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:release"
		defines "CJSONLOGGER_TEST_RELEASE"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:dist"
		defines "CJSONLOGGER_TEST_DIST"
		runtime "Release"
		symbols "off"
		optimize "on"

project "cJSONLoggerMultiThreadTests"
	kind "ConsoleApp"

	files
	{
		"tests/multithread_tests.c"
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

	filter "configurations:debug"
		defines "CJSONLOGGER_MULTITHREAD_TEST_DEBUG"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:release"
		defines "CJSONLOGGER_MULTITHREAD_TEST_RELEASE"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:dist"
		defines "CJSONLOGGER_MULTITHREAD_TEST_DIST"
		runtime "Release"
		symbols "off"
		optimize "on"
