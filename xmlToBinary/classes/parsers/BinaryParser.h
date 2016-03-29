//
//  BinaryParser.hpp
//  xmlToBinary
//
//  Created by Relvin on 16/3/29.
//  Copyright © 2016年 Relvin. All rights reserved.
//

#ifndef BinaryParser_hpp
#define BinaryParser_hpp

#include "DragonBones.h"
NAME_SPACE_DRAGON_BONES_BEGIN

class DragonBonesData;
class ArmatureData;
class AnimationData;
class BoneData;
class TransformFrame;
class TransformTimeline;
class Transform;
class SkinData;
class SlotData;
class DisplayData;
class SlotTimeline;
class SlotFrame;
class Frame;
class Timeline;
class Point;
class ColorTransform;

class BinaryParser
{
public:
    BinaryParser();
    virtual ~BinaryParser();
    
    virtual DragonBonesData* parseDragonBonesData(const void *rawDragonBonesData,float scale = 1.f) const;
    
protected:
    
    
    
private:
    
    
    
};


NAME_SPACE_DRAGON_BONES_END

#endif /* BinaryParser_hpp */
