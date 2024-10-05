project('libeizo', 'c',
  version : '0.1.0',
  license : 'LGPL-3',
  default_options : [
    'c_std=gnu23',
    'warning_level=3',
  ],
)

v_str = meson.project_version()
v_arr = v_str.split('.')
v_major = v_arr[0]
v_minor = v_arr[1]
v_patch = v_arr[2]

mod_pkg = import('pkgconfig')

inc = include_directories('include')

dep_systemd = dependency('libsystemd', version : '>=220')

subdir('include')
subdir('src')

executable('eizoctl', 'main.c',
  link_with : lib_eizo,
  include_directories : inc,
  install : true,
)

mod_pkg.generate(lib_eizo, subdirs : f'libeizo-@v_major@.@v_minor@')
