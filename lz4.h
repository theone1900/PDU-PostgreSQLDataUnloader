/*
 * Minimal LZ4 block decompression interface used by PDU.
 *
 * This project only needs LZ4_decompress_safe().  Providing the declaration
 * locally lets the project build in environments where the liblz4 development
 * package is unavailable.
 */
#ifndef PDU_LZ4_H
#define PDU_LZ4_H

#ifdef __cplusplus
extern "C" {
#endif

int LZ4_decompress_safe(const char *source, char *dest, int compressedSize, int maxDecompressedSize);

#ifdef __cplusplus
}
#endif

#endif
