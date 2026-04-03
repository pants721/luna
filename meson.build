project(
  'luna',
  'cpp',
  meson_version : '>= 1.3.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
  ],
  version : '0.1',
)

incdir = include_directories('include')

sfml_dep = dependency('sfml')
# imgui_sfml_dep = dependency('imgui-sfml')
gl_dep = dependency('opengl')

dependencies = [
  sfml_dep,
  # imgui_sfml_dep,
  gl_dep,
]

exe = executable(
  'luna',
  'src/main.cpp',
  'src/sim_state.cpp',
  'src/gfx/renderer.cpp',
  cpp_args: [
    '-O3',
    '-march=native',
  ],
  include_directories: incdir,
  dependencies : dependencies,
  install : true,
)

test('basic', exe)
