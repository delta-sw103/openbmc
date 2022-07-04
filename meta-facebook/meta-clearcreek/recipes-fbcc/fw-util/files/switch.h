#ifndef _SWITCH_H_
#define _SWITCH_H_

#include "fw-util.h"
#include "spiflash.h"
#include <syslog.h>
#include <openbmc/pal.h>

class PAXComponent : public GPIOSwitchedSPIMTDComponent {
  uint8_t _paxid;
  public:
    PAXComponent(std::string fru, std::string comp, uint8_t paxid, std::string shadow)
      : GPIOSwitchedSPIMTDComponent(fru, comp, "switch0", "spi1.0", shadow, true), _paxid(paxid) {
          spipath = "/sys/bus/spi/drivers/spi-nor";
        }
    int print_version() override;
    int update(std::string image) override;
};

#endif
