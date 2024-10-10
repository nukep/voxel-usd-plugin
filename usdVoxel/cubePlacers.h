#ifndef __CUBE_PLACERS_H__
#define __CUBE_PLACERS_H__

#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3h.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/vt/types.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/sdf/primSpec.h"
#include "pxr/usd/sdf/attributeSpec.h"
#include "pxr/usd/sdf/relationshipSpec.h"
#include "pxr/usd/sdf/types.h"

#include <stdio.h>

class SdfMeshCubePlacer {
    pxr::VtVec3fArray points;
    pxr::VtIntArray faceVertexIndices;
    pxr::VtIntArray faceVertexCounts;
    pxr::VtVec3fArray displayColor;
    pxr::VtVec3fArray normals;

    int currentLevel;
    float xcentroid, ycentroid, zcentroid;

public:
    SdfMeshCubePlacer()
        : currentLevel(0),
          xcentroid(0), ycentroid(0), zcentroid(0)
    {

    }
    void setLevel(int level) {
        this->currentLevel = level;
    }
    void setCentroid(float x, float y, float z) {
        this->xcentroid = x;
        this->ycentroid = y;
        this->zcentroid = z;
    }
    void place(int32_t ix, int32_t iy, int32_t iz, float r, float g, float b, uint8_t sides) {
        using namespace pxr;

        if (currentLevel != 0) {
            return;
        }
        float x = ix - this->xcentroid;
        float y = iy - this->ycentroid;
        float z = iz - this->zcentroid;

        float x1 = x - 0.5;
        float x2 = x + 0.5;
        float y1 = y - 0.5;
        float y2 = y + 0.5;
        float z1 = z - 0.5;
        float z2 = z + 0.5;

        // Cube vertices:
        //     y
        //     ^
        //     |
        //     2      3
        //   6      7
        //  
        //     0      1  --> x
        //   4      5
        //  /
        // v
        // z
        //
        // i.e. 0 is the negativemost point, 7 is the positivemost point

        // we dedupe points by creating a unique 64-bit number from the integer xyz coordinates.
        // this gives us an allowance of coordinates up to 2^24 (16,777,216 voxels on each side)
        // afterwards, we create a 1-1 correspondance from the set to a new consecutive set starting at 0.
        // 

        GfVec3f verts[8] = {
            GfVec3f(x1,y1,z1),
            GfVec3f(x2,y1,z1),
            GfVec3f(x1,y2,z1),
            GfVec3f(x2,y2,z1),
            GfVec3f(x1,y1,z2),
            GfVec3f(x2,y1,z2),
            GfVec3f(x1,y2,z2),
            GfVec3f(x2,y2,z2),
        };

        size_t offset = points.size();

        for (auto vert: verts) {
            points.push_back(vert);
        }

        static const int indices[][4] = {
            {0,4,6,2},  // left
            {5,1,3,7},  // right
            {1,0,2,3},  // back
            {4,5,7,6},  // front
            {6,7,3,2},  // top
            {0,1,5,4}   // bottom
        };
        static const GfVec3f sideNormals[] = {
            GfVec3f(-1,0,0),
            GfVec3f(1,0,0),
            GfVec3f(0,0,-1),
            GfVec3f(0,0,1),
            GfVec3f(0,1,0),
            GfVec3f(0,-1,0),
        };

        for (int j = 0; j < 6; j++) {
            int sidemask = 1<<j;
            if ((sides & sidemask) == 0) {
                continue;
            }
            for (int i = 0; i < 4; i++) {
                faceVertexIndices.push_back(offset + indices[j][i]);
            }
            faceVertexCounts.push_back(4);
            displayColor.push_back(GfVec3f(r, g, b));
            normals.push_back(sideNormals[j]);
        }
    }
    pxr::SdfPrimSpecHandle writePrim(pxr::SdfLayerHandle layer, pxr::SdfPath path) {
        using namespace pxr;

        auto primspec = SdfCreatePrimInLayer(layer, path);
        primspec->SetSpecifier(SdfSpecifierDef);
        primspec->SetTypeName("Mesh");

        auto subd_attr = SdfAttributeSpec::New(primspec, "subdivisionScheme", SdfValueTypeNames->Token);
        subd_attr->SetDefaultValue(VtValue(TfToken("none")));
        auto normals_attr = SdfAttributeSpec::New(primspec, "normals", SdfValueTypeNames->Normal3fArray);
        normals_attr->SetDefaultValue(VtValue(normals));
        normals_attr->SetField(TfToken("interpolation"), TfToken("uniform"));

        auto fvi_attr = SdfAttributeSpec::New(primspec, "faceVertexIndices", SdfValueTypeNames->IntArray);
        auto fvc_attr = SdfAttributeSpec::New(primspec, "faceVertexCounts", SdfValueTypeNames->IntArray);
        auto points_attr = SdfAttributeSpec::New(primspec, "points", SdfValueTypeNames->Vector3fArray);
        auto displayColor_attr = SdfAttributeSpec::New(primspec, "primvars:displayColor", SdfValueTypeNames->Color3fArray);
        displayColor_attr->SetField(TfToken("interpolation"), TfToken("uniform"));

        fvi_attr->SetDefaultValue(VtValue(faceVertexIndices));
        fvc_attr->SetDefaultValue(VtValue(faceVertexCounts));
        points_attr->SetDefaultValue(VtValue(points));
        displayColor_attr->SetDefaultValue(VtValue(displayColor));

        return primspec;
    }
};

