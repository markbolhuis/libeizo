#include <stdio.h>
#include <memory.h>

#include "internal.h"

#define HID_GLOBAL_STACK_LEN 4

enum hid_type : uint8_t {
    HID_TYPE_MAIN = 0,
    HID_TYPE_GLOBAL = 1,
    HID_TYPE_LOCAL = 2,
    HID_TYPE_RESERVED = 3,
};

enum hid_tag_main : uint8_t {
    HID_TAG_MAIN_INPUT = 8,
    HID_TAG_MAIN_OUTPUT = 9,
    HID_TAG_MAIN_COLLECTION = 10,
    HID_TAG_MAIN_FEATURE = 11,
    HID_TAG_MAIN_END_COLLECTION = 12,
};

enum hid_tag_global : uint8_t {
    HID_TAG_GLOBAL_USAGE_PAGE = 0,
    HID_TAG_GLOBAL_LOGICAL_MINIMUM = 1,
    HID_TAG_GLOBAL_LOGICAL_MAXIMUM = 2,
    HID_TAG_GLOBAL_PHYSICAL_MINIMUM = 3,
    HID_TAG_GLOBAL_PHYSICAL_MAXIMUM = 4,
    HID_TAG_GLOBAL_UNIT_EXPONENT = 5,
    HID_TAG_GLOBAL_UNIT = 6,
    HID_TAG_GLOBAL_REPORT_SIZE = 7,
    HID_TAG_GLOBAL_REPORT_ID = 8,
    HID_TAG_GLOBAL_REPORT_COUNT = 9,
    HID_TAG_GLOBAL_PUSH = 10,
    HID_TAG_GLOBAL_POP = 11,
};

enum hid_tag_local : uint8_t {
    HID_TAG_LOCAL_USAGE = 0,
    HID_TAG_LOCAL_USAGE_MINIMUM = 1,
    HID_TAG_LOCAL_USAGE_MAXIMUM = 2,
    HID_TAG_LOCAL_DESIGNATOR_INDEX = 3,
    HID_TAG_LOCAL_DESIGNATOR_MINIMUM = 4,
    HID_TAG_LOCAL_DESIGNATOR_MAXIMUM = 5,
    HID_TAG_LOCAL_STRING_INDEX = 7,
    HID_TAG_LOCAL_STRING_MINIMUM = 8,
    HID_TAG_LOCAL_STRING_MAXIMUM = 9,
    HID_TAG_LOCAL_DELIMITER = 10,
};

enum : uint8_t {
    HID_TAG_LONG = 15,
};

struct hid_item {
    uint8_t size;
    uint8_t type;
    uint8_t tag;
    union {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;

        int8_t i8;
        int16_t i16;
        int32_t i32;
    } data;
};

struct hid_global {
    uint32_t usage_page;
    int32_t logical_minimum;
    int32_t logical_maximum;
    int32_t physical_minimum;
    int32_t physical_maximum;
    int32_t unit_exponent;
    uint32_t unit;
    uint32_t report_size;
    uint32_t report_id;
    uint32_t report_count;
};

struct hid_local {
    uint32_t usage;
    uint32_t usage_minimum;
    uint32_t usage_maximum;
    uint32_t designator_index;
    uint32_t designator_minimum;
    uint32_t designator_maximum;
    uint32_t string_index;
    uint32_t string_minimum;
    uint32_t string_maximum;
    uint32_t delimiter;
};

struct hid_parser {
    struct hid_local local;
    struct hid_global global;
    struct hid_global global_stack[HID_GLOBAL_STACK_LEN];
    int global_ptr;
};

static uint32_t
hid_item_udata(const struct hid_item *item)
{
    switch (item->size) {
        case 1:
            return item->data.u8;
        case 2:
            return item->data.u16;
        case 4:
            return item->data.u32;
        default:
            unreachable();
    }
}

static int32_t
hid_item_sdata(const struct hid_item *item)
{
    switch (item->size) {
        case 1:
            return item->data.i8;
        case 2:
            return item->data.i16;
        case 4:
            return item->data.i32;
        default:
            unreachable();
    }
}

