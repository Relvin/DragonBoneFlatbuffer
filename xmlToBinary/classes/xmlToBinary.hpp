//
//  xmlToBinary.hpp
//  Dragonbones
//
//  Created by Relvin on 15/12/2.
//  Copyright © 2015年 Relvin. All rights reserved.
//

#ifndef xmlToBinary_hpp
#define xmlToBinary_hpp

#include <stdio.h>
#include "objects/DragonBonesData.h"
#include "dragonbones_generated.h"

using namespace dragonBones;
class xmlToBinary
{
public:
    static xmlToBinary* getInstance();
    
    xmlToBinary();
    virtual ~xmlToBinary();
    
    virtual bool init();
    
    bool readFileAndConverToBinary(std::string &path,std::string& outPath);
    bool convertDragonboneToBinary (dragonBones::DragonBonesData *bonesData);
    
    bool convertBoneData(flatbuffers::FlatBufferBuilder &builder,BoneData* boneData,std::vector<flatbuffers::Offset<BoneDataOption>> &boneDataOptionList);
    
    bool convertSkinData(flatbuffers::FlatBufferBuilder &builder,dragonBones::SkinData* skinData,std::vector<flatbuffers::Offset<SkinDataOption>> &skinDataOptionList);
    
    bool convertSlotData(flatbuffers::FlatBufferBuilder &builder,SlotData* slotData,std::vector<flatbuffers::Offset<SlotDataOption>> &slotDataOptionList);
    
    bool convertDisplayData(flatbuffers::FlatBufferBuilder &builder,DisplayData* displayData,std::vector<flatbuffers::Offset<DisplayOption>> &displayOptionList);
    
    bool convertAnimationData(flatbuffers::FlatBufferBuilder &builder, AnimationData* animationData,std::vector<flatbuffers::Offset<AnimationDataOption>> &animationDataOptionList);
    
    bool convertTransformTimelineData(flatbuffers::FlatBufferBuilder &builder,TransformTimeline* timeline,std::vector<flatbuffers::Offset<TransformTimelineOption>> &transformTimelineOptionList);
    
    bool convertSlotTimelineData(flatbuffers::FlatBufferBuilder &builder,SlotTimeline* timeline,std::vector<flatbuffers::Offset<SlotTimelineOption>> &slotTimelineOptionList);
    

    bool convertFrameData(flatbuffers::FlatBufferBuilder &builder,Frame *frame,std::vector<flatbuffers::Offset<FrameOption>> &frameOptionList);
    
    bool writeToPath(const char* data,size_t size);
private:
    flatbuffers::FlatBufferBuilder builder;
    std::string m_filePath;
    std::string m_outPath;
	bool notCreate;
    
};


#endif /* xmlToBinary_hpp */
