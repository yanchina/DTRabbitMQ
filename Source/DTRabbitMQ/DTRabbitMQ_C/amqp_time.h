// Copyright 2024 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#ifndef AMQP_TIMER_H
#define AMQP_TIMER_H

#include <stdint.h>

#if ((defined(_WIN32)) || (defined(__MINGW32__)) || (defined(__MINGW64__)))
#ifndef WINVER
#define WINVER 0x0502
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

#define AMQP_MS_PER_S 1000
#define AMQP_US_PER_MS 1000
#define AMQP_NS_PER_S 1000000000
#define AMQP_NS_PER_MS 1000000
#define AMQP_NS_PER_US 1000

/* This represents a point in time in reference to a monotonic clock.
 *
 * The internal representation is ns, relative to the monotonic clock.
 *
 * There are two 'special' values:
 * - 0: means 'this instant', its meant for polls with a 0-timeout, or
 *   non-blocking option
 * - UINT64_MAX: means 'at infinity', its mean for polls with an infinite
 *   timeout
 */
typedef struct amqp_time_t_ {
  uint64_t time_point_ns;
} amqp_time_t;

/* Gets a monotonic timestamp. This will return 0 if the underlying call to the
 * system fails.
 */
uint64_t amqp_get_monotonic_timestamp(void);

/* Get a amqp_time_t that is timeout from now.
 * If timeout is NULL, an amqp_time_infinite() is created.
 *
 * Returns AMQP_STATUS_OK on success.
 * AMQP_STATUS_INVALID_PARAMETER if timeout is invalid
 * AMQP_STATUS_TIMER_FAILURE if the underlying call to get the current timestamp
 * fails.
 */
int amqp_time_from_now(amqp_time_t *time, const struct timeval *timeout);

/* Get a amqp_time_t that is seconds from now.
 * If seconds <= 0, then amqp_time_infinite() is created.
 *
 * Returns AMQP_STATUS_OK on success.
 * AMQP_STATUS_TIMER_FAILURE if the underlying call to get the current timestamp
 * fails.
 */
int amqp_time_s_from_now(amqp_time_t *time, int seconds);

/* Create an infinite amqp_time_t */
amqp_time_t amqp_time_infinite(void);

/* Gets the number of ms until the amqp_time_t, suitable for the timeout
 * parameter in poll().
 *
 * -1 will be returned for amqp_time_infinite values.
 * 0 will be returned for amqp_time_immediate values.
 * AMQP_STATUS_TIMEOUT will be returned if time was in the past.
 * AMQP_STATUS_TIMER_FAILURE will be returned if the underlying call to get the
 * current timestamp fails.
 */
int amqp_time_ms_until(amqp_time_t time);

/* Gets a timeval filled in with the time until amqp_time_t. Suitable for the
 * parameter in select().
 *
 * The in parameter specifies a storage location for *out.
 * If time is an inf timeout, then *out = NULL.
 * If time is a 0-timeout or the timer has expired, then *out = {0, 0}
 * Otherwise *out is set to the time left on the time.
 *
 * AMQP_STATUS_OK will be returned if successfully filled.
 * AMQP_STATUS_TIMER_FAILURE is returned when the underlying call to get the
 * current timestamp fails.
 */
int amqp_time_tv_until(amqp_time_t time, struct timeval *in,
                       struct timeval **out);

/* Test whether current time is past the provided time.
 *
 * TODO: this isn't a great interface to use. Fix this.
 *
 * Return AMQP_STATUS_OK if time has not past
 * Return AMQP_STATUS_TIMEOUT if time has past
 * Return AMQP_STATUS_TIMER_FAILURE if the underlying call to get the current
 * timestamp fails.
 */
int amqp_time_has_past(amqp_time_t time);

/* Return the time value that happens first */
amqp_time_t amqp_time_first(amqp_time_t l, amqp_time_t r);

int amqp_time_equal(amqp_time_t l, amqp_time_t r);
#endif /* AMQP_TIMER_H */
