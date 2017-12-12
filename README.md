pmdk-tests: Tests for [Non-Volatile Memory Library](https://github.com/pmem/nvml)
=================================

We use [Google Test](https://github.com/google/googletest) C++ test framework and [pugiXML](https://github.com/zeux/pugixml).

### Building The Source ###

To build pmdk-tests on Linux, following packages are required:
* **Cmake - version 2.8.12 or greater**
* **[NVML](https://github.com/pmem/nvml)**

On Windows, following packages are required:
* **Cmake - version 3.1 or greater**
* **[NVML](https://github.com/pmem/nvml)**
* **MS Visual Studio 2015**
* [Windows SDK 10.0.14393](https://developer.microsoft.com/pl-pl/windows/downloads/sdk-archive) (or later)
* **PowerShell 5**

You can use ```make-project.sh``` to generate Makefiles (on Linux) or solution files (on Windows) in default ```build``` directory using cmake. Detailed step-by-step instructions are described below.
#### Building pmdk-tests on Linux ####
In the pmdk-tests root directory:
```
	$ mkdir build
	$ cd build
	$ cmake .. <args>
	$ make
```
`<args>` - if you want to build debug version use `-DCMAKE_BUILD_TYPE=DEBUG`. No `-DCMAKE_BUILD_TYPE` argument provided is equivalent to `-DCMAKE_BUILD_TYPE=RELEASE`.

If you want to build specific group of tests provide its target binary name:
```
	$ make PMEMPOOLS
```

#### Building pmdk-tests on Windows ####
Set following environment variables:

```PMDKDebug``` - nvml debug build directory path, e.g. ```C:\[your path]\nvml\src\x64\Debug\```

```PMDKRelease``` - nvml release build directory path, e.g. ```C:\[your path]\nvml\src\x64\Relase\```

```PMDKInclude``` - nvml include path, e.g. ```C:\[your path]\nvml\src\include\```

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
`<config>` - `Debug` or `Release`

`<target>` - specified group of tests to build, e.g. `PMEMPOOLS`

### Running Tests ####

Before executing tests you need to provide valid configuration file named `config.xml` and place it in the same directory as the test binary. Template `config.xml` is located in `etc/config` directory. After this setup, tests can be run simply by executing the binary:

```
	$ cd build
	$ cp ../etc/config/config.xml .
	# Fill template config.xml fields
	$ PMEMPOOLS
```
pmdk-tests are implemented with use of Google Test framework, and thus resulting binaries share its behavior and command line interface.
To list all tests to be run from specific binary:
```
	$ PMEMPOOLS --gtest_list_tests
```
To run a subset of tests:
```
	# Runs only tests with 'POSITIVE' in title.
	$ PMEMPOOLS --gtest_filter=*POSITIVE*
```
`--gtest_filter` argument can also be used to exclude tests from execution (mind the `-` before filtered out phrase):
```
	# Exclude tests with 'POSITIVE' in title from execution.
	$ PMEMPOOLS --gtest_filter=-*POSITIVE*
```
To see usage:
```
	$ PMEMPOOLS --help
```
For more information about running tests see Google Test  [documentation](https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#running-test-programs-advanced-options).

### Other Requirements ###

Python scripts in pmdk-tests repository require Python 3.6 version.