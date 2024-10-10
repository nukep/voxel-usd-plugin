// 2024 - Danny Spencer

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

#include "SdfMagicaVoxel.h"

#include <stdio.h>
#include <iostream>

using namespace pxr;

#define USD_VOXEL_VOX_TOKENS    \
    ((Id, "usdVoxelVox"))       \
    ((Version, "1.0"))          \
    ((Target, "usd"))           \
    ((Extension, "vox"))

TF_DECLARE_PUBLIC_TOKENS(UsdVoxelVoxTokens, 
                         USD_VOXEL_VOX_TOKENS);

TF_DEFINE_PUBLIC_TOKENS(
    UsdVoxelVoxTokens, 
    USD_VOXEL_VOX_TOKENS);

class UsdVoxelVoxFileFormat : public SdfFileFormat {
public:
    UsdVoxelVoxFileFormat()
    : SdfFileFormat(
        UsdVoxelVoxTokens->Id,
        UsdVoxelVoxTokens->Version,
        UsdVoxelVoxTokens->Target,
        UsdVoxelVoxTokens->Extension)
    {
    }
    ~UsdVoxelVoxFileFormat() {
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

        const char *contents = buf.get();
        size_t contents_size = asset->GetSize();
        SdfMagicaVoxelRead(lyr, (unsigned char*)contents, contents_size);
        
        layer->GetPseudoRoot()->SetField(TfToken("upAxis"), TfToken("Z"));

        layer->SetPermissionToSave(false);
        layer->SetPermissionToEdit(false);

        return true;
    }
};

TF_DECLARE_WEAK_AND_REF_PTRS(UsdVoxelVoxFileFormat);

TF_REGISTRY_FUNCTION(TfType)
{
    SDF_DEFINE_FILE_FORMAT(UsdVoxelVoxFileFormat, SdfFileFormat);
}