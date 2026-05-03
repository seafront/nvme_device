#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "nvme_device.h"

#include <array>
#include <cstring>

using namespace nvme;

class NVMeDeviceTest : public ::testing::Test {
protected:
    NVMeDevice dev{"TestDevice", 1 * 1024 * 1024}; // 1 MiB
};

TEST_F(NVMeDeviceTest, IdentifyControllerReturnsValidData) {
    auto ctrl = dev.identify_controller();
    EXPECT_EQ(ctrl.nn, 1u);
    EXPECT_NE(ctrl.vid, 0u);
}

TEST_F(NVMeDeviceTest, ListNamespacesReturnsOne) {
    auto nsids = dev.list_namespaces();
    ASSERT_EQ(nsids.size(), 1u);
    EXPECT_EQ(nsids[0], 1u);
}

class NVMeNamespaceTest : public ::testing::Test {
protected:
    NVMeDevice dev{"TestDevice", 1 * 1024 * 1024}; // 1 MiB
    NVMeNamespace ns = dev.open_namespace(1);
};

TEST_F(NVMeNamespaceTest, BlockSizeIs512) {
    EXPECT_EQ(ns.block_size(), 512u);
}

TEST_F(NVMeNamespaceTest, CapacityMatchesRequested) {
    EXPECT_EQ(ns.capacity_bytes(), 1u * 1024 * 1024);
}

TEST_F(NVMeNamespaceTest, InitialStorageIsZeroed) {
    std::array<uint8_t, 512> buf{};
    buf.fill(0xFF);
    ns.read_lba(buf.data(), 0, 0);
    for (auto b : buf) EXPECT_EQ(b, 0u);
}

TEST_F(NVMeNamespaceTest, WriteThenReadLba) {
    std::array<uint8_t, 512> wbuf{};
    for (int i = 0; i < 512; ++i) wbuf[i] = static_cast<uint8_t>(i);

    ns.write_lba(wbuf.data(), 0, 0);

    std::array<uint8_t, 512> rbuf{};
    ns.read_lba(rbuf.data(), 0, 0);

    EXPECT_EQ(wbuf, rbuf);
}

TEST_F(NVMeNamespaceTest, WriteThenReadMultipleBlocks) {
    std::array<uint8_t, 1024> wbuf{};
    wbuf.fill(0xAB);

    ns.write_lba(wbuf.data(), 2, 1); // slba=2, nlb=1 → 2 blocks

    std::array<uint8_t, 1024> rbuf{};
    ns.read_lba(rbuf.data(), 2, 1);

    EXPECT_EQ(wbuf, rbuf);
}

TEST_F(NVMeNamespaceTest, ByteAddressedWriteThenRead) {
    const char msg[] = "hello nvme";
    ns.write(msg, sizeof(msg), 512);

    char echo[sizeof(msg)]{};
    ns.read(echo, sizeof(echo), 512);

    EXPECT_STREQ(echo, msg);
}

TEST_F(NVMeNamespaceTest, OutOfBoundsReadThrows) {
    std::array<uint8_t, 512> buf{};
    uint64_t last_lba = ns.block_count(); // one past the end
    EXPECT_THROW(ns.read_lba(buf.data(), last_lba, 0), NVMeError);
}

TEST_F(NVMeNamespaceTest, OutOfBoundsWriteThrows) {
    std::array<uint8_t, 512> buf{};
    uint64_t last_lba = ns.block_count();
    EXPECT_THROW(ns.write_lba(buf.data(), last_lba, 0), NVMeError);
}

TEST_F(NVMeNamespaceTest, BlocksDontOverlapEachOther) {
    std::array<uint8_t, 512> a{}, b{};
    a.fill(0xAA);
    b.fill(0xBB);

    ns.write_lba(a.data(), 0, 0);
    ns.write_lba(b.data(), 1, 0);

    std::array<uint8_t, 512> ra{}, rb{};
    ns.read_lba(ra.data(), 0, 0);
    ns.read_lba(rb.data(), 1, 0);

    EXPECT_EQ(ra, a);
    EXPECT_EQ(rb, b);
}
