project('voxelUsdPlugin', 'cpp')

# Build the USD plugins
USD_PLUGINS = []

# Relative to the prefix (such as /usr/local)

if get_option('dcc') == 'standalone'
    LIB_PATH = 'lib'
    LIB_USD_PATH = LIB_PATH / 'usd'
    LIB_PYTHON_PATH = LIB_PATH / 'python'

    usd_dep = dependency(
        'pxr',
        method : 'cmake'
    )
elif get_option('dcc') == 'houdini'
    # meson configure -Ddcc=houdini
    LIB_PATH = get_option('houdiniPath') / 'dsolib'
    LIB_USD_PATH = LIB_PATH / 'usd_plugins'
    # TODO - don't hardcode the python version
    LIB_PYTHON_PATH = get_option('houdiniPath') / 'python/lib/python3.11/site-packages'

    usd_dep = dependency(
        'Houdini',
        method : 'cmake',
        modules : ['Houdini', 'HoudiniThirdParty']
    )
elif get_option('dcc') == 'maya'
    # meson configure -Ddcc=maya
    LIB_PATH = get_option('mayaUsdPath') / 'lib'
    LIB_USD_PATH = LIB_PATH / 'usd'
    LIB_PYTHON_PATH = get_option('mayaUsdPath') / 'lib/python'

    usd_dep = dependency(
        'pxr',
        method : 'cmake'
    )
endif

subdir('usdVoxel')
subdir('usdmeson')
