#pragma once

typedef struct eizo_handle *eizo_handle_t;

void
eizo_dbg_dump_descriptor(eizo_handle_t handle);

void
eizo_dbg_dump_ff300009(eizo_handle_t handle);
