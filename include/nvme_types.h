#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace nvme {

struct IdentifyController {
    uint16_t vid;
    uint16_t ssvid;
    char     sn[20];
    char     mn[40];
    char     fr[8];
    uint8_t  rab;
    uint8_t  ieee[3];
    uint8_t  cmic;
    uint8_t  mdts;
    uint16_t cntlid;
    uint32_t ver;
    uint8_t  reserved[172];
    uint16_t oacs;
    uint8_t  acl;
    uint8_t  aerl;
    uint8_t  frmw;
    uint8_t  lpa;
    uint8_t  elpe;
    uint8_t  npss;
    uint8_t  avscc;
    uint8_t  apsta;
    uint16_t wctemp;
    uint16_t cctemp;
    uint8_t  reserved2[242];
    uint8_t  sqes;
    uint8_t  cqes;
    uint16_t maxcmd;
    uint32_t nn;
    uint16_t oncs;
    uint16_t fuses;
    uint8_t  fna;
    uint8_t  vwc;
    uint16_t awun;
    uint16_t awupf;
    uint8_t  nvscc;
    uint8_t  reserved3[1];
    uint16_t acwu;
    uint8_t  reserved4[2];
    uint32_t sgls;
    uint8_t  reserved5[228];
    uint8_t  subnqn[256];
    uint8_t  reserved6[768];
    uint8_t  psd[1024];
    uint8_t  vs[1024];
};

struct IdentifyNamespace {
    uint64_t nsze;
    uint64_t ncap;
    uint64_t nuse;
    uint8_t  nsfeat;
    uint8_t  nlbaf;
    uint8_t  flbas;
    uint8_t  mc;
    uint8_t  dpc;
    uint8_t  dps;
    uint8_t  nmic;
    uint8_t  rescap;
    uint8_t  fpi;
    uint8_t  dlfeat;
    uint16_t nawun;
    uint16_t nawupf;
    uint16_t nacwu;
    uint16_t nabsn;
    uint16_t nabo;
    uint16_t nabspf;
    uint16_t noiob;
    uint8_t  nvmcap[16];
    uint8_t  reserved[40];
    uint8_t  nguid[16];
    uint8_t  eui64[8];
    struct {
        uint16_t ms;
        uint8_t  ds;  // LBA data size: 2^ds bytes
        uint8_t  rp;
    } lbaf[16];
    uint8_t  reserved2[192];
    uint8_t  vs[3712];
};

class NVMeError : public std::runtime_error {
public:
    explicit NVMeError(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace nvme
