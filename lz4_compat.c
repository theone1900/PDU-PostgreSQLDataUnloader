/*
 * Minimal LZ4 block decompressor.
 *
 * Implements the public LZ4_decompress_safe() API used by PDU.  The decoder
 * follows the LZ4 block format and returns the number of bytes written to the
 * destination buffer, or a negative value when the input is malformed or the
 * destination buffer is too small.
 */
#include <stdint.h>
#include <string.h>

#include "lz4.h"

static int read_length(const unsigned char **ip, const unsigned char *iend, int initial, int *length)
{
    int len = initial;

    if (initial == 15) {
        unsigned int s;

        do {
            if (*ip >= iend)
                return -1;
            s = *(*ip)++;
            if (len > INT32_MAX - (int)s)
                return -1;
            len += (int)s;
        } while (s == 255);
    }

    *length = len;
    return 0;
}

int LZ4_decompress_safe(const char *source, char *dest, int compressedSize, int maxDecompressedSize)
{
    const unsigned char *ip = (const unsigned char *)source;
    const unsigned char *iend = ip + compressedSize;
    unsigned char *op = (unsigned char *)dest;
    unsigned char *oend = op + maxDecompressedSize;

    if (source == NULL || dest == NULL || compressedSize < 0 || maxDecompressedSize < 0)
        return -1;

    while (ip < iend) {
        unsigned int token = *ip++;
        int literal_length;
        int match_length;
        unsigned int offset;
        const unsigned char *match;

        if (read_length(&ip, iend, (int)(token >> 4), &literal_length) != 0)
            return -1;

        if (literal_length > iend - ip || literal_length > oend - op)
            return -1;

        memcpy(op, ip, (size_t)literal_length);
        ip += literal_length;
        op += literal_length;

        if (ip == iend)
            return (int)(op - (unsigned char *)dest);

        if (iend - ip < 2)
            return -1;

        offset = (unsigned int)ip[0] | ((unsigned int)ip[1] << 8);
        ip += 2;
        if (offset == 0 || offset > (unsigned int)(op - (unsigned char *)dest))
            return -1;

        if (read_length(&ip, iend, (int)(token & 0x0F), &match_length) != 0)
            return -1;
        match_length += 4;

        if (match_length > oend - op)
            return -1;

        match = op - offset;
        while (match_length-- > 0)
            *op++ = *match++;
    }

    return (int)(op - (unsigned char *)dest);
}
