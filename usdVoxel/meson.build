
USD_PLUGINS += [{
    'name' : 'usdVoxel',

    'sources': files(
        'UsdVoxelKvxFileFormat.cpp', 'kvx.h',
        'UsdVoxelVoxFileFormat.cpp', 'SdfMagicaVoxel.cpp', 'ogt_vox.cpp', 'ogt_vox.h',
    ),
    'plugInfo': files('plugInfo.json'),

    # declare dependencies for runtime (by OpenUSD's PluginRegistry)
    'usd_deps': [
        'sdf', 'tl', 'usdGeom'
    ],
    # additional arguments passed to shared_library()
    'lib_kwargs': {
    }
}]
