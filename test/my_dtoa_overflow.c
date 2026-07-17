/* Copyright (c) 2018-Present MongoDB Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/*
  SECBUG-2210 regression test.

  Reproduces the stack buffer overflow that occurs when a large-magnitude
  DOUBLE/FLOAT is formatted with "%f" into the fixed 50-byte buffers used by
  the driver (see driver/my_prepared_stmt.c my_f_to_a(), driver/results.c and
  driver/cursor.c "char as_string[50]").

  The driver formats such values via my_snprintf(buf, 49, "%f", value), which
  flows my_snprintf -> my_vsnprintf -> process_dbl_arg -> my_fcvt. Before the
  fix, process_dbl_arg clamps only the *precision* argument, while my_fcvt
  writes integer_digits + '.' + precision bytes with no buffer bound, so a
  value near 1e308 writes ~340 bytes into the 50-byte destination.

  This is a self-contained unit test: it links the mysql_strings library and
  calls my_snprintf directly. It needs no ODBC driver manager and no database
  connection, so it must NOT use the odbctap.h harness. Overflow is detected
  deterministically (without a sanitizer) by wrapping the destination buffer
  in sentinel "guard" regions and asserting they are untouched after each call.
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/*
  Declared here rather than via m_string.h to keep the test independent of the
  plugin-service my_snprintf macro indirection; this is the plain library
  symbol defined in mysql_strings/my_vsnprintf.c.
*/
extern size_t my_snprintf(char *to, size_t n, const char *fmt, ...);

/* Mirrors the driver: a 50-byte destination, filled by my_snprintf(buf, 49, ...). */
#define DEST_SIZE 50
#define DEST_LIMIT 49
#define GUARD_LO 32
#define GUARD_HI 512
#define SENTINEL 0xAB

static int failures= 0;

/*
  Format `value` into a sentinel-guarded destination and verify that the call
  stayed within the 50-byte buffer. Returns the number of guard bytes that were
  corrupted (0 == safe).
*/
static int check_no_overflow(const char *label, double value)
{
  struct
  {
    unsigned char guard_lo[GUARD_LO];
    char          dest[DEST_SIZE];
    unsigned char guard_hi[GUARD_HI];
  } framed;
  size_t i, corrupted= 0;

  memset(&framed, SENTINEL, sizeof(framed));

  my_snprintf(framed.dest, DEST_LIMIT, "%f", value);

  for (i= 0; i < GUARD_LO; i++)
    if (framed.guard_lo[i] != (unsigned char) SENTINEL)
      corrupted++;
  for (i= 0; i < GUARD_HI; i++)
    if (framed.guard_hi[i] != (unsigned char) SENTINEL)
      corrupted++;

  if (corrupted)
  {
    printf("not ok - %s: %zu guard byte(s) clobbered "
           "(buffer overflow of the %d-byte destination)\n",
           label, corrupted, DEST_SIZE);
    failures++;
  }
  else
  {
    printf("ok - %s: no overflow\n", label);
  }
  return (int) corrupted;
}

/*
  Same call driven into an exact-size heap allocation. Under AddressSanitizer
  the pre-fix overflow crosses the allocation's red zone and is reported as a
  heap-buffer-overflow originating in my_fcvt; the post-fix code stays in
  bounds. (Without a sanitizer this relies on the stack-guard check above; this
  variant exists specifically to give ASan a red zone to trip on.)
*/
static void check_heap_no_overflow(const char *label, double value)
{
  char *dest= (char *) malloc(DEST_SIZE);

  if (dest == NULL)
  {
    printf("not ok - %s: malloc failed\n", label);
    failures++;
    return;
  }
  my_snprintf(dest, DEST_LIMIT, "%f", value);
  printf("ok - %s: heap call in bounds\n", label);
  free(dest);
}

/*
  For values that fit, the "%f" output must be byte-for-byte identical to the
  platform snprintf and must be NUL-terminated within the limit.
*/
static void check_matches_libc(const char *label, double value)
{
  char got[DEST_SIZE];
  char want[DEST_SIZE];

  memset(got, SENTINEL, sizeof(got));
  my_snprintf(got, DEST_LIMIT, "%f", value);
  snprintf(want, sizeof(want), "%f", value);

  if (strcmp(got, want) != 0)
  {
    printf("not ok - %s: got \"%s\", want \"%s\"\n", label, got, want);
    failures++;
  }
  else
  {
    printf("ok - %s: \"%s\"\n", label, got);
  }
}

int main(void)
{
  printf("# SECBUG-2210: my_snprintf(\"%%f\") 50-byte buffer overflow\n");

  /* 1-3: attacker-controlled large magnitudes must not overflow. */
  check_no_overflow("overflow_double_max_1e308", 1e308);
  check_no_overflow("overflow_negative_1e308", -1e308);
  check_no_overflow("overflow_dbl_max", DBL_MAX);
  check_no_overflow("boundary_1e42", 1e42);

  /* Heap-backed variants so AddressSanitizer has a red zone to catch the
     pre-fix overflow independently of the stack-guard check. */
  check_heap_no_overflow("heap_double_max_1e308", 1e308);
  check_heap_no_overflow("heap_dbl_max", DBL_MAX);

  /* 4: normal values keep exact, regression-free behavior. */
  check_matches_libc("normal_pi", 3.14);
  check_matches_libc("normal_zero", 0.0);
  check_matches_libc("normal_negative", -0.5);
  check_matches_libc("normal_midsize", 123456.789);
  check_no_overflow("normal_pi_guard", 3.14);

  /* 5: NaN / infinities take my_fcvt's overflow path and must not write OOB. */
  check_no_overflow("special_nan", (double) NAN);
  check_no_overflow("special_inf", (double) INFINITY);
  check_no_overflow("special_neg_inf", (double) -INFINITY);

  /* 6: tiny / subnormal magnitudes. */
  check_no_overflow("tiny_1e-300", 1e-300);
  check_no_overflow("tiny_subnormal", DBL_MIN / 4.0);

  if (failures)
  {
    printf("# FAILED: %d check(s)\n", failures);
    return 1;
  }
  printf("# PASSED\n");
  return 0;
}
