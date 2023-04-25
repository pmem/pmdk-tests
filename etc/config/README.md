# Config File #

In order to succesfully run test binary, valid `config.xml` file needs
to be placed in binary directory.
Config file consists of multiple main sections (xml tags):
* `localConfiguration`: used for tests run on local machine 
* `rasConfiguration`: used for RAS tests by test controller machine

### localConfiguration structure ###
* `testDir`: path to test execution directory. If `dimmConfiguration` section
is defined, it should represent a mountpoint of non-NVDIMM device.
* `dimmConfiguration`: NVDIMM devices configuration section
	* `mountPoint`: path to mountpoint associated with single bus connected with
one or more NVDIMMS

### rasConfiguration structure ###
* `DUT` - node representing single testing machine managed by controller
	* `address`: DUT address
	* `powerCycleCommand`: command triggering DUT power cycle
	* `binDir`: DUT test binaries directory

See also: config.xml [example file](config.xml.example).
