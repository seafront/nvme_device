#pragma once

#include "nvme_namespace.h"

#include <string>
#include <vector>

namespace nvme {

class NVMeDevice {
public:
    // capacity_bytes: size of each namespace's backing store (default 1 GiB)
    explicit NVMeDevice(const std::string& name,
                        uint64_t capacity_bytes = 1ULL * 1024 * 1024 * 1024);
    ~NVMeDevice() = default;

    NVMeDevice(const NVMeDevice&) = delete;
    NVMeDevice& operator=(const NVMeDevice&) = delete;

    IdentifyController    identify_controller();
    std::vector<uint32_t> list_namespaces();

    NVMeNamespace open_namespace(uint32_t nsid = 1);

    const std::string& name() const { return name_; }

private:
    std::string name_;
    uint64_t    capacity_bytes_;
};

} // namespace nvme
