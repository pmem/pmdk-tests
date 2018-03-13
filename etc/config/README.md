# Config File #

In order to succesfully run test binary, valid `config.xml` file needs
to be placed in binary directory.

Config file consists of two main sections (xml tags): `localConfiguration` 
and `remoteConfiguration`, representing local and remote machines respectively.
There can be more than one `remoteConfiguration`.

### localConfiguration structure ###
* `testDir`: path to test execution directory
* `dimmConfiguration`: NVDIMM devices configuration section
    * `mountPoint`: path to mountpoint associated with single bus connected with
one or more NVDIMMS

### remoteConfiguration structure ###
* `testDir`: path to test execution directory on remote host
* `address`: remote host address in format ```[<user>@]<hostname>[:<port>]``` with optional user and port number. Provided address needs to be sufficient to authenticate without further quering.
* `binsDir`: path to directory with pmdk-tests binaries on remote host
* `dimmConfiguration`: same as in
[local configuration](#localConfiguration-structure)

See also: config.xml [example file](config.xml.example).
