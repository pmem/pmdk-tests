Reliability, Availability and Serviceability (RAS) tests
=================================
RAS tests cover PMDK features utilizing NVDIMMs RAS mechanisms. These tests
need to be run on machine with NVDIMM hardware described by
appropriate [config.xml](../../../etc/config/README.md) file.

### Unsafe shutdown tests ###
Unsafe shutdown tests (compiled into ```UNSAFE_SHUTDOWN``` binary) involve
machine power cycle during test. Because of that, these tests need to be run
with use of remote controller machine that can connect through ssh, without
being prompted for authentication, with root access user on test machine.
```US_TEST_CONTROLLER``` is utilized for this purpose. This binary needs
`config.xml` file with defined `rasConfiguration` section. Scope of ran tests
can be controlled by the `--gtest_filter` option which is passed down
to test binaries on DUTs.

```
$ ./US_TEST_CONTROLLER [--gtest_filter=filter]
```

Example of running US tests:
```
$ ./UNSAFE_SHUTDOWN_LOCAL 1 inject all --gtest_output=xml:{{ logs_dir_path }}/phase1.xml
```
After reboot:
```
$ ./UNSAFE_SHUTDOWN_LOCAL 2 cleanup all --gtest_output=xml:{{ logs_dir_path }}/phase2.xml
```
### Dependencies ###
* [ndctl](https://github.com/pmem/ndctl) - version 60.0 or greater
