#include <cstdio>
#include <cstring>
#include <syslog.h>
#include <openbmc/obmc-i2c.h>
#include <openbmc/pal.h>
#include <openbmc/cpld.h>
#include <openbmc/kv.h>
#include <openbmc/ipmi.h>
#include "fw-util.h"
#include <libpldm/pldm.h>
#include <libpldm/platform.h>
#include <libpldm-oem/pldm.h>
#include "signed_decoder.hpp"
#include "signed_info.hpp"

#define SWB_CPLD_BUS_ID   (7)
#define ACB_CPLD_BUS_ID   (4)

using namespace std;

int
cpld_pldm_wr(uint8_t bus, uint8_t addr,
           uint8_t *txbuf, uint8_t txlen,
           uint8_t *rxbuf, uint8_t rxlen) {
  uint8_t tbuf[255] = {0};
  uint8_t tlen=0;
  int rc;

  if (pal_is_artemis()) {
    tbuf[0] = (ACB_CPLD_BUS_ID << 1) + 1;
  } else {
    tbuf[0] = (SWB_CPLD_BUS_ID << 1) + 1;
  }
  tbuf[1] = addr;
  tbuf[2] = rxlen;
  memcpy(tbuf+3, txbuf, txlen);
  tlen = txlen + 3;

  size_t rlen = 0;
  rc = pldm_norm_ipmi_send_recv(bus, SWB_BIC_EID,
                               NETFN_APP_REQ, CMD_APP_MASTER_WRITE_READ,
                               tbuf, tlen,
                               rxbuf, &rlen);
  return rc;
}

class CpldComponent : public Component {
  uint8_t pld_type;
  i2c_attr_t attr;
  int _update(const char *path, uint8_t is_signed, i2c_attr_t attr);

  public:
    CpldComponent(const string &fru, const string &comp, uint8_t type, uint8_t bus, uint8_t addr,
      int (*cpld_xfer)(uint8_t, uint8_t, uint8_t *, uint8_t, uint8_t *, uint8_t))
      : Component(fru, comp), pld_type(type), attr{bus, addr, cpld_xfer} {}
    int update(string image) override;
    int fupdate(string image) override;
    int get_version(json& j) override;
};

int CpldComponent::_update(const char *path, uint8_t is_signed, i2c_attr_t attr ) {
  int ret = -1;
  string comp = this->component();

  syslog(LOG_CRIT, "Component %s%s upgrade initiated", comp.c_str(), is_signed? "": " force");
  if (cpld_intf_open(pld_type, INTF_I2C, &attr)) {
    cerr << "Cannot open i2c!" << endl;
    return ret;
  }

  ret = cpld_program((char *)path, NULL, false);
  cpld_intf_close();
  if (ret) {
    cerr << "Error Occur at updating CPLD FW!" << endl;
    return ret;
  }

  syslog(LOG_CRIT, "Component %s%s upgrade completed", comp.c_str(), is_signed? "": " force");
  return ret;
}

int CpldComponent::update(string image) {
  return _update(image.c_str(), 1, attr);
}

int CpldComponent::fupdate(string image) {
  return _update(image.c_str(), 0, attr);
}

int CpldComponent::get_version(json& j) {
  int ret = -1;
  uint8_t ver[4];
  char strbuf[MAX_VALUE_LEN];
  string comp = this->component();
  transform(comp.begin(), comp.end(),comp.begin(), ::toupper);
  j["PRETTY_COMPONENT"] = comp;

  if (!cpld_intf_open(pld_type, INTF_I2C, &attr)) {
    ret = cpld_get_ver((uint32_t *)ver);
    cpld_intf_close();
  }

  if (ret) {
    sprintf(strbuf, "NA");
  } else {
    sprintf(strbuf, "%02X%02X%02X%02X", ver[0], ver[1], ver[2], ver[3]);
    kv_set(_component.c_str(), strbuf, 0, 0);
  }
  j["VERSION"] = string(strbuf);
  return FW_STATUS_SUCCESS;
}

class Signed_CpldComponent : public CpldComponent, public SignedDecoder {
  public:
    Signed_CpldComponent(const string &fru, const string &comp,
      uint8_t type, uint8_t bus, uint8_t addr, int (*cpld_xfer)(uint8_t, uint8_t, uint8_t *, uint8_t, uint8_t *, uint8_t),
      signed_header_t sign_info): CpldComponent(fru, comp, type, bus, addr, cpld_xfer),
      SignedDecoder(sign_info, fru) {}
    int update(string image);
};

int Signed_CpldComponent::update(string image) {
  int ret = 0;
  ret = is_image_signed(image);
  if (ret) {
    printf("Firmware not valid. error(%d)\n", -ret);
    return -1;
  }

  ret = get_image(image);
  if (ret) {
    printf("Get copy file. error(%d)\n", -ret);
    return -1;
  }

  ret = CpldComponent::update(image);
  if (ret) {
    printf("%s\n", image.c_str());
    printf("Update component with temp file failed. error(%d)\n", ret);
    return -1;
  }

  ret = delete_image();
  if (ret) {
    printf("Remove temp file failed. error(%d)\n", -ret);
    return -1;
  }

  return ret;
}

// CpldComponent mb_cpld("mb", "mb_cpld", LCMXO3_9400C, 7, 0x40, nullptr);
// CpldComponent swb_cpld("swb", "swb_cpld", LCMXO3_9400C, 3, 0x40, &cpld_pldm_wr);
// CpldComponent scm_cpld("scm", "scm_cpld", LCMXO3_2100C, 15, 0x40, nullptr);
class fw_cpld_config {
  public:
    fw_cpld_config(){
      signed_header_t cpld_info = {
        signed_info::PLATFORM_NAME,
        signed_info::MB_BOARD,
        signed_info::DVT,
        signed_info::CPLD
      };
      static Signed_CpldComponent mb_cpld("mb", "mb_cpld", LCMXO3_9400C, 7, 0x40, nullptr, cpld_info);

      cpld_info.board_id = signed_info::SWB_BOARD;
      static Signed_CpldComponent swb_cpld("swb", "swb_cpld", LCMXO3_9400C, 3, 0x40, &cpld_pldm_wr, cpld_info);

      cpld_info.board_id = signed_info::SCM_BOARD;
      static Signed_CpldComponent scm_cpld("scm", "scm_cpld", LCMXO3_2100C, 15, 0x40, nullptr, cpld_info);
    }
};

fw_cpld_config _fw_cpld_config;