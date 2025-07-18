//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//
#if BOARD == BOARD_ARDUINO_MKR_WIFI_1010
#ifndef _ZENOH_GENERIC_PLATFORM_H
#define _ZENOH_GENERIC_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/time.h>
#include <variant.h>

#include "zenoh_generic_config.h"


#ifdef __cplusplus
extern "C" {
#endif



#if Z_FEATURE_MULTI_THREAD == 1
typedef void *_z_task_t;
typedef void *z_task_attr_t;
typedef void *_z_mutex_t;
typedef void *_z_condvar_t;
#endif  // Z_FEATURE_MULTI_THREAD == 1

typedef struct timespec z_clock_t;
typedef struct timeval z_time_t;

typedef struct IPAddress IPAddress;    // Forward declaration to be used in __z_net_iptcp_addr_t
typedef struct WiFiClient WiFiClient;  // Forward declaration to be used in _z_sys_net_socket_t
typedef struct WiFiUDP WiFiUDP;        // Forward declaration to be used in _z_sys_net_socket_t
typedef struct Uart Uart;    // Forward declaration to be used in _z_sys_net_socket_t

typedef struct {
    union {
#if Z_FEATURE_LINK_TCP == 1
        WiFiClient *_tcp;  // As pointer to cross the boundary between C and C++
#endif
#if Z_FEATURE_LINK_UDP_MULTICAST == 1 || Z_FEATURE_LINK_UDP_UNICAST == 1
        WiFiUDP *_udp;  // As pointer to cross the boundary between C and C++
#endif
#if Z_FEATURE_LINK_SERIAL == 1
        Uart *_serial;
#endif
        bool _err;
    };
} _z_sys_net_socket_t;

typedef struct {
    IPAddress *_addr;  // As pointer to cross the boundary between C and C++
    uint16_t _port;
} __z_net_iptcp_addr_t;

typedef struct {
    union {
#if Z_FEATURE_LINK_TCP == 1 || Z_FEATURE_LINK_UDP_MULTICAST == 1 || Z_FEATURE_LINK_UDP_UNICAST == 1
        __z_net_iptcp_addr_t _iptcp;
#endif
    };
    bool _err;
} _z_sys_net_endpoint_t;

#ifdef __cplusplus
}
#endif

#endif /* ZENOH_PICO_SYSTEM_ARDUINO_OPENCR_TYPES_H */
#endif /* BOARD == BOARD_ARDUINO_MKR_WIFI_1010 */