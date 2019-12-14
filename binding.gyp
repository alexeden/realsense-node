{
  'variables': {
    'configuration%': '${BUILDTYPE}',
    'build_arch': '<!(node -p "process.arch")'
  },
  "targets": [
    {
      "target_name": "realsense_node",
      "sources": [
        "src/addon.cc",
      ],
      'include_dirs': [
        "./librealsense/include",
        "<!@(node -p \"require('node-addon-api').include\")",
        "<!@(node -p \"require('napi-thread-safe-callback').include\")",
      ],
      'dependencies': [
        "<!(node -p \"require('node-addon-api').gyp\")",
      ],
      "cflags!": [
        "-fno-exceptions"
      ],
      "cflags": [
        "-Wno-deprecated-declarations",
        "-Wno-switch",
        "-std=c++11",
        "-fstack-protector-strong",
        "-fPIE -fPIC",
        "-O2 -D_FORTIFY_SOURCE=2",
        "-Wformat -Wformat-security"
      ],
      "cflags_cc!": [
        "-fno-exceptions"
      ],
      "cflags_cc": [
        "-Wno-switch",
        "-Wno-deprecated-declarations"
      ],
      'conditions': [
        [
          'OS=="mac"',
          {
            "libraries": [
              '<(module_root_dir)/librealsense/build/<(configuration)/librealsense2.dylib',
              '-Wl,-rpath,@loader_path/../../librealsense/build/<(configuration)',
            ],
            'xcode_settings': {
              'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
              'CLANG_CXX_LIBRARY': 'libc++',
              'MACOS_DEPLOYMENT_TARGET': '10.12',
              'CLANG_CXX_LANGUAGE_STANDARD': 'c++14'
            }
          }
        ],
        [
          'OS=="linux"',
          {
            "libraries": [
              "-lrealsense2"
            ],
            'ldflags': [
              # rpath for build from source
              '-Wl,-rpath,\$$ORIGIN/../../../../build',
              '-L<(module_root_dir)/../../build',
              # rpatch for build debian package
              '-Wl,-rpath,\$$ORIGIN/../../../../obj-x86_64-linux-gnu',
              '-L<(module_root_dir)/../../obj-x86_64-linux-gnu'
           ],
            "cflags+": [
              "-std=c++11"
            ],
            "cflags_c+": [
              "-std=c++14"
            ],
            "cflags_cc+": [
              "-std=c++14"
            ]
          }
        ],
      ],
    }
    # {
		# 	"target_name": "vendor",
		# 	"type": "static_library",
    #   "defines": [
    #     "DREMOVE_DEPRECATED_TRANSFORMERS"
    #   ],
    #   'conditions': [
    #     [ 'OS!="linux"', {
    #       'sources': [
    #         # './src/unsupported.cc'
    #       ]
    #     }],
    #     [ 'OS=="linux"', {
    #       'sources': [
    #         # "./vendor/lib/thread.cc",
    #         # "./vendor/lib/pixel-mapper.cc",
    #         # "./vendor/lib/options-initialize.cc",
    #         # "./vendor/lib/multiplex-mappers.cc",
    #         # "./vendor/lib/led-matrix-c.cc",
    #         # "./vendor/lib/led-matrix.cc",
    #         # "./vendor/lib/graphics.cc",
    #         # "./vendor/lib/gpio.cc",
    #         # "./vendor/lib/framebuffer.cc",
    #         # "./vendor/lib/content-streamer.cc",
    #         # "./vendor/lib/bdf-font.cc",
    #         # "./vendor/lib/hardware-mapping.c"
    #       ],
    #       "libraries": ["-lrt", "-lm", "-lpthread"],
    #       "include_dirs": [
    #         # "./vendor/include"
    #       ],
    #       "direct_dependent_settings": {
    #         "include_dirs": [
    #           # "./vendor/include"
    #         ]
    #       }
    #     }]
    #   ]
    # }
  ]
}
