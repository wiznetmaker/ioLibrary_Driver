#ifndef _PING_H_
#define _PING_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/*
 * @brief
 * @details If you want to display debug & processing message, uncomment #define _PING_DEBUG_
 * @note    If defined, it depends on <stdio.h>
 */
#define _PING_DEBUG_

#define PING_DATA_SIZE       33

#define PING_TYPE_REQUEST    8
#define PING_TYPE_REPLY      0
#define PING_CODE_ZERO       0
#if (_WIZCHIP_ == W5100)
#define PING_IDENTIFIER      0x43
#elif (_WIZCHIP_ == W5100S)
#define PING_IDENTIFIER      getVER()
#elif ((_WIZCHIP_ == W5200) || (_WIZCHIP_ == W5500))
#define PING_IDENTIFIER      getVERSIONR()
#elif (_WIZCHIP_ == W5300)
#define PING_IDENTIFIER      getIDR()
#else
/* @todo: Handle exceptions. */
#endif
#define PING_SEQUENCE_NUMBER 0x0001

typedef enum pingerr
{
  PING_OK          = 0,
  PING_ERR         = -1,
  PING_SOCKET_ERR  = -2,
  PING_SEND_ERR    = -3,
  PING_RECEIVE_ERR = -4,
  PING_PACKET_ERR  = -5,
} PINGERR;

struct icmp_echo_header
{
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t identifier;
  uint16_t sequence_number;
};

typedef struct pingmsg
{
  struct icmp_echo_header pingheader;
  uint8_t data[PING_DATA_SIZE];
} PINGMSG;

int8_t ping_auto(uint8_t s, uint8_t *addr);
int8_t ping_request(uint8_t s, uint8_t *addr);
int8_t ping_reply(uint8_t s, uint8_t *addr, uint16_t len);
uint16_t checksum(uint8_t *buf, uint16_t len);
uint16_t htons(uint16_t hostshort);

#ifdef __cplusplus
}
#endif

#endif /* _PING_H_ */
