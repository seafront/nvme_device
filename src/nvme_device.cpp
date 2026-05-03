#include "nvme_device.h"

#include <cstring>
#include <sstream>
#include <stdexcept>

namespace nvme {

namespace {

constexpr uint32_t kFakeBlockSize = 512;

uint32_t lba_size_from_id(const IdentifyNamespace& id_ns) {
    uint8_t flbas = id_ns.flbas & 0x0F;
    uint8_t ds    = id_ns.lbaf[flbas].ds;
    return (ds == 0) ? 512u : (1u << ds);
}

IdentifyController make_fake_controller(const std::string& name, uint32_t nn) {
    IdentifyController c{};
    c.vid   = 0x8086;
    c.ssvid = 0x8086;
    c.ver   = 0x00010300; // NVMe 1.3
    c.nn    = nn;
    std::strncpy(c.sn, "FAKESN0000000001    ", sizeof(c.sn));
    std::strncpy(c.mn, name.c_str(), sizeof(c.mn));
    std::strncpy(c.fr, "1.0.0   ", sizeof(c.fr));
    return c;
}

IdentifyNamespace make_fake_namespace(uint64_t capacity_bytes) {
    uint64_t block_count = capacity_bytes / kFakeBlockSize;
    IdentifyNamespace ns{};
    ns.nsze  = block_count;
    ns.ncap  = block_count;
    ns.nuse  = 0;
    ns.nlbaf = 0;
    ns.flbas = 0;
    ns.lbaf[0].ms = 0;
    ns.lbaf[0].ds = 9; // 2^9 = 512 bytes
    ns.lbaf[0].rp = 0;
    return ns;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// NVMeDevice
// ---------------------------------------------------------------------------

NVMeDevice::NVMeDevice(const std::string& name, uint64_t capacity_bytes)
    : name_(name), capacity_bytes_(capacity_bytes) {}

IdentifyController NVMeDevice::identify_controller() {
    return make_fake_controller(name_, 1);
}

std::vector<uint32_t> NVMeDevice::list_namespaces() {
    return {1};
}

NVMeNamespace NVMeDevice::open_namespace(uint32_t nsid) {
    IdentifyNamespace id_ns = make_fake_namespace(capacity_bytes_);
    auto storage = std::make_unique<uint8_t[]>(capacity_bytes_);
    std::memset(storage.get(), 0, capacity_bytes_);
    return NVMeNamespace(std::move(storage), capacity_bytes_, nsid, id_ns);
}

// ---------------------------------------------------------------------------
// NVMeNamespace
// ---------------------------------------------------------------------------

NVMeNamespace::NVMeNamespace(std::unique_ptr<uint8_t[]> storage,
                              uint64_t storage_bytes,
                              uint32_t nsid,
                              const IdentifyNamespace& id_ns)
    : storage_(std::move(storage)), storage_bytes_(storage_bytes),
      nsid_(nsid), id_ns_(id_ns), block_size_(lba_size_from_id(id_ns)) {}

void NVMeNamespace::check_bounds(uint64_t byte_offset, size_t nbytes) const {
    if (byte_offset + nbytes > storage_bytes_) {
        std::ostringstream ss;
        ss << "out of bounds: offset=" << byte_offset
           << " nbytes=" << nbytes
           << " capacity=" << storage_bytes_;
        throw NVMeError(ss.str());
    }
}

void NVMeNamespace::read_lba(void* buf, uint64_t slba, uint16_t nlb) {
    uint64_t byte_offset = slba * block_size_;
    size_t   nbytes      = static_cast<size_t>(nlb + 1) * block_size_;
    check_bounds(byte_offset, nbytes);
    std::memcpy(buf, storage_.get() + byte_offset, nbytes);
}

void NVMeNamespace::write_lba(const void* buf, uint64_t slba, uint16_t nlb) {
    uint64_t byte_offset = slba * block_size_;
    size_t   nbytes      = static_cast<size_t>(nlb + 1) * block_size_;
    check_bounds(byte_offset, nbytes);
    std::memcpy(storage_.get() + byte_offset, buf, nbytes);
}

void NVMeNamespace::read(void* buf, size_t nbytes, uint64_t byte_offset) {
    check_bounds(byte_offset, nbytes);
    std::memcpy(buf, storage_.get() + byte_offset, nbytes);
}

void NVMeNamespace::write(const void* buf, size_t nbytes, uint64_t byte_offset) {
    check_bounds(byte_offset, nbytes);
    std::memcpy(storage_.get() + byte_offset, buf, nbytes);
}

} // namespace nvme
