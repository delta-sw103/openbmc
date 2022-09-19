#ifndef __MP2856_H__
#define __MP2856_H__

// Support MP2856,MP2857,MP2971,MP2973

#include "vr.h"

#define VR_MPS_PAGE_0    0x00
#define VR_MPS_PAGE_1    0x01
#define VR_MPS_PAGE_2    0x02
#define VR_MPS_PAGE_29   0x29
#define VR_MPS_PAGE_2A   0x2A


/*Page0 */
#define VR_MPS_REG_WRITE_PROTECT      0x10
#define VR_MPS_REG_PRODUCT_ID         0x9A
#define VR_MPS_REG_CONFIG_ID          0x9D
#define VR_MPS_REG_STATUS_CML         0x7E

#define VR_MPS_CMD_STORE_NORMAL_CODE  0xF1

/*Page1 */
#define VR_MPS_REG_MFR_VR_CONFIG2     0x35
#define VR_MPS_REG_MFR_MTP_PMBUS_CTRL 0x4F
#define VR_MPS_REG_CRC_NORMAL_CODE    0xAD
#define VR_MPS_REG_CRC_MULTI_CONFIG   0xAF

/*Page2 */
#define VR_MPS_CMD_STORE_MULTI_CODE   0xF3

/*Page29 */
#define VR_MPS_REG_CRC_USER           0xFF

/*Page2A */
#define VR_MPS_REG_MULTI_CONFIG       0xBF

/* STATUS_CML bit[3] */
#define MASK_PWD_MATCH                0x08
/* MFR_VR_CONFIG2 bit[2] */
#define  MASK_WRITE_PROTECT_MODE      0x04
/* MFR_MTP_PMBUS_CTRL bit[5] */
#define  MASK_MTP_BYTE_RW_EN          0x20

#define MP2856_DISABLE_WRITE_PROTECT 0x63
#define MP2856_PRODUCT_ID 0x2856
#define MP2857_PRODUCT_ID 0x2857

enum {
  BYTE_WRITE_READ = 0,
  WORD_WRITE_READ ,
  BLOCK_WRITE_2_BYTE,
};

enum {
  ATE_CONF_ID = 0,
  ATE_PAGE_NUM,
  ATE_REG_ADDR_HEX,
  ATE_REG_ADDR_DEC,
  ATE_REG_NAME,
  ATE_REG_DATA_HEX,
  ATE_REG_DATA_DEC,
  ATE_WRITE_TYPE,
  ATE_COL_MAX,
};

struct mp2856_data {
  uint16_t cfg_id;
  uint8_t page;
  uint8_t reg_addr;
  uint8_t reg_data[4];
  uint8_t reg_len;
};

struct mp2856_config {
  uint8_t mode;
  uint8_t addr;
  uint16_t cfg_id;
  uint16_t wr_cnt;
  uint16_t product_id_exp;
  struct mp2856_data pdata[720];
};

enum {
  MP285X,
  MP297X,
};

void* mp2856_parse_file(struct vr_info*, const char*);
int mp2856_fw_update(struct vr_info*, void*);
int get_mp2856_ver(struct vr_info*, char*);

#endif
