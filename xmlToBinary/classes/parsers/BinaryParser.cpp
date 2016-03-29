//
//  BinaryParser.cpp
//  xmlToBinary
//
//  Created by Relvin on 16/3/29.
//  Copyright © 2016年 Relvin. All rights reserved.
//

#include "BinaryParser.h"

#include "ConstValues.h"
// geoms
#include "geoms/ColorTransform.h"
#include "geoms/Matrix.h"
#include "geoms/Point.h"
#include "geoms/Rectangle.h"
#include "geoms/Transform.h"
// objects
#include "objects/Frame.h"
#include "objects/TransformFrame.h"
#include "objects/Timeline.h"
#include "objects/TransformTimeline.h"
#include "objects/AnimationData.h"
#include "objects/DisplayData.h"
#include "objects/SlotData.h"
#include "objects/SkinData.h"
#include "objects/BoneData.h"
#include "objects/ArmatureData.h"
#include "objects/DragonBonesData.h"
#include "objects/SlotFrame.h"


NAME_SPACE_DRAGON_BONES_BEGIN

BinaryParser::BinaryParser()
{
    
}

BinaryParser::~BinaryParser()
{
    
}

DragonBonesData* BinaryParser::parseDragonBonesData(const void *rawDragonBonesData,float scale) const
{
    return NULL;
}







NAME_SPACE_DRAGON_BONES_END