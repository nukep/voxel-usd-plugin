#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/tf/refPtr.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/tf/weakPtr.h"
#include "pxr/base/vt/types.h"
#include "pxr/usd/pcp/dynamicFileFormatDependencyData.h"
#include "pxr/usd/pcp/dynamicFileFormatInterface.h"
#include "pxr/usd/sdf/reference.h"
#include "pxr/usd/sdf/abstractData.h"
#include "pxr/usd/sdf/fileFormat.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/usd/sdf/listOp.h"
#include "pxr/usd/sdf/primSpec.h"
#include "pxr/usd/sdf/attributeSpec.h"
#include "pxr/usd/sdf/relationshipSpec.h"
#include "pxr/usd/sdf/types.h"
#include "pxr/usd/ar/asset.h"
#include "pxr/usd/ar/resolver.h"

#include "ogt_vox.h"

#include "cubePlacers.h"

#include <map>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

using namespace pxr;

template <class T> 
static bool MagicavoxelRead_Model(const ogt_vox_model *model, const ogt_vox_palette *palette, T &cubePlacer) {
    for (uint32_t z = 0; z < model->size_z; z++) {
    for (uint32_t y = 0; y < model->size_y; y++) {
    for (uint32_t x = 0; x < model->size_x; x++) {
        size_t voxel_index = x + (y * model->size_x) + (z * model->size_x * model->size_y);
        uint8_t color_index = model->voxel_data[voxel_index];
        if (color_index != 0) {
            // solid voxel
            ogt_vox_rgba color = palette->color[color_index];
            float r = (float)color.r / 255.0f;
            float g = (float)color.g / 255.0f;
            float b = (float)color.b / 255.0f;
            float a = (float)color.a / 255.0f;
            cubePlacer.place(x,y,z, r,g,b, 0xff);
        }
    }
    }
    }
    return true;
}

static GfMatrix4d transformToGfMatrix4d(const ogt_vox_transform *m) {
    // basis vectors are rows
    return GfMatrix4d(
        m->m00, m->m01, m->m02, m->m03,
        m->m10, m->m11, m->m12, m->m13,
        m->m20, m->m21, m->m22, m->m23,
        m->m30, m->m31, m->m32, m->m33
    );
}

static void createVisibilityForPrim(SdfPrimSpecHandle prim, bool hidden) {
    if (hidden) {
        SdfAttributeSpecHandle attr;
        attr = SdfAttributeSpec::New(prim, "visibility", SdfValueTypeNames->Token);
        attr->SetDefaultValue(VtValue(TfToken("invisible")));
    }
}

static void createTransformForPrim(SdfPrimSpecHandle prim, const ogt_vox_transform *m) {
    auto transform = transformToGfMatrix4d(m);
    SdfAttributeSpecHandle attr;
    attr = SdfAttributeSpec::New(prim, "xformOpOrder", SdfValueTypeNames->TokenArray);
    attr->SetDefaultValue(VtValue(VtTokenArray({ TfToken("xformOp:transform") })));
    attr = SdfAttributeSpec::New(prim, "xformOp:transform", SdfValueTypeNames->Matrix4d);
    attr->SetDefaultValue(VtValue(transform));
}

static SdfPrimSpecHandle createGroup(const ogt_vox_scene *scene, SdfLayerHandle lyr, std::map<uint32_t, SdfPrimSpecHandle> &groupPrims, size_t group_id) {
    if (group_id == k_invalid_group_index) {
        return lyr->GetPseudoRoot();
    }
    auto it = groupPrims.find(group_id);
    if (it != groupPrims.end()) {
        return it->second;
    }

    const ogt_vox_group *group = &scene->groups[group_id];
    auto parentPrim = createGroup(scene, lyr, groupPrims, group->parent_group_index);
    auto parentPath = parentPrim->GetPath();
    char pathc[64];
    snprintf(pathc, sizeof(pathc), "group%lu", group_id);
    auto path = parentPath.AppendChild(TfToken(pathc));
    auto prim = SdfCreatePrimInLayer(lyr, path);
    prim->SetSpecifier(SdfSpecifierDef);
    prim->SetTypeName("Xform");
    if (group->name) {
        prim->SetField(TfToken("displayName"), std::string(group->name));
    }

    createTransformForPrim(prim, &group->transform);
    createVisibilityForPrim(prim, group->hidden);

    groupPrims.emplace(group_id, prim);
    return prim;
}

static SdfPrimSpecHandle createModel(const ogt_vox_model *model, const ogt_vox_palette *palette, SdfLayerHandle lyr, SdfPath path) {
    SdfMeshCubePlacer cubePlacer;
    MagicavoxelRead_Model(model, palette, cubePlacer);
    return cubePlacer.writePrim(lyr, path);
}

static bool MagicavoxelRead_impl(const ogt_vox_scene *scene, SdfLayerHandle lyr) {
    // scene->palette
    // cameras, groups, instances have layer indexes
    // a group has a parent, a group has many children, a group has an xform
    // an instance has a group as a parent, refers to a model (first frame) and animation.
    // an animation is a list of keyframes to model indexes.
    std::map<uint32_t, SdfPrimSpecHandle> groupPrims;

    auto modelsPrim = SdfCreatePrimInLayer(lyr, SdfPath("/models"));
    modelsPrim->SetSpecifier(SdfSpecifierClass);
    modelsPrim->SetTypeName("Scope");

    for (uint32_t i = 0; i < scene->num_models; i++) {
        const ogt_vox_model *model = scene->models[i];
        char pathc[64];
        snprintf(pathc, sizeof(pathc), "/models/m%u", i);
        SdfPath path(pathc);
        createModel(model, &scene->palette, lyr, path);
    }

    for (uint32_t i = 0; i < scene->num_instances; i++) {
        const ogt_vox_instance *inst = &scene->instances[i];

        auto parentPrim = createGroup(scene, lyr, groupPrims, inst->group_index);
        auto parentPath = parentPrim->GetPath();
        char pathc[64];
        snprintf(pathc, sizeof(pathc), "inst%u", i);
        auto path = parentPath.AppendChild(TfToken(pathc));
        auto prim = SdfCreatePrimInLayer(lyr, path);
        prim->SetSpecifier(SdfSpecifierDef);
        prim->SetTypeName("Xform");
        if (inst->name) {
            prim->SetField(TfToken("displayName"), std::string(inst->name));
        }

        createTransformForPrim(prim, &inst->transform);
        createVisibilityForPrim(prim, inst->hidden);

        snprintf(pathc, sizeof(pathc), "/models/m%u", inst->model_index);
        SdfPath modelPath(pathc);
        auto modelPrim = SdfCreatePrimInLayer(lyr, path.AppendChild(TfToken("model")));
        modelPrim->GetReferenceList().Append(SdfReference("", modelPath));
    }

    for (uint32_t i = 0; i < scene->num_groups; i++) {
        createGroup(scene, lyr, groupPrims, i);
    }
    return true;
}

bool SdfMagicaVoxelRead(SdfLayerHandle layer, const unsigned char *contents, size_t contents_size) {
    const ogt_vox_scene *scene = ogt_vox_read_scene_with_flags(contents, contents_size, k_read_scene_flags_groups | k_read_scene_flags_keyframes | k_read_scene_flags_keep_empty_models_instances | k_read_scene_flags_keep_duplicate_models);
    bool result = MagicavoxelRead_impl(scene, layer);
    ogt_vox_destroy_scene(scene);
    return result;
}