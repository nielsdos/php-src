/* doaddsub.c: bcmath library file. */
/*
    Copyright (C) 1991, 1992, 1993, 1994, 1997 Free Software Foundation, Inc.
    Copyright (C) 2000 Philip A. Nelson

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.  (LICENSE)

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to:

      The Free Software Foundation, Inc.
      59 Temple Place, Suite 330
      Boston, MA 02111-1307 USA.

    You may contact the author by:
       e-mail:  philnelson@acm.org
      us-mail:  Philip A. Nelson
                Computer Science Department, 9062
                Western Washington University
                Bellingham, WA 98226-9062

*************************************************************************/

#include "bcmath.h"
#include "private.h"
#include <stddef.h>

#define IS_LITTLE_ENDIAN (*(unsigned char *)&(uint16_t){1})

/* This is based on the technique described in https://kholdstare.github.io/technical/2020/05/26/faster-integer-parsing.html.
 * This function transforms AABBCCDD into 1000 * AA + 100 * BB + 10 * CC + DD,
 * with the caveat that all components must be in the interval [0, 25] to prevent overflow
 * due to the multiplication by power of 10 (10 * 25 = 250 is the largest number that fits in a byte).
 * The advantage of this method instead of using shifts + 3 multiplications is that this is cheaper
 * due to its divide-and-conquer nature.
 *
 * The difference here however is that the first mask uses 0xff instead of 0x0f.
 * This doesn't make a difference when a byte is in the range [0, 9], but it has an extra advantage.
 * By setting the mask to 0xff we can already sum the two numbers in non-parsed form,
 * and the parsing will fix up the carry.
 * Here's how it works with an example:
 * Let's say A = [5 9 0 1] and B = [2 2 0 0], and we pass in A + B = [7 11 0 1]
 * We know that the highest number may only be 25 but that is no problem as the vectors only have
 * numbers in the range [0, 9] which means the sum vector has numbers in the range [0, 18].
 * Then [7 11 0 1] is transformed into 7 * 1000 + 11 * 100 + 0 * 10 + 1 = 8101,
 * which would have been the same as if the carry for the 11 would've been on the 7,
 * i.e. [8 1 0 1] would've been the right result, but that gives 8 * 1000 + 1 * 100 + 0 * 10 + 1 = *also* 8101.
 */
static uint32_t bc_parse_4_chars(uint32_t tmp)
{
	uint32_t lower_digits = (tmp & 0xff00ff00) >> 8;
	uint32_t upper_digits = (tmp & 0x00ff00ff) * 10;

	tmp = lower_digits + upper_digits;

	lower_digits = (tmp & 0x00ff0000) >> 16;
	upper_digits = (tmp & 0x000000ff) * 100;

	return lower_digits + upper_digits;
}

/* This LUT encodes the decimal representation of numbers 0-100
 * such that we can avoid taking modulos and divisions which would be slow. */
static const unsigned short LUT[100] = {
		0 | 0 << 8,
		0 | 1 << 8,
		0 | 2 << 8,
		0 | 3 << 8,
		0 | 4 << 8,
		0 | 5 << 8,
		0 | 6 << 8,
		0 | 7 << 8,
		0 | 8 << 8,
		0 | 9 << 8,
		1 | 0 << 8,
		1 | 1 << 8,
		1 | 2 << 8,
		1 | 3 << 8,
		1 | 4 << 8,
		1 | 5 << 8,
		1 | 6 << 8,
		1 | 7 << 8,
		1 | 8 << 8,
		1 | 9 << 8,
		2 | 0 << 8,
		2 | 1 << 8,
		2 | 2 << 8,
		2 | 3 << 8,
		2 | 4 << 8,
		2 | 5 << 8,
		2 | 6 << 8,
		2 | 7 << 8,
		2 | 8 << 8,
		2 | 9 << 8,
		3 | 0 << 8,
		3 | 1 << 8,
		3 | 2 << 8,
		3 | 3 << 8,
		3 | 4 << 8,
		3 | 5 << 8,
		3 | 6 << 8,
		3 | 7 << 8,
		3 | 8 << 8,
		3 | 9 << 8,
		4 | 0 << 8,
		4 | 1 << 8,
		4 | 2 << 8,
		4 | 3 << 8,
		4 | 4 << 8,
		4 | 5 << 8,
		4 | 6 << 8,
		4 | 7 << 8,
		4 | 8 << 8,
		4 | 9 << 8,
		5 | 0 << 8,
		5 | 1 << 8,
		5 | 2 << 8,
		5 | 3 << 8,
		5 | 4 << 8,
		5 | 5 << 8,
		5 | 6 << 8,
		5 | 7 << 8,
		5 | 8 << 8,
		5 | 9 << 8,
		6 | 0 << 8,
		6 | 1 << 8,
		6 | 2 << 8,
		6 | 3 << 8,
		6 | 4 << 8,
		6 | 5 << 8,
		6 | 6 << 8,
		6 | 7 << 8,
		6 | 8 << 8,
		6 | 9 << 8,
		7 | 0 << 8,
		7 | 1 << 8,
		7 | 2 << 8,
		7 | 3 << 8,
		7 | 4 << 8,
		7 | 5 << 8,
		7 | 6 << 8,
		7 | 7 << 8,
		7 | 8 << 8,
		7 | 9 << 8,
		8 | 0 << 8,
		8 | 1 << 8,
		8 | 2 << 8,
		8 | 3 << 8,
		8 | 4 << 8,
		8 | 5 << 8,
		8 | 6 << 8,
		8 | 7 << 8,
		8 | 8 << 8,
		8 | 9 << 8,
		9 | 0 << 8,
		9 | 1 << 8,
		9 | 2 << 8,
		9 | 3 << 8,
		9 | 4 << 8,
		9 | 5 << 8,
		9 | 6 << 8,
		9 | 7 << 8,
		9 | 8 << 8,
		9 | 9 << 8,
};

