# cJSONLogger
This is a thread safe C logger, designed to output logs in a JSON format.

## Why log in JSON format?
I can think one good reason.

Easy parsing and automation.

JSON parsers exist on most programming languages and tools.

Making it easy to automate evaluations of your application logs during testing.

## Dependencies
The cJSONLogger uses the [cJSON](https://github.com/DaveGamble/cJSON) parser as submodule.

## Usage
The logging interface can be found at include/cJSONLogger.h

First init the cJSONLogger by using the cJSONLoggerInit function.

Then use the CJSON_LOG_* macros to log your data.

```
cJSONLoggerInit(CJSON_LOG_LEVEL_INFO, "log.json");
char* jsonNodes[] = { "foo", "bar" };
CJSON_LOG_WARN(jsonNodes, "value %d", 4);
cJSONLoggerDump();
```

Output at log.json
```
{
	"foo":	{
		"bar":	{
			"logs":	[{
					"Time":	"yyyy-mm-dd h:m:s.ns",
					"LogLevel":	"WARN",
					"FileName":	"<file name>",
					"FuncName":	"<function name>",
					"FileLine":	<line>,
					"Log":	"value 4"
				}]
		}
	}
}
```

More examples can be viewed at examples/*

## Some caveats
Due to the nature of standard logging always appending logs at the end of a file. It is very difficult to do the same with a JSON file.

Instead the cJSONLogger stores the logs in memory using the cJSON module.

The logs persist either when the application exits normally or with a cJSONLoggerDump or cJSONLoggerRotate function call.

This way we minimize writing to disk every time

However this adds the penalty of an application consuming slightly more memory.

However a JSON logger by its nature is designed for debugging and testing builds, so the memory sideeffect often times is negligible.

Automatic mechanisms to avoid a potential scenario of running out of memory exist and are detailed on the features tab.

## Features

### Thread safe
The cJSONLogger is safe to use when your application is multithreaded.

### Automatic file rotation
When to many line of logs are written (MAX_LOG_COUNT = 500).

The log file rotates and stores them on disk with a h_m_s_ns_<file_name> format.

After MAX_LOG_ROTATION_FILES = 5 files are created the older file is removed in order to avoid filling up the disk.

If application need to rotate logs earlier it can be done with cJSONLoggerRotate function call.

If different config is needed the change the mentioned macros at src/cJSONLogger.c and re-build.

## Building
The cJSONLogger can be used either as a header only lib by adding to your codebase the files at include/* and src/*

Or by building as a shared lib (or static by editing the premake5.lua) with the [premake](https://premake.github.io) tool and adding the include/* files to your codebase.

```
premake gmake
make config=dist cJSONLogger
```
To build other configurations or the examples and tests
```
make config=<debug | release | dist> <cJSONLogger | cJSONLoggerExample | cJSONLoggerTests>
```

## Docs
Use the doxygen tool to generate the code documentation.
```
doxygen docs/Doxyfile
```
