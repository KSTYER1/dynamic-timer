/* Format-time token parser, ports the Lua advanced-timer DSL.
 *
 * Tokens (longest-match-first replacement order):
 *   %3t   thousandths (3 digits, 000..999)
 *   %2t   hundredths  (2 digits, 00..99)
 *   %t    tenths      (1 digit, 0..9)
 *   %0H   hours unbounded with leading zero (00..inf)
 *   %0M   minutes unbounded with leading zero (00..inf)
 *   %0S   seconds unbounded with leading zero (00..inf)
 *   %0h   hours mod 24 with leading zero (00..23)
 *   %0m   minutes mod 60 with leading zero (00..59)
 *   %0s   seconds mod 60 with leading zero (00..59)
 *   %H    hours unbounded (0..inf)
 *   %M    minutes unbounded (0..inf)
 *   %S    seconds unbounded (0..inf)
 *   %h    hours mod 24 (0..23)
 *   %m    minutes mod 60 (0..59)
 *   %s    seconds mod 60 (0..59)
 *   %d    days
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Format an absolute duration in nanoseconds into out[0..out_size-1].
 * Negative ns is clamped to 0. Always NUL-terminates. */
void format_time_ns(int64_t ns, const char *fmt, char *out, size_t out_size);

#ifdef __cplusplus
}
#endif
