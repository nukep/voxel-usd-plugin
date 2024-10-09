import sys
import os

pymodule_cpp, libname, py_module, deps_str = sys.argv[1:]

deps = deps_str.split(',')

reqs_tftokens = ', '.join([f'TfToken("{x}")' for x in deps])

with open(pymodule_cpp, 'w') as f:
    template = f'''
#include "pxr/pxr.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/base/tf/scriptModuleLoader.h"
#include "pxr/base/tf/token.h"

#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfScriptModuleLoader) {{
    // List of direct dependencies for this library.
    const std::vector<TfToken> reqs = {{
        {reqs_tftokens}
    }};
    TfScriptModuleLoader::GetInstance().
        RegisterLibrary(TfToken("{libname}"), TfToken("{py_module}"), reqs);
}}

PXR_NAMESPACE_CLOSE_SCOPE

'''
    f.write(template)