static int
hid_parse_global(struct hid_parser *parser, const struct hid_item *item)
{
    switch (item->tag) {
        case HID_TAG_GLOBAL_USAGE_PAGE:
            parser->global.usage_page = hid_item_udata(item);
            break;

        case HID_TAG_GLOBAL_LOGICAL_MINIMUM:
            parser->global.logical_minimum = hid_item_sdata(item);
            break;

        case HID_TAG_GLOBAL_LOGICAL_MAXIMUM:
            parser->global.logical_maximum = hid_item_sdata(item);
            break;

        case HID_TAG_GLOBAL_PHYSICAL_MINIMUM:
            parser->global.physical_minimum = hid_item_sdata(item);
            break;

        case HID_TAG_GLOBAL_PHYSICAL_MAXIMUM:
            parser->global.physical_maximum = hid_item_sdata(item);
            break;

        case HID_TAG_GLOBAL_UNIT_EXPONENT:
            parser->global.unit_exponent = hid_item_sdata(item);
            break;

        case HID_TAG_GLOBAL_UNIT:
            parser->global.unit = hid_item_sdata(item);
            break;

        case HID_TAG_GLOBAL_REPORT_SIZE:
            parser->global.report_size = hid_item_udata(item);
            break;

        case HID_TAG_GLOBAL_REPORT_ID:
            parser->global.report_id = hid_item_udata(item);
            break;

        case HID_TAG_GLOBAL_REPORT_COUNT:
            parser->global.report_count = hid_item_udata(item);
            break;

        case HID_TAG_GLOBAL_PUSH:
            if (parser->global_ptr == HID_GLOBAL_STACK_LEN) {
                return -1;
            }
            parser->global_stack[parser->global_ptr++] = parser->global;
            break;

        case HID_TAG_GLOBAL_POP:
            if (parser->global_ptr == 0) {
                return -1;
            }
            parser->global = parser->global_stack[--parser->global_ptr];
            break;

        default:
            return -1;
    }

    return 0;
}

static int
hid_parse_local(struct hid_parser *parser, const struct hid_item *item)
{
    switch (item->tag) {
        case HID_TAG_LOCAL_USAGE:
            parser->local.usage = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_USAGE_MINIMUM:
            parser->local.usage_minimum = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_USAGE_MAXIMUM:
            parser->local.usage_maximum = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_DESIGNATOR_INDEX:
            parser->local.designator_index = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_DESIGNATOR_MINIMUM:
            parser->local.designator_minimum = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_DESIGNATOR_MAXIMUM:
            parser->local.designator_maximum = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_STRING_INDEX:
            parser->local.string_index = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_STRING_MINIMUM:
            parser->local.string_minimum = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_STRING_MAXIMUM:
            parser->local.string_maximum = hid_item_udata(item);
            break;

        case HID_TAG_LOCAL_DELIMITER:
            fprintf(stderr, "%s: HID delimiter item found, aborting\n", __func__);

        default:
            return -1;
    }

    return 0;
}

static int
hid_fetch_item(const uint8_t **_ptr, const uint8_t *end, struct hid_item *item)
{
    const uint8_t *ptr = *_ptr;

    if ((end - ptr) <= 0) {
        return -1;
    }

    struct hid_item tmp = {};

    uint8_t b = *ptr++;

    tmp.size = b & 3;
    tmp.type = (b & 12) >> 2;
    tmp.tag = b >> 4;

    if (tmp.tag == HID_TAG_LONG) {
        fprintf(stderr, "%s: HID long item found, aborting.\n", __func__);
        return -1;
    }

    if (tmp.size == 3) {
        ++tmp.size;
    }

    if ((end - ptr) < tmp.size) {
        return -1;
    }

    switch (tmp.size) {
        case 1:
            tmp.data.u8 = ptr[0];
            break;
        case 2:
            tmp.data.u16 = ptr[0] | ptr[1] << 8;
            break;
        case 4:
            tmp.data.u32 = ptr[0] | ptr[1] << 8 | ptr[2] << 16 | ptr[3] << 24;
            break;
        default:
            unreachable();
    }

    ptr += tmp.size;
    *item = tmp;
    *_ptr = ptr;

    return 0;
}

void
eizo_parse_descriptor(const uint8_t *desc, size_t len)
{
    struct hid_item item;
    const uint8_t *ptr = desc;
    const uint8_t *end = desc + len;

    struct hid_parser parser;
    struct hid_local *local = &parser.local;
    struct hid_global *global = &parser.global;

    while (true) {
        int item_len = hid_fetch_item(&ptr, end, &item);
        if (item_len < 0) {
            break;
        }

        switch (item.type) {
            case HID_TYPE_MAIN:
                if (item.tag == HID_TAG_MAIN_FEATURE) {
                    uint32_t usage = global->usage_page << 16 | local->usage;
                    const char *ustr = eizo_usage_to_string(usage);

                    printf("%3u | 0x%08x | %-40s | %4u | %5u | %11i | %11i |\n",
                           global->report_id,
                           usage,
                           ustr == nullptr ? "" : ustr,
                           global->report_size,
                           global->report_count,
                           global->logical_minimum,
                           global->logical_maximum);
                }
                memset(local, 0, sizeof(*local));
                break;

            case HID_TYPE_GLOBAL:
                hid_parse_global(&parser, &item);
                break;

            case HID_TYPE_LOCAL:
                hid_parse_local(&parser, &item);
                break;

            case HID_TYPE_RESERVED:
            default:
                break;
        }
    }
}
