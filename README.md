pmdk-tests: Tests for [Persistent Memory Development Kit](https://github.com/pmem/pmdk)
=================================

This repository utilizes [Google Test](https://github.com/google/googletest) C++ test framework and [pugiXML](https://github.com/zeux/pugixml).

### Building The Source ###
To build pmdk-tests on Linux, the following packages are required:
* **[PMDK](https://github.com/pmem/pmdk)**
* **CMake - version 2.8.12 or greater**

On Windows, the following packages are required:
* **[PMDK](https://github.com/pmem/pmdk)**
* **CMake - version 3.1 or greater**
* **MS Visual Studio 2015**
* [Windows SDK 10.0.14393](https://developer.microsoft.com/pl-pl/windows/downloads/sdk-archive) (or later)
* **PowerShell 5**

#### Building pmdk-tests on Linux ####
In the pmdk-tests root directory:
```
	$ mkdir build
	$ cd build
	$ cmake .. <args>
	$ make
```
`<args>` - for building debug version use `-DCMAKE_BUILD_TYPE=DEBUG`. No `-DCMAKE_BUILD_TYPE` argument provided is equivalent to `-DCMAKE_BUILD_TYPE=RELEASE`.

For building a specific group of tests provide its target binary name:
```
	$ make PMEMPOOLS
```

##### PMDK Custom path
If PMDK is installed in custom path, then additional arguments need to be specified.
If pkg-config is available, then PKG_CONFIG_PATH environmental variable needs to be set to <PMDK_INSTALL_PATH>/lib/pkgconfig.
```
	$ PKG_CONFIG_PATH=<PMDK_INSTALL_PATH>/lib/pkgconfig cmake ..
```
If it's not available, then CMAKE_PREFIX_PATH needs to be specified. Relative paths are not supported with CMAKE_PREFIX_PATH.
```
	$ cmake .. -DCMAKE_PREFIX_PATH=<PMDK_INSTALL_PATH>
```

#### Building pmdk-tests on Windows ####
Environment variables should be set according to [PMDK Windows installation guide](https://github.com/pmem/pmdk/tree/master/src/windows/setup#pmdk-for-windows-installation).

In the pmdk-tests root directory:
```
	$ mkdir build
	$ cd build
	$ cmake -G "Visual Studio 14 2015 Win64" ..
```
CMake sets 32-bit build as default, however pmdk-tests works only in 64-bit environment.
PMDK supports only Visual Studio 2015 on Windows, hence `-G "Visual Studio 14 2015 Win64"`.
To build binaries use either generated `pmdk_tests.sln` solution file with Visual Studio or:
```
	$ cmake --build . --config <config> [--target <target>]
```
* `<config>` - `Debug` or `Release`

* `<target>` - specified group of tests to build, e.g. `PMEMPOOLS`

### Running Tests ###
Before executing tests, valid configuration `config.xml` file needs to be placed in the same directory as the test binary. Template `config.xml.example` is located in `etc/config` directory. After this setup, tests can be run simply by executing the binary:

```
	$ cd build
	$ cp ../etc/config/config.xml.example config.xml
	# Set own values in config.xml fields
	$ ./PMEMPOOLS
```
pmdk-tests are implemented using Google Test framework, and thus resulting binaries share its behavior and command line interface.
To list all tests to be run from specific binary:
```
	$ ./PMEMPOOLS --gtest_list_tests
```
To run a subset of tests:
```
	# Runs only tests with 'VERBOSE' in title.
	$ ./PMEMPOOLS --gtest_filter="*VERBOSE*"
```
`--gtest_filter` argument can also be used to exclude tests from execution (mind the `-` before filtered out phrase):
```
	# Exclude tests with 'VERBOSE' in title from execution.
	$ ./PMEMPOOLS --gtest_filter=-"*VERBOSE*"
```
To see usage:
```
	$ ./PMEMPOOLS --help
```
For more information about running tests see [Google Test documentation](https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#running-test-programs-advanced-options).

#### Running tests with run_tests.py script ####
Executing binary through `run_tests.py` script located in `etc/scripts` ensures that whole scope of tests will be run. In case of premature termination, execution will be resumed after the last ran test.

Running tests in `PMEMPOOLS` binary with 15 minutes timeout and tests with `VERBOSE` in name excluded from execution:
```
$ cd build
$ ../etc/scripts/run_tests.py -b ./PMEMPOOLS --timeout 15 -e "*VERBOSE*"
```

### Other Requirements ###
Python scripts in pmdk-tests require Python 3.6.
