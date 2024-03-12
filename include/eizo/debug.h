#pragma once

typedef struct eizo_handle *eizo_handle_t;

void
eizo_dbg_dump_secondary_descriptor(eizo_handle_t handle);

void
eizo_dbg_dump_ff300009(eizo_handle_t handle);

void
eizo_dbg_dump_eeprom(eizo_handle_t handle);

void
eizo_dbg_dump_available_custom_key_lock(eizo_handle_t handle);

void
eizo_dbg_dump_ff01010e(eizo_handle_t handle);

void
eizo_dbg_dump_edid(eizo_handle_t handle);

void
eizo_dbg_dump_ff020059(eizo_handle_t handle);

void
eizo_dbg_dump_gain_definition(eizo_handle_t handle);

int
eizo_dbg_poll(eizo_handle_t handle);
