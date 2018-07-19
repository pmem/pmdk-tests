# Config File #

In order to succesfully run test binary, valid `config.xml` file needs
to be placed in binary directory.
Config file consists of multiple main sections (xml tags):
* `localConfiguration`: used for tests run on local machine 
* `remoteConfiguration`: used for tests that need additional remote machines 
(mainly for tests using pools with remote replicas). There can be more than one
```remoteConfiguration``` node.
* `rasConfiguration`: used for RAS tests by test controller machine

### localConfiguration structure ###
* `testDir`: path to test execution directory. If `dimmConfiguration` section
* is defined, it should represent a mountpoint of non-NVDIMM device.
* `dimmConfiguration`: NVDIMM devices configuration section
    * `mountPoint`: path to mountpoint associated with single bus connected with
one or more NVDIMMS

### remoteConfiguration structure ###
* `testDir`: path to test execution directory on remote host. If
`dimmConfiguration` section is defined, it should represent a mountpoint of
non-NVDIMM device.
* `address`: remote host address in format `[<user>@]<hostname>[:<port>]`
with optional user and port number. Provided address needs to be sufficient to
authenticate without further quering.
* `binsDir`: path to directory with pmdk-tests binaries on remote host
* `dimmConfiguration`: same as in
[local configuration](#localConfiguration-structure)

### rasConfiguration structure ###
* `DUT` - node representing single testing machine managed by controller
    * `address`: DUT address
    * `powerCycleCommand`: command triggering DUT power cycle
    * `binDir`: DUT test binaries directory


See also: config.xml [example file](config.xml.example).
