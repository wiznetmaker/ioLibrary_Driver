#ifdef _PING_DEBUG_
#include <stdio.h>
#endif /* _PING_DEBUG_ */
#include "socket.h"
#include "ping.h"

uint16_t sequence_number = PING_SEQUENCE_NUMBER;
PINGMSG pingmsg_request;
PINGMSG pingmsg_reply;

int8_t ping_auto(uint8_t s, uint8_t *addr)
{
  int retval = 0;
  uint16_t size = 0;

  switch (getSn_SR(s))
  {
    case SOCK_IPRAW:
      if ((retval = ping_request(s, addr)) <= 0)
      {
#ifdef _PING_DEBUG_
        /* @todo: Handle errors. */
#endif /* _PING_DEBUG_ */
        return PING_SOCKET_ERR;
      }

      if ((size = getSn_RX_RSR(s)) > 0)
      {
        if ((retval = ping_reply(s, addr, size)) <= 0)
        {
#ifdef _PING_DEBUG_
          /* @todo: Handle errors. */
#endif /* _PING_DEBUG_ */
          return PING_SOCKET_ERR;
        }
      }
      break;

    case SOCK_CLOSED:
      close(s);
#if ((_WIZCHIP_ == W5100) || (_WIZCHIP_ == W5100S) || (_WIZCHIP_ == W5200) || (_WIZCHIP_ == W5300) || (_WIZCHIP_ == W5500))
      setSn_PROTO(s, IPPROTO_ICMP);
#else
      /* @todo: Handle exceptions. */
#endif
      if ((retval = socket(s, Sn_MR_IPRAW, 0x00, 0x00)) != s)
      {
#ifdef _PING_DEBUG_
        /* @todo: Handle errors. */
#endif /* _PING_DEBUG_ */
        return PING_SOCKET_ERR;
      }
      break;

    default:
      break;
  }

  return PING_OK;
}

int8_t ping_request(uint8_t s, uint8_t *addr)
{
  uint16_t i = 0;

  pingmsg_request.pingheader.type = PING_TYPE_REQUEST;
  pingmsg_request.pingheader.code = PING_CODE_ZERO;
  pingmsg_request.pingheader.checksum = 0;
  pingmsg_request.pingheader.identifier = htons(PING_IDENTIFIER);
  pingmsg_request.pingheader.sequence_number = htons(sequence_number++);
  for (i = 0; i < sizeof(pingmsg_request.data); i++)
  {
    pingmsg_request.data[i] = (uint8_t)i; // data has no special meaning, so fill it with any data.
  }
  pingmsg_request.pingheader.checksum = htons(checksum((uint8_t *)&pingmsg_request, sizeof(pingmsg_request)));

  if (sendto(s, (uint8_t *)&pingmsg_request, sizeof(pingmsg_request), addr, 0x00) <= 0)
  {
#ifdef _PING_DEBUG_
    /* @todo: Handle errors. */
#endif /* _PING_DEBUG_ */
    return PING_SEND_ERR;
  }

  return PING_OK;
}

int8_t ping_reply(uint8_t s, uint8_t *addr, uint16_t len)
{
  uint8_t receive_buf[sizeof(pingmsg_reply)] = {0, };
  uint16_t size = 0;
  uint16_t i = 0;

  if ((size = recvfrom(s, receive_buf, len, addr, (uint16_t *)0x00)) <= 0)
  {
#ifdef _PING_DEBUG_
    /* @todo: Handle errors. */
#endif /* _PING_DEBUG_ */
    return PING_RECEIVE_ERR;
  }

  if (receive_buf[0] == PING_TYPE_REPLY)
  {
    pingmsg_reply.pingheader.type = receive_buf[0];
    pingmsg_reply.pingheader.code = receive_buf[1];
    pingmsg_reply.pingheader.checksum = (receive_buf[3] << 8) + receive_buf[2];
    pingmsg_reply.pingheader.identifier = (receive_buf[5] << 8) + receive_buf[4];
    pingmsg_reply.pingheader.sequence_number = (receive_buf[7] << 8) + receive_buf[6];
    for (i = 0; i < size - sizeof(pingmsg_reply.pingheader); i++)
    {
      pingmsg_reply.data[i] = receive_buf[i + sizeof(pingmsg_reply.pingheader)];
    }
  }
  else
  {
#ifdef _PING_DEBUG_
    /* @todo: Handle errors. */
#endif /* _PING_DEBUG_ */
    return PING_PACKET_ERR;
  }

  return PING_OK;
}

uint16_t checksum(uint8_t *buf, uint16_t len)
{
  uint16_t checksum = 0;
  uint32_t sum = 0;

  while (len > 1)
  {
    sum += (*buf << 8) + *(buf + 1);
    buf += 2;
    len -= 2;
  }

  if (len > 0)
    sum += (*buf << 8);

  while (sum >> 16)
    sum = (sum & 0xffff) + (sum >> 16);

  checksum = ~sum;

  return checksum;
}

uint16_t htons(uint16_t hostshort)
{
  uint16_t netshort = 0;

  netshort = (hostshort & 0xFF) << 8;
  netshort |= ((hostshort >> 8) & 0xFF);

  return netshort;
}
