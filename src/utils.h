#pragma once

#include <stdint.h>

#define put_le16(ptr, val) (*((uint16_t *)(ptr)) = htole16(val))
#define put_le32(ptr, val) (*((uint32_t *)(ptr)) = htole32(val))
#define put_le64(ptr, val) (*((uint64_t *)(ptr)) = htole64(val))

#define get_le16(ptr) le16toh(*((uint16_t *)(ptr)))
#define get_le32(ptr) le32toh(*((uint32_t *)(ptr)))
#define get_le64(ptr) le32toh(*((uint64_t *)(ptr)))

#define put_be16(ptr, val) (*((uint16_t *)(ptr)) = htobe16(val))
#define put_be32(ptr, val) (*((uint32_t *)(ptr)) = htobe32(val))
#define put_be64(ptr, val) (*((uint64_t *)(ptr)) = htobe64(val))

#define get_be16(ptr) be16toh(*((uint16_t *)(ptr)))
#define get_be32(ptr) be32toh(*((uint32_t *)(ptr)))
#define get_be64(ptr) be64toh(*((uint64_t *)(ptr)))

static inline uint16_t
get_unaligned_le16(const uint8_t *ptr)
{
    return ((uint16_t)ptr[0]) << 0
         | ((uint16_t)ptr[1]) << 8;
}

static inline void
put_unaligned_le16(uint8_t *ptr, uint16_t value)
{
    ptr[0] = (uint8_t)(value >> 0);
    ptr[1] = (uint8_t)(value >> 8);
}

static inline uint32_t
get_unaligned_le32(const uint8_t *ptr)
{
    return ((uint32_t)ptr[0]) << 0
         | ((uint32_t)ptr[1]) << 8
         | ((uint32_t)ptr[2]) << 16
         | ((uint32_t)ptr[3]) << 24;
}

static inline void
put_unaligned_le32(uint8_t *ptr, uint32_t value)
{
    ptr[0] = (uint8_t)(value >> 0);
    ptr[1] = (uint8_t)(value >> 8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
}

static inline uint64_t
get_unaligned_le64(const uint8_t *ptr)
{
    return ((uint64_t)ptr[0]) << 0
         | ((uint64_t)ptr[1]) << 8
         | ((uint64_t)ptr[2]) << 16
         | ((uint64_t)ptr[3]) << 24
         | ((uint64_t)ptr[4]) << 32
         | ((uint64_t)ptr[5]) << 40
         | ((uint64_t)ptr[6]) << 48
         | ((uint64_t)ptr[7]) << 56;
}

static inline void
put_unaligned_le64(uint8_t *ptr, uint64_t value)
{
    ptr[0] = (uint8_t)(value >> 0);
    ptr[1] = (uint8_t)(value >> 8);
    ptr[2] = (uint8_t)(value >> 16);
    ptr[3] = (uint8_t)(value >> 24);
    ptr[4] = (uint8_t)(value >> 32);
    ptr[6] = (uint8_t)(value >> 40);
    ptr[6] = (uint8_t)(value >> 48);
    ptr[7] = (uint8_t)(value >> 56);
}

