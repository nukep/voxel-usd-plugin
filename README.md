# voxel-usd-plugin

A USD file format plugin for reading voxel models. MagicaVoxel (.vox) and slab6 (.kvx) are currently implemented.

## Goals

- [x] Build for Houdini
- [ ] Add variantsets to select between Mesh vs PointInstancer
- [ ] Set extents
- [ ] Build for Maya
- [ ] Implement animations (e.g. for MagicaVoxel)
- [ ] Implement MagicaVoxel materials
- [ ] MAYBE: Implement a writer to kvx and/or vox
- [ ] MAYBE: Support OpenVDB volumes


## How to use

You can pretty much have the plugin installed and open .vox and .kvx files as you would any .usd file.

```sh
usdview cars.vox
```

If you want to save out the resulting prims as a .usd file, you can do this by flattening it with usdcat:

```sh
usdcat -f cars.vox -o cars.usdc
```

## Building standalone

You'll need CMake and Meson installed.

OpenUSD should be installed as a dependency in your configured prefix path (such as /usr).

To build and install:

```sh
mkdir build
cd build
meson setup ..
meson compile
meson install
```

### Building for Houdini

It's the same as above, but replace the setup line with:

```sh
meson setup .. -Ddcc=houdini --cmake-prefix-path=/opt/hfs20.5/toolkit
```

where the path to Houdini is whichever version's on your machine.
