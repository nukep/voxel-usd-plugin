## USD Plugin building with Meson



The structure of each USD plugin is slightly opinionated, but is modeled roughly after OpenUSD's `pxr_plugin` macro.

By default, each plugin is installed alongside the builtin USD plugins in `<PREFIX>/lib/usd`.
This is probably what you want if you have an in-house OpenUSD distribution.

If a plugin has a Python module, then by default, it's installed in `<PREFIX>/lib/python` (notice the lack of version numbers, just like where OpenUSD installs its Python modules).
