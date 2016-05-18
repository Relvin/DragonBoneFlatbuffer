//
//  xmlToBinary.hpp
//  Dragonbones
//
//  Created by Relvin on 15/12/2.
//  Copyright © 2015年 Relvin. All rights reserved.
//

#ifndef xmlToBinary_hpp
#define xmlToBinary_hpp

#include "objects/DragonBonesData.h"
#include "dragonbones_generated.h"
#include "objects/SlotFrame.h"
#include "objects/FFDFrame.h"
#include "objects/MeshData.h"

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
    
    bool convertIKData(flatbuffers::FlatBufferBuilder &builder,IKData* ikData,std::vector<flatbuffers::Offset<IKDataOption>> &ikDataOptionList);
    
    bool convertSlotData(flatbuffers::FlatBufferBuilder &builder,SlotData* slotData,std::vector<flatbuffers::Offset<SlotDataOption>> &slotDataOptionList);
    
    bool convertMeshData(flatbuffers::FlatBufferBuilder &builder,MeshData* displayData,std::vector<flatbuffers::Offset<MeshDataOption>> &MeshDataOptionList);
    
    bool convertMainDisplayData(flatbuffers::FlatBufferBuilder &builder,DisplayData* displayData,std::vector<flatbuffers::Offset<DisplayDataOption>> &displayOptionList);
    
    bool convertAnimationData(flatbuffers::FlatBufferBuilder &builder, AnimationData* animationData,std::vector<flatbuffers::Offset<AnimationDataOption>> &animationDataOptionList);
    
    bool convertTransformTimelineData(flatbuffers::FlatBufferBuilder &builder,TransformTimeline* timeline,std::vector<flatbuffers::Offset<TransformTimelineOption>> &transformTimelineOptionList);
    
    bool convertSlotTimelineData(flatbuffers::FlatBufferBuilder &builder,SlotTimeline* timeline,std::vector<flatbuffers::Offset<SlotTimelineOption>> &slotTimelineOptionList);
    
    bool convertFFDTimelineData(flatbuffers::FlatBufferBuilder &builder,FFDTimeline* timeline,std::vector<flatbuffers::Offset<FFDTimelineOption>> &ffdTimelineOptionList);
    
    flatbuffers::Offset<DisplayDataOption> convertDisplayData(flatbuffers::FlatBufferBuilder &builder,DisplayData *displayData);

    flatbuffers::Offset<TransformFrameOption> convertTransformFrameData(flatbuffers::FlatBufferBuilder &builder,TransformFrame *transformFrame);
    
    flatbuffers::Offset<FrameOption> convertFrameData(flatbuffers::FlatBufferBuilder &builder,Frame *frame);
    flatbuffers::Offset<SlotFrameOption> convertSlotFrameData(flatbuffers::FlatBufferBuilder &builder,SlotFrame *slotFrame);
    flatbuffers::Offset<FFDFrameOption> convertFFDFrameData(flatbuffers::FlatBufferBuilder &builder,FFDFrame *ffdFrame);

    flatbuffers::Offset<ColorTransformOption> convertColorTransformData(flatbuffers::FlatBufferBuilder &builder,ColorTransform *colorTransform);
    
    bool writeToPath(const char* data,size_t size);
private:
    flatbuffers::FlatBufferBuilder builder;
    std::string m_filePath;
    std::string m_outPath;
	bool notCreate;
    
    //for test
    bool readXMLB;
};


#endif /* xmlToBinary_hpp */
