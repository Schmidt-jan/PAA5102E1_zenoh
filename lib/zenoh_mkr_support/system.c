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
#include <zenoh_generic_platform.h>
#include <Arduino.h>
#include <stddef.h>
#include <sys/time.h>

#include "zenoh-pico/config.h"
#include "zenoh-pico/system/platform.h"

/*------------------ Random ------------------*/
uint8_t z_random_u8(void) { return z_random_u32() & 0xFF; }

uint16_t z_random_u16(void) { return z_random_u32() & 0xFFFF; }

uint32_t z_random_u32(void) { return random(); }

uint64_t z_random_u64(void) {
    uint64_t ret = 0;
    ret |= z_random_u32();
    ret = ret << 32;
    ret |= z_random_u32();

    return ret;
}


void z_random_fill(void *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        ((uint8_t *)buf)[i] = z_random_u8();
    }
}

size_t total_heap_used = 0;
/*------------------ Memory ------------------*/
void *z_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr) {
        total_heap_used += size;
    }
    return ptr;
}

void *z_realloc(void *ptr, size_t size) {
    // Not implemented by the platform
    // raise error
    return NULL;
}

void z_free(void *ptr) {
    if (ptr) {
     //   total_heap_used -= malloc_usable_size(ptr);
        free(ptr);
    }}

#if Z_FEATURE_MULTI_THREAD == 1
#include <FreeRTOS.h>

#error "Multi-threading not supported yet on OpenCR port. Disable it by defining Z_FEATURE_MULTI_THREAD=0"

/*------------------ Task ------------------*/
z_result_t _z_task_init(_z_task_t *task, z_task_attr_t *attr, void *(*fun)(void *), void *arg) { return -1; }

z_result_t _z_task_join(_z_task_t *task) { return -1; }

z_result_t _z_task_detach(_z_task_t *task) { return -1; }

z_result_t _z_task_cancel(_z_task_t *task) { return -1; }

void _z_task_exit(void) {}

void _z_task_free(_z_task_t **task) {
    _z_task_t *ptr = *task;
    z_free(ptr);
    *task = NULL;
}

/*------------------ Mutex ------------------*/
z_result_t _z_mutex_init(_z_mutex_t *m) { return -1; }

z_result_t _z_mutex_drop(_z_mutex_t *m) { return -1; }

z_result_t _z_mutex_lock(_z_mutex_t *m) { return -1; }

z_result_t _z_mutex_try_lock(_z_mutex_t *m) { return -1; }

z_result_t _z_mutex_unlock(_z_mutex_t *m) { return -1; }

/*------------------ Condvar ------------------*/
z_result_t _z_condvar_init(_z_condvar_t *cv) { return -1; }

z_result_t _z_condvar_drop(_z_condvar_t *cv) { return -1; }

z_result_t _z_condvar_signal(_z_condvar_t *cv) { return -1; }
z_result_t _z_condvar_signal_all(_z_condvar_t *cv) { return -1; }

z_result_t _z_condvar_wait(_z_condvar_t *cv, _z_mutex_t *m) { return -1; }
z_result_t _z_condvar_wait_until(_z_condvar_t *cv, _z_mutex_t *m, const z_clock_t *abstime) { return -1; }
#endif  // Z_FEATURE_MULTI_THREAD == 1

/*------------------ Sleep ------------------*/
z_result_t z_sleep_us(size_t time) {
    delayMicroseconds(time);
    return 0;
}

z_result_t z_sleep_ms(size_t time) {
    delay(time);
    return 0;
}

z_result_t z_sleep_s(size_t time) {
    z_time_t start = z_time_now();

    // Most sleep APIs promise to sleep at least whatever you asked them to.
    // This may compound, so this approach may make sleeps longer than expected.
    // This extra check tries to minimize the amount of extra time it might sleep.
    while (z_time_elapsed_s(&start) < time) {
        z_sleep_ms(1000);
    }

    return 0;
}

/*------------------ Instant ------------------*/
void __z_clock_gettime(z_clock_t *ts) {
    uint64_t m = millis();
    ts->tv_sec = m / (uint64_t)1000;
    ts->tv_nsec = (m % (uint64_t)1000) * (uint64_t)1000000;
}

z_clock_t z_clock_now(void) {
    z_clock_t now;
    __z_clock_gettime(&now);
    return now;
}

unsigned long z_clock_elapsed_us(z_clock_t *instant) {
    z_clock_t now;
    __z_clock_gettime(&now);

    unsigned long elapsed = (1000000 * (now.tv_sec - instant->tv_sec) + (now.tv_nsec - instant->tv_nsec) / 1000);
    return elapsed;
}

unsigned long z_clock_elapsed_ms(z_clock_t *instant) {
    z_clock_t now;
    __z_clock_gettime(&now);

    unsigned long elapsed = (1000 * (now.tv_sec - instant->tv_sec) + (now.tv_nsec - instant->tv_nsec) / 1000000);
    return elapsed;
}

unsigned long z_clock_elapsed_s(z_clock_t *instant) {
    z_clock_t now;
    __z_clock_gettime(&now);

    unsigned long elapsed = now.tv_sec - instant->tv_sec;
    return elapsed;
}

void z_clock_advance_us(z_clock_t *clock, unsigned long duration) {
    clock->tv_sec += duration / 1000000;
    clock->tv_nsec += (duration % 1000000) * 1000;

    if (clock->tv_nsec >= 1000000000) {
        clock->tv_sec += 1;
        clock->tv_nsec -= 1000000000;
    }
}

void z_clock_advance_ms(z_clock_t *clock, unsigned long duration) {
    clock->tv_sec += duration / 1000;
    clock->tv_nsec += (duration % 1000) * 1000000;

    if (clock->tv_nsec >= 1000000000) {
        clock->tv_sec += 1;
        clock->tv_nsec -= 1000000000;
    }
}

void z_clock_advance_s(z_clock_t *clock, unsigned long duration) { clock->tv_sec += duration; }

/*------------------ Time ------------------*/
z_time_t z_time_now(void) {
    z_time_t now;
    gettimeofday(&now, NULL);
    return now;
}

const char *z_time_now_as_str(char *const buf, unsigned long buflen) {
    z_time_t tv = z_time_now();
    struct tm ts;
    ts = *localtime(&tv.tv_sec);
    strftime(buf, buflen, "%Y-%m-%dT%H:%M:%SZ", &ts);
    return buf;
}

unsigned long z_time_elapsed_us(z_time_t *time) {
    z_time_t now;
    gettimeofday(&now, NULL);

    unsigned long elapsed = (1000000 * (now.tv_sec - time->tv_sec) + (now.tv_usec - time->tv_usec));
    return elapsed;
}

unsigned long z_time_elapsed_ms(z_time_t *time) {
    z_time_t now;
    gettimeofday(&now, NULL);

    unsigned long elapsed = (1000 * (now.tv_sec - time->tv_sec) + (now.tv_usec - time->tv_usec) / 1000);
    return elapsed;
}

unsigned long z_time_elapsed_s(z_time_t *time) {
    z_time_t now;
    gettimeofday(&now, NULL);

    unsigned long elapsed = now.tv_sec - time->tv_sec;
    return elapsed;
}

z_result_t _z_get_time_since_epoch(_z_time_since_epoch *t) {
    z_time_t now;
    gettimeofday(&now, NULL);
    t->secs = now.tv_sec;
    t->nanos = now.tv_usec * 1000;
    return 0;
}
#endif