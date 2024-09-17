project(
  'libeizo', 'c', 
  license : 'LGPL-3',
  default_options : [
    'c_std=gnu23',
    'warning_level=3',
  ],
)

inc = include_directories('include')

dep_systemd = dependency('libsystemd')

subdir('include')
subdir('src')

executable('eizoctl', 'main.c',
  link_with : lib_eizo,
  include_directories : inc,
)
