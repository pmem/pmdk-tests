Reliability, Availability and Serviceability (RAS) tests
=================================
RAS tests cover PMDK features utilizing NVDIMMs RAS mechanisms. These tests
need to be run on machine with NVDIMM hardware described by
appropriate [config.xml](../../../etc/config/README.md) file.

### Dependencies ###
* [ndctl](https://github.com/pmem/ndctl) - version 60.0 or greater