/* Writes the character representation of the number encoded in value.
 * E.g. if value = 1234, then the string "1234" will be written to str. */
static void bc_write_char_representation(uint32_t value, char *str)
{
	uint32_t upper = value / 100; /* e.g. 12 */
	uint32_t lower = value % 100; /* e.g. 34 */

	/* Note: little endian, so `lower` comes before `upper`! */
	uint32_t digits = LUT[lower] << 16 | LUT[upper];
	memcpy(str, &digits, sizeof(digits));
}

/* Perform addition: N1 is added to N2 and the value is
   returned.  The signs of N1 and N2 are ignored.
   SCALE_MIN is to set the minimum scale of the result. */

bc_num _bc_do_add(bc_num n1, bc_num n2, size_t scale_min)
{
	bc_num sum;
	size_t sum_scale, sum_digits;
	const char *n1ptr, *n2ptr;
	char *restrict sumptr;
	size_t n1bytes, n2bytes;
	bool carry;

	/* Prepare sum. */
	sum_scale = MAX (n1->n_scale, n2->n_scale);
	sum_digits = MAX (n1->n_len, n2->n_len) + 1;
	sum = bc_new_num (sum_digits, MAX(sum_scale, scale_min));

	/* Zero extra digits made by scale_min. */
	if (scale_min > sum_scale) {
		sumptr = (char * restrict) (sum->n_value + sum_scale + sum_digits);
		for (int count = scale_min - sum_scale; count > 0; count--) {
			*sumptr++ = 0;
		}
	}

	/* Start with the fraction part.  Initialize the pointers. */
	n1bytes = n1->n_scale;
	n2bytes = n2->n_scale;
	n1ptr = n1->n_value + n1->n_len + n1bytes - 1;
	n2ptr = n2->n_value + n2->n_len + n2bytes - 1;
	sumptr = (char * restrict) (sum->n_value + sum_scale + sum_digits - 1);

	/* Add the fraction part.  First copy the longer fraction.*/
	if (n1bytes != n2bytes) {
		if (n1bytes > n2bytes) {
			while (n1bytes > n2bytes) {
				*sumptr-- = *n1ptr--;
				n1bytes--;
			}
		} else {
			while (n2bytes > n1bytes) {
				*sumptr-- = *n2ptr--;
				n2bytes--;
			}
		}
	}

	/* Now add the remaining fraction part and equal size integer parts. */
	n1bytes += n1->n_len;
	n2bytes += n2->n_len;
	carry = 0;

	if (IS_LITTLE_ENDIAN) {
		/* The idea here is to work in a higher base, allowing summing digits in parallel. */
		while (n1bytes >= 4 && n2bytes >= 4) {
			n1ptr -= 4;
			n2ptr -= 4;
			sumptr -= 4;

			/* Fetches the two number chunks into tmp1 and tmp2. */
			uint32_t tmp1, tmp2;
			memcpy(&tmp1, n1ptr + 1, sizeof(tmp1));
			memcpy(&tmp2, n2ptr + 1, sizeof(tmp2));
			/* We don't have to parse tmp1 and tmp2 separately, we can perform
			 * the addition already and the parsing code will fix up the carry.
			 * See comment above bc_parse_4_chars(). We can only add the carry
			 * after parsing however because of endianness. */
			uint32_t tmp_sum = bc_parse_4_chars(tmp1 + tmp2) + carry;

			/* We're handling 4 digits at once, i.e. the base is 10**4 here. */
			if (tmp_sum >= 10000) {
				tmp_sum -= 10000;
				carry = 1;
			} else {
				carry = 0;
			}

			bc_write_char_representation(tmp_sum, sumptr + 1);
			n1bytes -= 4;
			n2bytes -= 4;
		}
	}

	while ((n1bytes > 0) && (n2bytes > 0)) {
		*sumptr = *n1ptr-- + *n2ptr-- + carry;
		if (*sumptr > (BASE - 1)) {
			carry = 1;
			*sumptr -= BASE;
		} else {
			carry = 0;
		}
		sumptr--;
		n1bytes--;
		n2bytes--;
	}

	/* Now add carry the longer integer part. */
	if (n1bytes == 0) {
		n1bytes = n2bytes;
		n1ptr = n2ptr;
	}
	while (n1bytes-- > 0) {
		*sumptr = *n1ptr-- + carry;
		if (*sumptr > (BASE - 1)) {
			carry = true;
			*sumptr -= BASE;
		} else {
			carry = false;
		}
		sumptr--;
	}

	/* Set final carry. */
	if (carry) {
		*sumptr += 1;
	}

	/* Adjust sum and return. */
	_bc_rm_leading_zeros(sum);
	return sum;
}


