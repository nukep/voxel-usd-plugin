# Please see the README.md in this directory for information

fs = import('fs')
prog_python = find_program('python3')

foreach plugin : USD_PLUGINS

    libname = plugin['name']    # usdMyPlugin

    PLUGIN_PATH = LIB_USD_PATH / libname
    RES_PATH = PLUGIN_PATH / 'resources'

    message('PLUGIN NAME:', libname)
    kwargs = {}
    if 'lib_kwargs' in plugin
        kwargs = plugin['lib_kwargs']
    endif

    deps = [usd_dep]

    if 'dependencies' in plugin
        deps = deps + plugin['dependencies']
    endif

    lib = shared_library(
        'usd_' + libname,
        sources: plugin['sources'],
        cpp_args : [
            # TF_REGISTRY_FUNCTION uses MFB_ALT_PACKAGE_NAME
            '-DMFB_ALT_PACKAGE_NAME=' + libname,
            '-DMFB_PACKAGE_NAME=' + libname
        ],
        dependencies: deps,
        install : true,
        install_dir : LIB_PATH,
        kwargs: kwargs
    )

    library_path = LIB_PATH / fs.name(lib.path())
    library_path = fs.relative_to(library_path, PLUGIN_PATH)

    cf = configure_file(
        input: plugin['plugInfo'][0],
        output: libname + '_plugInfo.json',
        configuration : {
            # relative to plugInfo.json
            'PLUG_INFO_ROOT': fs.relative_to(PLUGIN_PATH, RES_PATH),
            # relative to root
            'PLUG_INFO_LIBRARY_PATH': library_path,
            'PLUG_INFO_RESOURCE_PATH': 'resources',
        }
    )

    install_data(
        cf,
        rename : 'plugInfo.json',
        install_dir : RES_PATH
    )

    if 'resources' in plugin
        foreach res : plugin['resources']
            install_data(
                res,
                install_dir : RES_PATH
            )
        endforeach
    endif

    if 'py_module' in plugin
        py_module = plugin['py_module']     # 'pxr.UsdMyPlugin'
        python_path = py_module.split('.')  # ['pxr', 'UsdMyPlugin']
        python_subdir  = py_module.replace('.', '/')

        UpperName = python_path[-1]
        lowerName = UpperName[0].to_lower() + UpperName.substring(1)

        assert(
            libname == lowerName,
            'The plugin name must match the lowercased tail of the Python module name'
        )

        kwargs = {}
        if 'py_kwargs' in plugin
            kwargs = plugin['py_kwargs']
        endif

        sources = plugin['py_sources']

        PYTHON_MODULE_PATH = LIB_PYTHON_PATH / python_subdir

        deps_str = ''
        if 'usd_deps' in plugin
            deps_str = ','.join(plugin['usd_deps'])
        endif

        pymodule = custom_target(
            libname + '_pymodule',
            output : [libname + '_pymodule.cpp'],
            input : 'pymodule_genfiles.py',
            command : [prog_python, '@INPUT@', '@OUTPUT@', lowerName, py_module, deps_str],
        )

        sources += [pymodule]

        shared_library(
            '_' + lowerName,
            sources: sources,
            name_prefix : '',

            cpp_args : [
                # pyModule.h uses these to create PyInit_lib* and PyInit__*
                '-DMFB_ALT_PACKAGE_NAME=' + libname,
                '-DMFB_PACKAGE_NAME=' + libname,
                '-DMFB_PACKAGE_MODULE=' + UpperName
            ],
            dependencies: [usd_dep],
            link_with: [lib],
            install : true,
            install_dir : PYTHON_MODULE_PATH,
            kwargs: kwargs
        )

        install_data(
            'pymodule_init.py',
            rename : '__init__.py',
            install_dir : PYTHON_MODULE_PATH
        )
    endif

endforeach
