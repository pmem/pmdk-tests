# Contributing to pmdk-tests

### Code contributions

Code contributions for `pmdk-tests` are made through GitHub pull requests.

**NOTE: If you do decide to implement code changes and contribute them,
please make sure you agree your contribution can be made available
under the [BSD-style License used for the pmdk-tests](https://github.com/pmem/pmdk-tests/blob/master/LICENSE).**

**NOTE: Submitting your changes also means that you certify the following:**

```
Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

In case of any doubt, the gatekeeper may ask you to certify the above
in writing, i.e. via email or by including a `Signed-off-by:` line at the bottom
of your commit comments.

To improve tracking of who is the author of a contribution, we kindly ask you
to use your real name (not an alias) when committing your changes to the
Persistent Memory Development Kit:
```
Author: Random J Developer <random@developer.example.org>
```

General rules of code contribution can be found in
[Git Workflow](http://pmem.io/2014/09/09/git-workflow.html) article on pmem.io.
However, specific checks necessary to run before submitting pull request vary
from those described in `"Before submitting changes"` paragraph. Before making
pull request to `pmdk-tests` please make sure to successfully execute the
following steps locally:

On Linux:
```
# in build directory:
$ cmake .. -DDEVELOPER_MODE=ON
$ make
```

On Windows:
```
# in build directory:
$ cmake -G "Visual Studio 14 2015 Win64" -DDEVELOPER_MODE=ON ..
$ cmake --build .
```

### Bug reports, questions and feature requests

Issues for the `pmdk-tests` project are tracked in project
[github issues](https://github.com/pmem/pmdk-tests/issues).
