# VAST LD_PRELOAD Library

**LD_PRELOAD library to inject O_DIRECT into file I/O**

This library intercepts open() calls of the C standard library via the LD_PRELOAD mechanism and injects the O_DIRECT flag for files that match the name pattern given in the config file. This mechanism has the advantage that the source code of the actual application does not need to be modified to take advantage of O_DIRECT and that O_DIRECT can also be used in cases where the application could otherwise not specify O_DIRECT, e.g. in case of MPI-IO.

## Table of Contents

<details>
  <summary><b>(click to expand)</b></summary>

- [Usage](#usage)
  - [Simple Usage Example](#simple-usage-example)
  - [MPI-IO Usage Example](#mpi-io-usage-example)
    - [OpenMPI](#openmpi)
- [Build Prerequisites](#build-prerequisites)
  - [Dependencies for Debian/Ubuntu](#dependencies-for-debian-ubuntu)
  - [Dependencies for RHEL/CentOS](#dependencies-for-rhel-centos)
- [Build & Install](#build-install)

</details>

## Usage

The preload library is activated on a per-application basis through the LD_PRELOAD environment variable and reads two environment variables for configuration:

- `LD_VAST_PATHFILE` [*mandatory*]:
  The path to a text file containing a list of newline-separated paths for which O_DIRECT should be injected. Wildcards "*" and "?" can be used.
  - The asterisk also matches slashes and thus subdirectories, e.g. `/data/*.xyz` also matches `/data/dir1/dir2/myfile.xyz`
- `LD_VAST_LOG_TOPICS` [*optional*]:
  Enables console logging based on the flags defined as `ENV_LOG_TOPIC_...` in `source/Logger.h`. E.g.  use `LD_VAST_LOG_TOPICS=2` to print paths for which O_DIRECT gets injected. Or use `LD_VAST_LOG_TOPICS=-1` to enable most verbose logging.

### Simple Usage Example

Let's assume we have an application `myapp` and we know that this application reads several large files in the directory `/data/myapp/input`, which we want to speed up through injection of O_DIRECT.

For that, we first create a config file containing the corresponding path:

```bash
echo "/data/myapp/input/*" > /data/myapp/vastpreload.paths
```

Now we're ready to run the application as usual, just with LD_PRELOAD and the LD_VAST_PATHFILE environment variables prepended. (The path here assumes that the library has been installed to /usr/lib, e.g. via an .rpm or .deb package.)

```bash
LD_PRELOAD=/usr/lib/libvastpreload.so LD_VAST_PATHFILE=/data/myapp/vastpreload.paths ./myapp`
```

*Note*: If we wanted to see where O_DIRECT gets injected, we could also add `LD_VAST_LOG_TOPICS=2`.

### MPI-IO Usage Example

In this example, we use the I/O benchmarking tool [ior](https://github.com/hpc/ior/) together with **MPICH** to show how the preload library can be used for applications based on MPI-IO.

*Note*: Typical MPI-IO implementations (including ROMIO, which is used by MPICH) detect when they access files on an NFS mountpoint and then add POSIX file lock calls around all read & write operations to achieve coherence between different nodes. This locking is often unnecessary, e.g. when the nodes are reading a large input dataset or when they are writing to different output files.
Especially when O_DIRECT is used, any locking for coherence is unnecessary, because O_DIRECT bypasses caching and thus makes accesses automatically coherent.
MPICH provides the `ROMIO_FSTYPE_FORCE="ufs"` option to disable the unnecessary locking, which we will use here. This selects the ROMIO ufs (Universal Filesystem) driver instead of the nfs driver.

*Note*: By default, ior generates highly compressible data. Recent versions of ior have a new `-l random` option to make data less compressible, which we will use here.

We assume that the preload library has been installed on the system through an rpm/deb package or through `make install` to `/usr/lib` by a sysadmin, but it can generally also be used from other paths to avoid the need for root privileges.

First, we need to prepare a path file to configure for which paths O_DIRECT should be injected. This file needs to be in a location where all participating hosts can access it, so we just put it into our shared mountpoint `/mnt/vast`.

```bash
mkdir /mnt/vast/ior
echo '/mnt/vast/ior/*' > /mnt/vast/ior/vastpreload.paths
```

Now we're ready to run the actual `ior` command via mpirun. The LD_PRELOAD variable and corresponding preload library config options get transferred to the other hosts via the `-env` argument of mpirun.

```bash
ROMIO_FSTYPE_FORCE="ufs:/mnt/vast/ior" mpirun -env LD_VAST_PATHFILE=/mnt/vast/ior/vastpreload.paths -env LD_PRELOAD=/usr/lib/libvastpreload.so -env LD_VAST_LOG_TOPICS=2 --hostfile myhosts -np 16 ./ior -w -r -i 1 -t 1m -b 2g -g -k -K -F -l random -o /mnt/vast/ior/myfile -a MPIIO
```

In this example, we enabled extra logging through the optional `LD_VAST_LOG_TOPICS=2` setting, so we will now see corresponding messages appear on the console about O_DIRECT being injected by the different MPI processes:

```bash
[...]
vastpreload: Injecting O_DIRECT. fd: 5; path: /mnt/vast/ior/myfile.0001
vastpreload: Injecting O_DIRECT. fd: 5; path: /mnt/vast/ior/myfile.0002
[...]
```

#### OpenMPI

- For OpenMPI, the environment variables are set via `-x name=value`.
- To disable automatic MPI-IO locking for NFS access, OpenMPI provides the `fs_ufs_lock_algorithm 1` option.

Our example above would look like this with OpenMPI:

```bash
mpirun --mca fs_ufs_lock_algorithm 1 -x LD_VAST_PATHFILE=/mnt/vast/ior/vastpreload.paths -x LD_PRELOAD=/usr/lib/libvastpreload.so -x LD_VAST_LOG_TOPICS=2 --hostfile myhosts -np 16 ./ior -w -r -i 1 -t 1m -b 2g -g -k -K -F -l random -o /mnt/vast/ior/myfile -a MPIIO
```

## Build Prerequisites

Building the VAST LD_PRELOAD Library requires a C++14 compatible compiler, such as gcc version 5.x or higher.

### Dependencies for Debian/Ubuntu

```bash
sudo apt install build-essential debhelper devscripts fakeroot git libboost-filesystem-dev lintian
```

### Dependencies for RHEL/CentOS

```bash
sudo yum install boost-devel gcc-c++ git make rpm-build
```

#### On RHEL / CentOS 7.x: Prepare Environment with newer gcc Version

Skip these steps on RHEL / CentOS 8.0 or newer.

```bash
sudo yum install centos-release-scl # for CentOS
# ...or alternatively for RHEL: yum-config-manager --enable rhel-server-rhscl-7-rpms
sudo yum install devtoolset-8
scl enable devtoolset-8 bash # alternatively: source /opt/rh/devtoolset-8/enable
```

The `scl enable` command enters a shell in which the environment variables are pointing to a newer gcc version. (The standard gcc version of the system remains unchanged.) Use this shell to run `make` later. The resulting executable can run outside of this shell.

## Build & Install

Start by cloning the main repository:

```bash
git clone https://github.com/vast-data/vast-preload-lib.git
cd vast-preload-lib
```

`make help` will show you all build and rpm/deb packaging options.

This is the standard build command:

```bash
make -j $(nproc)
```

You can use the library directly from the bin subdir (`bin/libvastpreload.so`), but you probably want to run `make rpm` or `make deb` now to build a package and install it on several hosts via yum/apt. On Ubuntu, run this:

```bash
make deb
sudo apt install ./packaging/vastpreload*.deb
```

**There you go. Happy streaming!**