/* Perform subtraction: N2 is subtracted from N1 and the value is
   returned.  The signs of N1 and N2 are ignored.  Also, N1 is
   assumed to be larger than N2.  SCALE_MIN is the minimum scale
   of the result. */
bc_num _bc_do_sub(bc_num n1, bc_num n2, size_t scale_min)
{
	bc_num diff;
	size_t diff_scale, diff_len;
	size_t min_scale, min_len;
	size_t borrow, count;
	int val;
	const char *n1ptr, *n2ptr;
	char *restrict diffptr;

	/* Allocate temporary storage. */
	diff_len = MAX(n1->n_len, n2->n_len);
	diff_scale = MAX(n1->n_scale, n2->n_scale);
	min_len = MIN(n1->n_len, n2->n_len);
	min_scale = MIN(n1->n_scale, n2->n_scale);
	diff = bc_new_num (diff_len, MAX(diff_scale, scale_min));

	/* Zero extra digits made by scale_min. */
	if (scale_min > diff_scale) {
		diffptr = (char * restrict) (diff->n_value + diff_len + diff_scale);
		for (count = scale_min - diff_scale; count > 0; count--) {
			*diffptr++ = 0;
		}
	}

	/* Initialize the subtract. */
	n1ptr = n1->n_value + n1->n_len + n1->n_scale - 1;
	n2ptr = n2->n_value + n2->n_len + n2->n_scale - 1;
	diffptr = (char * restrict) (diff->n_value + diff_len + diff_scale - 1);

	/* Subtract the numbers. */
	borrow = 0;

	/* Take care of the longer scaled number. */
	if (n1->n_scale != min_scale) {
		/* n1 has the longer scale */
		for (count = n1->n_scale - min_scale; count > 0; count--) {
			*diffptr-- = *n1ptr--;
		}
	} else {
		/* n2 has the longer scale */
		for (count = n2->n_scale - min_scale; count > 0; count--) {
			val = -*n2ptr-- - borrow;
			if (val < 0) {
				val += BASE;
				borrow = 1;
			} else {
				borrow = 0;
			}
			*diffptr-- = val;
		}
	}

	/* Now do the equal length scale and integer parts. */
	count = 0;

	if (IS_LITTLE_ENDIAN) {
		/* This follows the same idea as the addition acceleration. */
		while (count + 4 <= min_len + min_scale) {
			n1ptr -= 4;
			n2ptr -= 4;
			diffptr -= 4;
			count += 4;

			uint32_t tmp1, tmp2;
			memcpy(&tmp1, n1ptr + 1, sizeof(tmp1));
			memcpy(&tmp2, n2ptr + 1, sizeof(tmp2));
			/* temp{1,2} digits are in range [0, 9], so the difference is in range [-9, 9].
			 * As this algorithm requires digits to be in range [0, 25] we can add 9 to each digit. */
			const uint32_t nines = 9 << 24 | 9 << 16 | 9 << 8 | 9;
			int32_t tmp_diff = bc_parse_4_chars(nines + tmp1 - tmp2) - 9999 - borrow;

			if (tmp_diff < 0) {
				tmp_diff += 10000;
				borrow = 1;
			} else {
				borrow = 0;
			}

			bc_write_char_representation(tmp_diff, diffptr + 1);
		}
	}

	for (; count < min_len + min_scale; count++) {
		val = *n1ptr-- - *n2ptr-- - borrow;
		if (val < 0) {
			val += BASE;
			borrow = 1;
		} else {
			borrow = 0;
		}
		*diffptr-- = val;
	}

	/* If n1 has more digits than n2, we now do that subtract. */
	if (diff_len != min_len) {
		for (count = diff_len - min_len; count > 0; count--) {
			val = *n1ptr-- - borrow;
			if (val < 0) {
				val += BASE;
				borrow = 1;
			} else {
				borrow = 0;
			}
			*diffptr-- = val;
		}
	}

	/* Clean up and return. */
	_bc_rm_leading_zeros(diff);
	return diff;
}
