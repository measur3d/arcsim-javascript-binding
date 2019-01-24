{
    'targets': [
        {
            'target_name': 'arcsim-binding-native',
            'sources': [
                'src/module.cpp',
                'src/arcsim_binding.cpp',
                'src/arcsim_translator.cpp',        
                'src/translation/arcsim_translation.cpp',
                'src/jsoncpp.cpp'
            ],
            'include_dirs': [
                "<!@(node -p \"require('node-addon-api').include\")",
                "<!@(node -p \"require('napi-thread-safe-callback').include\")",
                'src'
            ],
            'dependencies': [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            'cflags': [
                '-fexceptions', '-std=c++14', '-frtti'
            ],
            'cflags_cc': [
                '-fexceptions', '-std=c++14', '-frtti'
            ],
            'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                'CLANG_CXX_LIBRARY': 'libc++',
                'MACOSX_DEPLOYMENT_TARGET': '10.7',
                'OTHER_CFLAGS': [           
                    "-std=c++14",         
                    "-stdlib=libc++",
                    "-fexceptions",
                    "-frtti"
                ]
            },
            'msvs_settings': {
                'VCCLCompilerTool': { 'ExceptionHandling': 1 },
            },
            'conditions': [
                ['OS=="linux"', {
                    'defines': [
                        'PLATFORM_LINUX',
                    ]
                }],
                ['OS=="win"', {
                    'defines': [
                        'PLATFORM_WINDOWS',
                        '_HAS_EXCEPTIONS=1'
                    ]
                }],
                ['OS=="mac"', {
                    'defines': [
                        'PLATFORM_OSX',
                    ]
                }]        
            ]
        }
    ]
}
