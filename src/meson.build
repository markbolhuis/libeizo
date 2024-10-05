mod_py = import('python')

prog_python = mod_py.find_installation('python3')

usage_to_str_py = files('usage_to_str.py')

usage_to_str_c = custom_target('usage_to_str',
  input: 'internal.h',
  output: 'usage_to_str.c',
  command: [prog_python, usage_to_str_py, '@INPUT@', '@OUTPUT@'],
)

src_eizo = [
  'handle.c',
  'control.c',
  'debug.c',
  'hid.c',
  usage_to_str_c,
]

lib_eizo = library(
  'eizo', 
  src_eizo,
  dependencies : [
    dep_systemd,
  ],
  include_directories : inc,
  version : v_str,
  install : true,
)
