// 2024 - Danny Spencer
// Used Ken Silverman's documentation of KVX - slab6.txt in slab6.zip:
// http://advsys.net/ken/download.htm#slab6

// Our approach is to load the .kvx file and convert it to a Mesh prim.
// We'll use the displayColor primvar for colors.
// A KVX file (typically) has 5 levels of detail. This can be selected as a variant.

#include "cubePlacers.h"

#include "pxr/base/gf/vec3f.h"
#include "pxr/base/tf/refPtr.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/tf/weakPtr.h"
#include "pxr/base/vt/types.h"
#include "pxr/usd/pcp/dynamicFileFormatDependencyData.h"
#include "pxr/usd/pcp/dynamicFileFormatInterface.h"
#include "pxr/usd/sdf/abstractData.h"
#include "pxr/usd/sdf/fileFormat.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/usd/sdf/primSpec.h"
#include "pxr/usd/sdf/attributeSpec.h"
#include "pxr/usd/sdf/relationshipSpec.h"
#include "pxr/usd/sdf/types.h"
#include "pxr/usd/ar/asset.h"
#include "pxr/usd/ar/resolver.h"

#include "kvx.h"

#include <stdio.h>
#include <iostream>

// TODO: add to plugin.json
// "UsdVoxelVoxFileFormat": {
//     "bases": [
//         "SdfFileFormat"
//     ],
//     "displayName": "Magicavoxel voxel format",
//     "extensions": [
//         "vox"
//     ],
//     "formatId": "vox",
//     "primary": true,
//     "supportsEditing": false,
//     "supportsWriting": false,
//     "target": "usd"
// },

using namespace pxr;

#define USD_VOXEL_KVX_TOKENS    \
    ((Id, "usdVoxelKvx"))       \
    ((Version, "1.0"))          \
    ((Target, "usd"))           \
    ((Extension, "kvx"))

TF_DECLARE_PUBLIC_TOKENS(UsdVoxelKvxTokens, 
                         USD_VOXEL_KVX_TOKENS);

TF_DEFINE_PUBLIC_TOKENS(
    UsdVoxelKvxTokens, 
    USD_VOXEL_KVX_TOKENS);

class UsdVoxelKvxFileFormat : public SdfFileFormat {
public:
    UsdVoxelKvxFileFormat()
    : SdfFileFormat(
        UsdVoxelKvxTokens->Id,
        UsdVoxelKvxTokens->Version,
        UsdVoxelKvxTokens->Target,
        UsdVoxelKvxTokens->Extension)
    {
    }
    ~UsdVoxelKvxFileFormat() {
    }
    bool CanRead(const std::string &filePath) const override {
        return true;
    }
    bool Read(SdfLayer *layer, const std::string &resolvedPath, bool metadataOnly) const override {
        if (!TF_VERIFY(layer)) {
            return false;
        }

        const FileFormatArguments &args = layer->GetFileFormatArguments();
        auto data = InitData(args);
        _SetLayerData(layer, data);

        auto asset = ArGetResolver().OpenAsset(ArResolvedPath(resolvedPath));
        if (!asset) {
            return false;
        }

        // read the entire asset in memory
        auto buf = asset->GetBuffer();
        if (!buf) {
            return false;
        }

        // Create specs directly on the SdfData object
        // Interface to it through SdfLayer

        bool zUp = false;
        if (zUp) {
            layer->GetPseudoRoot()->SetField(TfToken("upAxis"), TfToken("Z"));
        }

        SdfLayerHandle lyr(layer);

        SdfPointInstanceCubePlacer pointsCubePlacer;
        SdfMeshCubePlacer meshCubePlacer;

        const char *contents = buf.get();
        size_t contents_size = asset->GetSize();
        bool success;
        success = KvxRead((const unsigned char*)contents, contents_size, meshCubePlacer);
        if (success) {
            meshCubePlacer.writePrim(lyr, SdfPath("/mesh"));
        }

        layer->SetDefaultPrim(TfToken("mesh"));

        layer->SetPermissionToSave(false);
        layer->SetPermissionToEdit(false);

        return true;
    }
};

TF_DECLARE_WEAK_AND_REF_PTRS(UsdVoxelKvxFileFormat);

TF_REGISTRY_FUNCTION(TfType)
{
    SDF_DEFINE_FILE_FORMAT(UsdVoxelKvxFileFormat, SdfFileFormat);
}