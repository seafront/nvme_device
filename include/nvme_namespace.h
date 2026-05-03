#pragma once

#include "nvme_types.h"

#include <cstdint>
#include <memory>

namespace nvme {

class NVMeNamespace {
public:
    NVMeNamespace(std::unique_ptr<uint8_t[]> storage, uint64_t storage_bytes,
                  uint32_t nsid, const IdentifyNamespace& id_ns);
    ~NVMeNamespace() = default;

    NVMeNamespace(const NVMeNamespace&) = delete;
    NVMeNamespace& operator=(const NVMeNamespace&) = delete;
    NVMeNamespace(NVMeNamespace&&) noexcept = default;
    NVMeNamespace& operator=(NVMeNamespace&&) noexcept = default;

    // LBA-addressed I/O (nlb is 0-based: 0 = 1 block)
    void read_lba(void* buf, uint64_t slba, uint16_t nlb);
    void write_lba(const void* buf, uint64_t slba, uint16_t nlb);

    // Byte-addressed I/O
    void read(void* buf, size_t nbytes, uint64_t byte_offset);
    void write(const void* buf, size_t nbytes, uint64_t byte_offset);

    uint32_t nsid()           const { return nsid_; }
    uint64_t block_count()    const { return id_ns_.nsze; }
    uint32_t block_size()     const { return block_size_; }
    uint64_t capacity_bytes() const { return storage_bytes_; }

    const IdentifyNamespace& identify() const { return id_ns_; }

private:
    std::unique_ptr<uint8_t[]> storage_;
    uint64_t          storage_bytes_;
    uint32_t          nsid_;
    IdentifyNamespace id_ns_;
    uint32_t          block_size_;

    void check_bounds(uint64_t byte_offset, size_t nbytes) const;
};

} // namespace nvme
