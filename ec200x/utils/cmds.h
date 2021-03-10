#ifndef __CMDS_H
#define __CMDS_H

#include <stdint.h>

#include "at_ec200x.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum cmds_err_t
{
    CMD_ERR_NONE        = 0,
    CMD_ERR_INVAILD_ARG,
    CMD_ERR_LEN_ERR,
    CMD_ERR_UNSUPPORT,
    CMD_ERR_UNKNOWN,
    CMD_ERR_WAIT_TIMEOUT,

    CMD_ERR_CME_0       = 0x4000, /* Phone failure */
    CMD_ERR_CME_1, /* No connection to phone */
    CMD_ERR_CME_2, /* Phone-adaptor link reserved */
    CMD_ERR_CME_3, /* Operation not allowed */
    CMD_ERR_CME_4, /* Operation not supported */
    CMD_ERR_CME_5, /* PH_SIM PIN required */
    CMD_ERR_CME_6, /* PH_FSIM PIN required */
    CMD_ERR_CME_7, /* PH_FSIM PUK required */
    CMD_ERR_CME_8, /* Reserved */
    CMD_ERR_CME_9, /* Reserved */

    CMD_ERR_CME_10, /* SIM not inserted */
    CMD_ERR_CME_11, /* SIM PIN required */
    CMD_ERR_CME_12, /* SIM PUK required */
    CMD_ERR_CME_13, /* SIM failure */
    CMD_ERR_CME_14, /* SIM busy */
    CMD_ERR_CME_15, /* SIM wrong */
    CMD_ERR_CME_16, /* Incorrect password */
    CMD_ERR_CME_17, /* SIM PIN2 required */
    CMD_ERR_CME_18, /* SIM PUK2 required */
    CMD_ERR_CME_19, /* Reserved */

    CMD_ERR_CME_20, /* Memrory full */
    CMD_ERR_CME_21, /* Invaild index */
    CMD_ERR_CME_22, /* Not found */
    CMD_ERR_CME_23, /* Memrory failure */
    CMD_ERR_CME_24, /* Text string too long */
    CMD_ERR_CME_25, /* Invaild characters in text string */
    CMD_ERR_CME_26, /* Dial string too long */
    CMD_ERR_CME_27, /* Invaild characters in dial string */
    CMD_ERR_CME_28, /* Reserved */
    CMD_ERR_CME_29, /* Reserved */

    CMD_ERR_CME_30, /* No network service */
    CMD_ERR_CME_31, /* Network timeout */
    CMD_ERR_CME_32, /* Network not allowed-emergency calls only */
    CMD_ERR_CME_33, /* Reserved */
    CMD_ERR_CME_34, /* Reserved */
    CMD_ERR_CME_35, /* Reserved */
    CMD_ERR_CME_36, /* Reserved */
    CMD_ERR_CME_37, /* Reserved */
    CMD_ERR_CME_38, /* Reserved */
    CMD_ERR_CME_39, /* Reserved */

    CMD_ERR_CME_40, /* Network personalization PIN required */
    CMD_ERR_CME_41, /* Network personalization PUK required */
    CMD_ERR_CME_42, /* Network subset personalization PIN required */
    CMD_ERR_CME_43, /* Network subset personalization PUK required */
    CMD_ERR_CME_44, /* Service provider personalization PIN required */
    CMD_ERR_CME_45, /* Service provider personalization PUK required */
    CMD_ERR_CME_46, /* Corporate personalization PIN required */
    CMD_ERR_CME_47, /* Corporate personalization PUK required */

    CMD_ERR_CMS_0       = 0x5000,

    CMD_ERR_CMS_300     = CMD_ERR_CMS_0 + 300, /* ME failure */
    CMD_ERR_CMS_301, /* SMS ME reserved */
    CMD_ERR_CMS_302, /* Operation not allowed */
    CMD_ERR_CMS_303, /* Operation not supported */
    CMD_ERR_CMS_304, /* Invaild PDU mode */
    CMD_ERR_CMS_305, /* Invaild text mode */
    CMD_ERR_CMS_306, /* Reserved */
    CMD_ERR_CMS_307, /* Reserved */
    CMD_ERR_CMS_308, /* Reserved */
    CMD_ERR_CMS_309, /* Reserved */

    CMD_ERR_CMS_310, /* SIM not inserted */
    CMD_ERR_CMS_311, /* SIM pin necessary */
    CMD_ERR_CMS_312, /* PH SIM pin necessary */
    CMD_ERR_CMS_313, /* SIM failure */
    CMD_ERR_CMS_314, /* SIM busy */
    CMD_ERR_CMS_315, /* SIM wrong */
    CMD_ERR_CMS_316, /* SIM PUK required */
    CMD_ERR_CMS_317, /* SIM PIN2 required */
    CMD_ERR_CMS_318, /* SIM PUK2 required */
    CMD_ERR_CMS_319, /* Reserved */

    CMD_ERR_CMS_320, /* Memrory failure */
    CMD_ERR_CMS_321, /* Invaild memrory index */
    CMD_ERR_CMS_322, /* Memeoeyr full */
    CMD_ERR_CMS_323, /* Reserved */
    CMD_ERR_CMS_324, /* Reserved */
    CMD_ERR_CMS_325, /* Reserved */
    CMD_ERR_CMS_326, /* Reserved */
    CMD_ERR_CMS_327, /* Reserved */
    CMD_ERR_CMS_328, /* Reserved */
    CMD_ERR_CMS_329, /* Reserved */

    CMD_ERR_CMS_330, /* SMSC address unknown */
    CMD_ERR_CMS_331, /* No network */
    CMD_ERR_CMS_332, /* Network timeout */

    CMD_ERR_CMS_500 = CMD_ERR_CMS_300 + 200, /* Unknown */
    CMD_ERR_CMS_501, /* Reserved */
    CMD_ERR_CMS_502, /* Reserved */
    CMD_ERR_CMS_503, /* Reserved */
    CMD_ERR_CMS_504, /* Reserved */
    CMD_ERR_CMS_505, /* Reserved */
    CMD_ERR_CMS_506, /* Reserved */
    CMD_ERR_CMS_507, /* Reserved */
    CMD_ERR_CMS_508, /* Reserved */
    CMD_ERR_CMS_509, /* Reserved */

    CMD_ERR_CMS_510, /* Reserved */
    CMD_ERR_CMS_511, /* Reserved */
    CMD_ERR_CMS_512, /* SIM not ready */
    CMD_ERR_CMS_513, /* Message length exceeds */
    CMD_ERR_CMS_514, /* Invaild request parameters */
    CMD_ERR_CMS_515, /* ME storage failure */
    CMD_ERR_CMS_516, /* Reserved */
    CMD_ERR_CMS_517, /* Invaild service mode */
    CMD_ERR_CMS_518, /* Reserved */
    CMD_ERR_CMS_519, /* Reserved */

    CMD_ERR_CMS_520, /* Reserved */
    CMD_ERR_CMS_521, /* Reserved */
    CMD_ERR_CMS_522, /* Reserved */
    CMD_ERR_CMS_523, /* Reserved */
    CMD_ERR_CMS_524, /* Reserved */
    CMD_ERR_CMS_525, /* Reserved */
    CMD_ERR_CMS_526, /* Reserved */
    CMD_ERR_CMS_527, /* Reserved */
    CMD_ERR_CMS_528, /* More message to send state error */
    CMD_ERR_CMS_529, /* MO SMS is not allow */

    CMD_ERR_CMS_530, /* GPRS is suppended */
    CMD_ERR_CMS_531, /* ME storage full */

    CMD_ERR_ERROR       = 0x8000,
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static inline char *skip_chr(char ch, char *p)
{
    char *op = p;
    while((NULL != p) && (ch != *p)) { p++; }
    return (++p) ? p : op;
}

static inline uint8_t chr_to_u8(char c)
{
    if(c >= '0' && c <= '9') {
        return (uint8_t)(c - '0');
    } else if(c >= 'A' && c <= 'F') {
        return (uint8_t)(0x0a + c - 'A');
    } else if(c >= 'a' && c <= 'f'){
        return (uint8_t)(0x0a + c - 'a');
    }

    return 0;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct ec200x_basic_info_t
{
    int csq;
    char iccid[24];
    char imei[24];
};

extern struct ec200x_basic_info_t ec200x_basic_info;

extern struct at_urc_t basic_cmds[];
extern const uint32_t basic_cmd_tabsz;

char * ec200x_get_imei(void);

int ex200x_get_csq(void);
char * ec200x_get_ccid(void);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ec200x_gps_info_t
{
    uint32_t flag;
    float latitude;
    float longitude;
    float hdop;
    float altitude;
    uint32_t fix;
    float cog;
    float spkm;
    float spkn;
    uint32_t nsta;

    union {
        struct {
            uint16_t year;
            uint8_t mon;
            uint8_t day;

            uint8_t hour;
            uint8_t min;
            uint8_t sec;

            uint8_t resv;
        };
        uint64_t u64;
    }time;
};

extern struct ec200x_gps_info_t ec200x_gps_info;

extern struct at_urc_t gps_cmds[];
extern const uint32_t gps_cmd_tabsz;

int ec200x_gps_en(void);
struct ec200x_gps_info_t * ec200x_gps_get_loc(void);

void ec200x_gps_print(struct ec200x_gps_info_t *info);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ec200x_context_ip_t
{
    uint8_t contextID;
    uint8_t contextType;
    uint8_t contextState;

    char ip[24];
};

struct ec200x_net_info_t
{
    struct ec200x_context_ip_t context_ip;
};

extern struct ec200x_net_info_t ec200x_net_info;

extern struct at_urc_t net_cmds[];
extern const uint32_t net_cmd_tabsz;

int ec200x_net_enable(void);

struct ec200x_context_ip_t * ec200x_net_get_ip(void);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif /* __CMDS_H */