class SdfPointInstanceCubePlacer {
    // for the point instancer
    pxr::VtVec3fArray positions;
    pxr::VtIntArray protoIndices;
    pxr::VtVec3fArray displayColor;

    int currentLevel;
    float xcentroid, ycentroid, zcentroid;

public:
    SdfPointInstanceCubePlacer()
        : currentLevel(0),
          xcentroid(0), ycentroid(0), zcentroid(0)
    {

    }
    void setLevel(int level) {
        this->currentLevel = level;
    }
    void setCentroid(float x, float y, float z) {
        this->xcentroid = x;
        this->ycentroid = y;
        this->zcentroid = z;
    }
    void place(int32_t ix, int32_t iy, int32_t iz, float r, float g, float b, uint8_t _sides) {
        using namespace pxr;

        if (currentLevel != 0) {
            return;
        }
        float x = ix - this->xcentroid;
        float y = iy - this->ycentroid;
        float z = iz - this->zcentroid;

        positions.push_back(GfVec3f(x, y, z));
        displayColor.push_back(GfVec3f(r, g, b));
        protoIndices.push_back(0);

    }
    pxr::SdfPrimSpecHandle writePrim(pxr::SdfLayerHandle layer, pxr::SdfPath path) {
        using namespace pxr;

        auto primspec = SdfCreatePrimInLayer(layer, path);
        primspec->SetSpecifier(SdfSpecifierDef);
        primspec->SetTypeName("PointInstancer");

        auto protoprimspec = SdfCreatePrimInLayer(layer, path.AppendChild(TfToken("Prototypes")).AppendChild(TfToken("cube")));
        protoprimspec->SetSpecifier(SdfSpecifierDef);
        protoprimspec->SetTypeName("Cube");
        SdfAttributeSpec::New(protoprimspec, "size", SdfValueTypeNames->Double)->SetDefaultValue(VtValue(1.0));

        auto positions_attr = SdfAttributeSpec::New(primspec, "positions", SdfValueTypeNames->Point3fArray);
        auto protoIndices_attr = SdfAttributeSpec::New(primspec, "protoIndices", SdfValueTypeNames->IntArray);
        auto prototypes_attr = SdfRelationshipSpec::New(primspec, "prototypes");
        auto displayColor_attr = SdfAttributeSpec::New(primspec, "primvars:displayColor", SdfValueTypeNames->Color3fArray);
        displayColor_attr->SetField(TfToken("interpolation"), TfToken("varying"));

        prototypes_attr->GetTargetPathList().Append(protoprimspec->GetPath());

        positions_attr->SetDefaultValue(VtValue(positions));
        protoIndices_attr->SetDefaultValue(VtValue(protoIndices));
        displayColor_attr->SetDefaultValue(VtValue(displayColor));

        return primspec;
    }
};

#endif
