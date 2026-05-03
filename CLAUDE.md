# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

C++ library for NVMe device I/O on Linux. Uses the kernel's NVMe ioctl interface (`linux/nvme_ioctl.h`) — no external dependencies required.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the example (requires root or appropriate device permissions):
```bash
sudo ./build/basic_io /dev/nvme0
```

## Architecture

```
include/nvme_device.hpp   — Public API: NVMeDevice, NVMeNamespace, IdentifyController, IdentifyNamespace
src/nvme_device.cpp       — Implementation: admin passthru, LBA I/O, block device I/O
examples/basic_io.cpp     — Demo: identify controller/namespace, read first 4 KiB
```

**NVMeDevice** (`/dev/nvmeX`) — wraps the controller fd; issues admin commands via `NVME_IOCTL_ADMIN_CMD`. Key methods: `identify_controller()`, `list_namespaces()`, `open_namespace()`.

**NVMeNamespace** (`/dev/nvmeXnY`) — wraps a namespace fd opened with `O_DIRECT`. Two I/O paths:
- `read_lba` / `write_lba` — native NVMe I/O via `NVME_IOCTL_SUBMIT_IO` (LBA addressing)
- `read` / `write` — POSIX `pread`/`pwrite` (byte offset addressing)

Buffers for `O_DIRECT` I/O must be page-aligned (`posix_memalign`, 4096-byte alignment).

## Notes

- Linux only (`linux/nvme_ioctl.h`). Requires kernel NVMe driver (`nvme`, `nvme-core`).
- Admin commands need the controller device (`/dev/nvme0`); I/O commands need the namespace device (`/dev/nvme0n1`).
- `nlb` in `read_lba`/`write_lba` is 0-based (NVMe spec): `nlb=0` transfers 1 block.
