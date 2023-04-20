project('rba_simple', 'c',
  default_options: [
    'b_lto=true',
    'b_staticpic=false',
    'c_std=c2x',
    'warning_level=3',
  ],
  license: ['Apache-2.0'],
  version: '0.1.0')

rba_lib = dependency('rba_atp')

# FIXME There are way too many defines the SDK needs-- all of these were in use
# by the Makefile used to build for the Artemis ATP.

c_args = [
  '-DAM_CUSTOM_BDADDR', '-DAM_PACKAGE_BGA', '-DWSF_TRACE_ENABLED',
  '-DWSF_TRACE_ENABLED', '-DWSF_TRACE_ENABLED', '-DAM_PART_APOLLO3',
  '-DPART_APOLLO3', '-ffunction-sections', '-fno-delete-null-pointer-checks',
]

link_args = [
  '--specs=nano.specs', '-nostartfiles', '-Wl,--gc-sections',
  '-fno-exceptions', '-fno-delete-null-pointer-checks', 
]

# Library

lib_sources = files([
  'src/uart.c',
  'src/adc.c',
  'src/spi.c',
  'src/lora.c',
  'src/gpio.c'
])

includes = include_directories([
  'include/rba_simple',
])

lib = library('rba_simple',
  lib_sources,
  dependencies: [rba_lib],
  include_directories: includes,
  c_args: c_args,
  link_args: link_args,
  install: true,
)

install_subdir('include', install_dir: '')

# Executables

sources = files([
  'src/main.c',
])

cc = meson.get_compiler('c', native: false)
m_dep = cc.find_library('m', required : false)

exe = executable(meson.project_name(),
  sources,
  dependencies: [rba_lib, m_dep],
  link_with: lib,
  include_directories: includes,
  c_args: c_args,
  link_args: link_args + ['-T' + meson.source_root() / 'linker.ld']
)

# Custom flash target, to make it easier to program the RedBoard

objcopy = find_program('objcopy')

bin = custom_target(
  input : exe,
  output : exe.name().split('.')[0] + '.bin',
  command : [objcopy, '-O', 'binary', '@INPUT@', '@OUTPUT@', ],
  build_by_default: true
)

run_target('flash',
  command : ['python3', meson.source_root() / 'svl.py',
    get_option('tty'), '-f',  bin, '-b', '921600', '-v'],
  depends : bin,
)

# Create a pkgconfig file

pkg = import('pkgconfig')

pkg.generate(lib, subdirs: ['', 'rba_simple'])
