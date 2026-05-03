#include "nvme_device.h"

#include <cstdio>
#include <cstring>
#include <array>

int main() {
    try {
        nvme::NVMeDevice dev("Fake NVMe Device", 64ULL * 1024 * 1024); // 64 MiB

        // Identify controller
        auto ctrl = dev.identify_controller();
        char mn[41]{}, sn[21]{};
        std::memcpy(mn, ctrl.mn, 40);
        std::memcpy(sn, ctrl.sn, 20);
        std::printf("Controller : %s\n", dev.name().c_str());
        std::printf("  Model    : %.40s\n", mn);
        std::printf("  Serial   : %.20s\n", sn);
        std::printf("  Firmware : %.8s\n", ctrl.fr);

        // Namespace info
        auto nsids = dev.list_namespaces();
        auto ns    = dev.open_namespace(nsids[0]);
        std::printf("\nNamespace %u:\n", ns.nsid());
        std::printf("  Block size    : %u bytes\n", ns.block_size());
        std::printf("  Block count   : %llu\n", (unsigned long long)ns.block_count());
        std::printf("  Capacity      : %llu MiB\n",
                    (unsigned long long)(ns.capacity_bytes() / (1024 * 1024)));

        // Write pattern to LBA 0
        std::array<uint8_t, 512> wbuf{};
        for (int i = 0; i < 512; ++i) wbuf[i] = static_cast<uint8_t>(i & 0xFF);
        ns.write_lba(wbuf.data(), 0, 0); // nlb=0 → 1 block

        // Read back and verify
        std::array<uint8_t, 512> rbuf{};
        ns.read_lba(rbuf.data(), 0, 0);

        bool ok = (wbuf == rbuf);
        std::printf("\nWrite→Read verify: %s\n", ok ? "PASS" : "FAIL");

        // Byte-addressed I/O
        const char msg[] = "hello nvme";
        ns.write(msg, sizeof(msg), 1024);

        char echo[sizeof(msg)]{};
        ns.read(echo, sizeof(echo), 1024);
        std::printf("Byte I/O echo   : \"%s\"\n", echo);

    } catch (const nvme::NVMeError& e) {
        std::fprintf(stderr, "NVMe error: %s\n", e.what());
        return 1;
    }

    return 0;
}
