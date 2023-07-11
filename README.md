pmdk-tests: Tests for [Persistent Memory Development Kit](https://github.com/pmem/pmdk)
=================================

> NOTICE:
This repository is not actively developed. At this moment only the RAS tests are supported. If you are looking for tests
for the PMDK repository, go to [this page](https://github.com/pmem/pmdk).

This repository utilizes [Google Test](https://github.com/google/googletest) C++ test framework and [pugiXML](https://github.com/zeux/pugixml).

### Building The Source ###
To build pmdk-tests, the following packages are required:
* **[PMDK](https://github.com/pmem/pmdk)**
* **CMake - version 2.8.12 or greater**

#### Building pmdk-tests ####
In the pmdk-tests root directory:
```
	$ mkdir build
	$ cd build
	$ cmake .. <args>
	$ make
```
`<args>` - for building debug version use `-DCMAKE_BUILD_TYPE=DEBUG`. No `-DCMAKE_BUILD_TYPE` argument provided is equivalent to `-DCMAKE_BUILD_TYPE=RELEASE`.

##### PMDK Custom path
If PMDK is installed in custom path, then additional arguments need to be specified.
If pkg-config is available, then PKG_CONFIG_PATH environmental variable needs to be set to <PMDK_INSTALL_PATH>/lib/pkgconfig.
```
	$ PKG_CONFIG_PATH=<PMDK_INSTALL_PATH>/lib/pkgconfig cmake ..
```
If it's not available, then PMDK_INSTALL_PATH needs to be specified.
PMDK_INSTALL_PATH must be an absolute path.
```
	$ cmake .. -DPMDK_INSTALL_PATH=<PMDK_INSTALL_PATH>
```

### Running Tests ###
> NOTICE:
Currently only RAS tests are supported. Please check this [README](src/tests/ras/README.md) file for more information about RAS.

Before executing tests, valid configuration `config.xml` file needs to be placed in the same directory as the test binary. Template `config.xml.example` is located in `etc/config` directory. For more information see dedicated [README](etc/config/README.md) file.
After this setup, tests can be run simply by executing the binary:

```
	$ cd build
	$ cp ../etc/config/config.xml.example config.xml
	# Set own values in config.xml fields
	$ ./UNSAFE_SHUTDOWN_LOCAL
```
pmdk-tests are implemented using Google Test framework, and thus resulting binaries share its behavior and command line interface.
To list all tests to be run from specific binary:
```
	$ ./UNSAFE_SHUTDOWN_LOCAL --gtest_list_tests
```
To run a subset of tests:
```
	# Runs only tests with 'VERBOSE' in title.
	$ ./UNSAFE_SHUTDOWN_LOCAL --gtest_filter="*VERBOSE*"
```
`--gtest_filter` argument can also be used to exclude tests from execution (mind the `-` before filtered out phrase):
```
	# Exclude tests with 'VERBOSE' in title from execution.
	$ ./UNSAFE_SHUTDOWN_LOCAL --gtest_filter=-"*VERBOSE*"
```
To see usage:
```
	$ ./UNSAFE_SHUTDOWN_LOCAL --help
```
For more information about running tests see [Google Test documentation](https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#running-test-programs-advanced-options).

### Other Requirements ###
Python scripts in pmdk-tests are compatible with Python 3.4.

### See also ###
Detailed documentation for specific test groups:
* [Reliability, Availability and Serviceability
(RAS)](src/tests/ras/README.md)
