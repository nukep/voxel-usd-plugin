#ifndef __SDFMAGICAVOXEL_H__
#define __SDFMAGICAVOXEL_H__

#include "pxr/usd/sdf/primSpec.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/sdf/types.h"
#include <stdint.h>

bool SdfMagicaVoxelRead(pxr::SdfLayerHandle layer, const unsigned char *contents, size_t contents_size);

#endif
