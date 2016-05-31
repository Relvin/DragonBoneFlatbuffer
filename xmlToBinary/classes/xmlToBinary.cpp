//
//  xmlToBinary.cpp
//  Dragonbones
//
//  Created by Relvin on 15/12/2.
//  Copyright © 2015年 Relvin. All rights reserved.
//

#include "xmlToBinary.hpp"
#include "parsers/XMLDataParser.h"
#include "Platform.h"
#include "parsers/BinaryParser.h"

static xmlToBinary* s_instance = nullptr;

xmlToBinary* xmlToBinary::getInstance()
{
    if (!s_instance)
    {
        s_instance = new (std::nothrow) xmlToBinary();
        s_instance->init();
    }
    
    return s_instance;
}

xmlToBinary::xmlToBinary()
: m_filePath ("")
, m_outPath ("")
, notCreate(false)
, readXMLB (false)
{
    
}

xmlToBinary::~xmlToBinary()
{
    
}

bool xmlToBinary::init()
{
    
    
    return true;
}



bool xmlToBinary::readFileAndConverToBinary(std::string &path,std::string& outPath)
{
    this->m_filePath = path;
    this->m_outPath = outPath;
    size_t size = 0;
    size_t readsize = 0;
    std::string filename = path;
    size_t pos = filename.find_last_of('.');
    std::string suffix = filename.substr(pos + 1, path.length());
    
    if (suffix != "xml" && suffix != "xmlb")
    {
        printf("%s is not xml file !\n",path.c_str());
        return false;
    }
    
    unsigned char* buffer = nullptr;
    do
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        FILE *fp;
        fopen_s(&fp,path.c_str(), "rb");
#else
        FILE *fp = fopen(path.c_str(), "rb");
#endif
        if (!fp)
        {
            printf("%s open error !\n",path.c_str());
            break;
        }
        fseek(fp,0,SEEK_END);
        size = ftell(fp);
        fseek(fp,0,SEEK_SET);
        
        
        buffer = (unsigned char*)malloc(sizeof(unsigned char) * size);
        
        readsize = fread(buffer, sizeof(unsigned char), size, fp);
        fclose(fp);
        
    } while (0);
    
    if (nullptr == buffer || 0 == readsize)
    {
        printf("Get data from file %s failed!\n", path.c_str());
    }
    else
    {
        printf("%s\t", m_filePath.c_str());
        if (suffix == "xml")
        {
            readXMLB = false;
            dragonBones::XMLDataParser parser;
            dragonBones::XMLDocument doc;
            doc.Parse(reinterpret_cast<char*>(buffer), readsize);
            auto dragonBonesData = parser.parseDragonBonesData(doc.RootElement());
            if (dragonBonesData)
            {
                this->convertDragonboneToBinary(dragonBonesData);
            }
            
        }
        else
        {
            readXMLB = true;
            dragonBones::BinaryParser parser;
            auto dragonBonesData = parser.parseDragonBonesData(buffer);
            this->convertDragonboneToBinary(dragonBonesData);
        }
        
        
        free(buffer);
        return true;
    }
    
    return false;
    
}

bool xmlToBinary::convertDragonboneToBinary(dragonBones::DragonBonesData *dragonbonesData)
{
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<ArmatureOption>> armatureOptionList;
    for (int armIdx = 0;armIdx < dragonbonesData->armatureDataList.size();armIdx++)
    {
        ArmatureData* armData = dragonbonesData->armatureDataList.at(armIdx);
        
        std::vector<flatbuffers::Offset<BoneDataOption>> boneDataOptionList;
        for (int boneIdx = 0;boneIdx < armData->boneDataList.size();boneIdx++)
        {
            convertBoneData(builder,armData->boneDataList.at(boneIdx), boneDataOptionList);
        }

        std::vector<flatbuffers::Offset<SkinDataOption>> skinDataOptionList;
        for (int skinIdx = 0; skinIdx < armData->skinDataList.size(); skinIdx++)
        {
            convertSkinData(builder,armData->skinDataList.at(skinIdx), skinDataOptionList);
        }
        
        std::vector<flatbuffers::Offset<SlotDataOption>> slotDataOptionList;
        for (int slotIdx = 0; slotIdx < armData->slotDataList.size(); slotIdx++)
        {
            convertSlotData(builder,armData->slotDataList.at(slotIdx), slotDataOptionList);
        }
        std::vector<flatbuffers::Offset<IKDataOption>> ikDataOptionList;
        for (auto ikData : armData->ikDataList)
        {
            convertIKData(builder,ikData, ikDataOptionList);
        }
        
        std::vector<flatbuffers::Offset<AnimationDataOption>> animationDataOptionList;
        for (int aniIdx = 0; aniIdx < armData->animationDataList.size(); aniIdx++)
        {
            convertAnimationData(builder,armData->animationDataList.at(aniIdx), animationDataOptionList);
        }

        auto armatureOpt = CreateArmatureOption(builder,
                                                armData->name.length() == 0 ? 0 : builder.CreateString(armData->name.c_str()),
                                                boneDataOptionList.size() == 0 ? 0 : builder.CreateVector(boneDataOptionList),
                                                skinDataOptionList.size() == 0 ? 0 : builder.CreateVector(skinDataOptionList),
                                                slotDataOptionList.size() == 0 ? 0 : builder.CreateVector(slotDataOptionList),
                                                ikDataOptionList.size() == 0 ? 0 : builder.CreateVector(ikDataOptionList),
                                                animationDataOptionList.size() == 0 ? 0 : builder.CreateVector(animationDataOptionList));
        
        armatureOptionList.push_back(armatureOpt);

    }
    
    
    auto dragonBoneBinaryData =
    CreateDragonBonesParseBinary(builder,
                      dragonbonesData->name.length() == 0 ? 0 : builder.CreateString(dragonbonesData->name.c_str()),
                      dragonbonesData->autoSearch,
                      dragonbonesData->isGlobalData,
                      dragonbonesData->version.length() == 0 ? 0 : builder.CreateString(dragonbonesData->version.c_str()),
                      armatureOptionList.size() == 0 ? 0 : builder.CreateVector(armatureOptionList));
    FinishDragonBonesParseBinaryBuffer(builder, dragonBoneBinaryData);
    
    this->writeToPath((char *)builder.GetBufferPointer(), builder.GetSize());
    
    
    return true;
}


bool xmlToBinary::convertBoneData(flatbuffers::FlatBufferBuilder &builder,BoneData* boneData,std::vector<flatbuffers::Offset<BoneDataOption>> &boneDataOptionList)
{
    
    bool inheritScale = boneData->inheritScale;
    bool inheritRotation = boneData->inheritRotation;
    float length = boneData->length;
    
    std::string name = boneData->name;
    std::string parent = boneData->parent;
    Transform global = boneData->global;
    TransformOption globalOption(global.x,global.y,global.skewX,global.skewY,global.scaleX,global.scaleY);
    Transform transform = boneData->transform;
    TransformOption transformOption(transform.x,transform.y,transform.skewX,transform.skewY,transform.scaleX,transform.scaleY);
    
    auto boneDataOption = CreateBoneDataOption(builder,
                                       name.length() == 0 ? 0 : builder.CreateString(name),
                                       parent.length() == 0 ? 0 : builder.CreateString(parent),
                                       length,
                                       inheritScale,
                                       inheritRotation,
                                       &globalOption,
                                       &transformOption);
    boneDataOptionList.push_back(boneDataOption);
    return true;
}

bool xmlToBinary::convertSkinData(flatbuffers::FlatBufferBuilder &builder,dragonBones::SkinData* skinData,std::vector<flatbuffers::Offset<SkinDataOption>> &skinDataOptionList)
{
    
    std::vector<flatbuffers::Offset<SlotDataOption>> slotDataOptionList;
    for (int slotIdx = 0; slotIdx < skinData->slotDataList.size(); slotIdx++)
    {
        convertSlotData(builder,skinData->slotDataList.at(slotIdx), slotDataOptionList);
    }
    
    skinDataOptionList.push_back(
                                 CreateSkinDataOption(builder,
                                                      skinData->name.length() == 0 ? 0 : builder.CreateString(skinData->name),
                                                      slotDataOptionList.size() == 0 ? 0 : builder.CreateVector(slotDataOptionList)));
    return true;
}

bool xmlToBinary::convertSlotData(flatbuffers::FlatBufferBuilder &builder,SlotData* slotData,std::vector<flatbuffers::Offset<SlotDataOption>> &slotDataOptionList)
{
    
    
    std::vector<flatbuffers::Offset<DisplayDataOption>> displayDataList;
    
    std::vector<flatbuffers::Offset<MeshDataOption>> meshDataList;
    for (int disIdx = 0; disIdx < slotData->displayDataList.size(); disIdx++)
    {
        DisplayData* displayData = slotData->displayDataList.at(disIdx);
        
        switch (displayData->type) {
            case DisplayType::DT_IMAGE:
                convertMainDisplayData(builder,displayData, displayDataList);
                break;
            case DisplayType::DT_MESH:
                convertMeshData(builder,dynamic_cast<MeshData*>( displayData), meshDataList);
                break;
            default:
                break;
        }
    }
    
    int displayIndex = slotData->displayIndex;
    float zOrder = slotData->zOrder;
    std::string name = slotData->name;
    std::string parent = slotData->parent;
    auto slotOption = CreateSlotDataOption(builder,
                                           displayIndex,
                                           zOrder,
                                           name.length() == 0 ? 0 : builder.CreateString(name),
                                           parent.length() == 0 ? 0 : builder.CreateString(parent),
                                           (unsigned char)slotData->blendMode,
                                           displayDataList.size() == 0 ? 0 : builder.CreateVector(displayDataList),
                                           meshDataList.size() == 0 ? 0 : builder.CreateVector(meshDataList));
    
    slotDataOptionList.push_back(slotOption);
    
    return true;
}

bool xmlToBinary::convertIKData(flatbuffers::FlatBufferBuilder &builder,IKData* ikData,std::vector<flatbuffers::Offset<IKDataOption>> &ikDataOptionList)
{
    std::string name = ikData->name;
    std::string target = ikData->target;
    std::string bone = ikData->bone;
    auto ikDataOption = CreateIKDataOption(builder,
                                           name.length() == 0 ? 0 : builder.CreateString(name),
                                           target.length() == 0 ? 0 : builder.CreateString(target),
                                           bone.length() == 0 ? 0 : builder.CreateString(bone),
                                           ikData->chain,
                                           ikData->weight,
                                           ikData->bendPositive);
    ikDataOptionList.push_back(ikDataOption);
    return true;
}

bool xmlToBinary::convertMeshData(flatbuffers::FlatBufferBuilder &builder,MeshData* meshData,std::vector<flatbuffers::Offset<MeshDataOption>> &MeshDataOptionList)
{
    float width = meshData->getWidth();
    float height = meshData->getHeight();
    
    std::vector<flatbuffers::Offset<Vec2Option>> vectices;
    for (Point vectice : meshData->_orgVectices)
    {
        auto vec2Option = CreateVec2Option(builder,vectice.x,vectice.y);
        vectices.push_back(vec2Option);
        
    }
    
    std::vector<flatbuffers::Offset<Vec2Option>> uvs;
    for (Point uv : meshData->_UVs)
    {
        auto vec2Option = CreateVec2Option(builder,uv.x,uv.y);
        uvs.push_back(vec2Option);
        
    }
    auto dispLayDataOption = convertDisplayData(builder, meshData);
    auto meshDataOption = CreateMeshDataOption(builder,
                                               width,
                                               height,
                                               meshData->getTriangles().size() > 0 ? builder.CreateVector(meshData->getTriangles()) : 0,
                                               vectices.size() > 0 ? builder.CreateVector(vectices) : 0,
                                               uvs.size() > 0 ? builder.CreateVector(uvs) : 0,
                                               dispLayDataOption
                                               );

    MeshDataOptionList.push_back(meshDataOption);
    return true;
}

bool xmlToBinary::convertMainDisplayData(flatbuffers::FlatBufferBuilder &builder,DisplayData* displayData,std::vector<flatbuffers::Offset<DisplayDataOption>> &displayOptionList)
{
    auto dispLayDataOption = convertDisplayData(builder, displayData);
    displayOptionList.push_back(dispLayDataOption);
    
    return true;
}

flatbuffers::Offset<DisplayDataOption> xmlToBinary::convertDisplayData(flatbuffers::FlatBufferBuilder &builder,DisplayData *displayData)
{
    std::string name = displayData->name;
    std::string slotName = displayData->slotName;
    uint8_t type = (uint8_t)displayData->type;
    Transform transform = displayData->transform;
    dragonBones::Point pivot = displayData->pivot;
    TransformOption transformOption(transform.x,transform.y,transform.skewX,transform.skewY,transform.scaleX,transform.scaleY);
    PointOption point(pivot.x,pivot.y);
    auto dispLayDataOption = CreateDisplayDataOption(builder,
                                                     name.length() == 0 ? 0 : builder.CreateString(name),
                                                     slotName.length() == 0 ? 0 : builder.CreateString(slotName),
                                                     type,
                                                     &transformOption,
                                                     &point);
    return dispLayDataOption;
}


bool xmlToBinary::convertAnimationData(flatbuffers::FlatBufferBuilder &builder, AnimationData* animationData,std::vector<flatbuffers::Offset<AnimationDataOption>> &animationDataOptionList)
{

    std::vector<flatbuffers::Offset<TransformTimelineOption>> transformTimelineOptionList;
    for (int tlIdx = 0; tlIdx < animationData->timelineList.size(); tlIdx++)
    {
        convertTransformTimelineData(builder,animationData->timelineList.at(tlIdx),transformTimelineOptionList);
    }

    bool autoTween = animationData->autoTween;
    int frameRate = animationData->frameRate;
    int playTimes = animationData->playTimes;
    float fadeTime = animationData->fadeTime;
    float tweenEasing = animationData->tweenEasing;
    std::string name = animationData->name;
    
    std::vector<flatbuffers::Offset<FrameOption>> frameOptionList;
    
    for (int flIdx = 0; flIdx < animationData->frameList.size(); flIdx++)
    {
        auto frameOption = convertFrameData(builder,animationData->frameList.at(flIdx));
        frameOptionList.push_back(frameOption);
    }
    
    auto timelinOption = CreateTimelineOption(builder,
                                              animationData->duration,
                                              animationData->scale,
                                              0,
                                              0,
                                              0,
                                              frameOptionList.size() == 0 ? 0 : builder.CreateVector(frameOptionList));
    
    std::vector<flatbuffers::Offset<flatbuffers::String>> hideTimelineOptionList;
    for (int hideIdx = 0; hideIdx < animationData->hideTimelineList.size(); hideIdx++) {
        std::string hideTimeline = animationData->hideTimelineList.at(hideIdx);
        if (hideTimeline.length() != 0)
        {
            hideTimelineOptionList.push_back(builder.CreateString(hideTimeline));
        }
        
    }
    
    std::vector<flatbuffers::Offset<SlotTimelineOption>> slotTimelineOptionList;
    for (int slotTlIdx = 0; slotTlIdx < animationData->slotTimelineList.size(); slotTlIdx++) {
        auto slotTimeline = animationData->slotTimelineList.at(slotTlIdx);
        convertSlotTimelineData(builder, slotTimeline, slotTimelineOptionList);
        
    }
    
    std::vector<flatbuffers::Offset<FFDTimelineOption>> ffdTimelineOptionList;
    for (auto ffdTimeline : animationData->ffdTimelineList) {
        convertFFDTimelineData(builder, ffdTimeline, ffdTimelineOptionList);
        
    }
    
    auto animationOption = CreateAnimationDataOption(builder,
                                                     name.length() == 0 ? 0 : builder.CreateString(name),
                                                     frameRate,
                                                     fadeTime,
                                                     playTimes,
                                                     tweenEasing,
                                                     autoTween,
                                                     animationData->lastFrameDuration,
                                                     timelinOption,
                                                     hideTimelineOptionList.size() == 0 ? 0 : builder.CreateVector(hideTimelineOptionList),
                                                     transformTimelineOptionList.size() == 0 ? 0 : builder.CreateVector(transformTimelineOptionList),
                                                     slotTimelineOptionList.size() == 0 ? 0 : builder.CreateVector(slotTimelineOptionList),
                                                     ffdTimelineOptionList.size() == 0 ? 0 : builder.CreateVector(ffdTimelineOptionList)
                                                 );
    
    
    animationDataOptionList.push_back(animationOption);

    return true;
}

bool xmlToBinary::convertTransformTimelineData(flatbuffers::FlatBufferBuilder &builder,TransformTimeline* timeline,std::vector<flatbuffers::Offset<TransformTimelineOption>> &transformTimelineOptionList)
{
    std::vector<flatbuffers::Offset<TransformFrameOption>> transformFrameOptionList;
    
    for (int flIdx = 0; flIdx < timeline->frameList.size(); flIdx++)
    {
        TransformFrame* transformFrame = dynamic_cast<TransformFrame *>(timeline->frameList.at(flIdx));
        if (transformFrame)
        {
            auto transformOption = convertTransformFrameData(builder,transformFrame);
            transformFrameOptionList.push_back(transformOption);
        }
        
    }
    
    bool transformed = timeline->transformed;
    float offset = timeline->offset;
    
    std::string name = timeline->name;
    Transform transform = timeline->originTransform;
    
    TransformOption orgTransformOption(transform.x,transform.y,transform.skewX,transform.skewY,transform.scaleX,transform.scaleY);
    
    dragonBones::Point originPivot = timeline->originPivot;
    
    PointOption pivotOption(originPivot.x,originPivot.y);
    
    
    auto timelinOption = CreateTimelineOption(builder,
                                              timeline->duration,
                                              timeline->scale,
                                              transformFrameOptionList.size() == 0 ? 0 : builder.CreateVector(transformFrameOptionList));
    auto transformTimelineOpt = CreateTransformTimelineOption(builder,
                                                              transformed,
                                                              offset,
                                                              name.length() == 0 ? 0 : builder.CreateString(name),
                                                              &orgTransformOption,
                                                              &pivotOption,
                                                              timelinOption);
    
    transformTimelineOptionList.push_back(transformTimelineOpt);
    
    return true;
}

bool xmlToBinary::convertSlotTimelineData(flatbuffers::FlatBufferBuilder &builder,SlotTimeline* timeline,std::vector<flatbuffers::Offset<SlotTimelineOption>> &slotTimelineOptionList)
{
    bool transformed = timeline->transformed;
    float offset = timeline->offset;
    std::string name = timeline->name;
    
    std::vector<flatbuffers::Offset<SlotFrameOption>> slotFrameOptionList;
    
    for (int flIdx = 0; flIdx < timeline->frameList.size(); flIdx++)
    {
        SlotFrame* slotFrame = dynamic_cast<SlotFrame *>(timeline->frameList.at(flIdx));
        if (slotFrame)
        {
            auto frameOption = convertSlotFrameData(builder,slotFrame);
            slotFrameOptionList.push_back(frameOption);
        }
    }
    
    auto timelinOption = CreateTimelineOption(builder,
                                              timeline->duration,
                                              timeline->scale,
                                              0,
                                              slotFrameOptionList.size() == 0 ? 0 : builder.CreateVector(slotFrameOptionList));
    
    auto slotTimelineOption = CreateSlotTimelineOption(builder,
                                                       name.length() == 0 ? 0 : builder.CreateString(name),
                                                       transformed,
                                                       offset,
                                                       timelinOption
                                                       );

    slotTimelineOptionList.push_back(slotTimelineOption);
    return true;
}

bool xmlToBinary::convertFFDTimelineData(flatbuffers::FlatBufferBuilder &builder,FFDTimeline* timeline,std::vector<flatbuffers::Offset<FFDTimelineOption>> &ffdTimelineOptionList)
{
    std::string name = timeline->name;
    std::string skinName = timeline->skinName;
    std::string slotName = timeline->slotName;
    float offset = timeline->offset;

    std::vector<flatbuffers::Offset<FFDFrameOption>> ffdFrameOptionList;
    
    for (auto frame : timeline->frameList)
    {
        FFDFrame* ffdFrame = dynamic_cast<FFDFrame *>(frame);
        if (ffdFrame)
        {
            auto frameOption = convertFFDFrameData(builder,ffdFrame);
            ffdFrameOptionList.push_back(frameOption);
        }
    }
    
    auto timelinOption = CreateTimelineOption(builder,
                                              timeline->duration,
                                              timeline->scale,
                                              0,
                                              0,
                                              ffdFrameOptionList.size() == 0 ? 0 : builder.CreateVector(ffdFrameOptionList)
                                              );
    auto ffdTimelineOption = CreateFFDTimelineOption(builder,
                                                     name.length() == 0 ? 0 : builder.CreateString(name),
                                                     skinName.length() == 0 ? 0 : builder.CreateString(skinName),
                                                     slotName.length() == 0 ? 0 : builder.CreateString(slotName),
                                                     offset,
                                                     timelinOption
                                                     );
    ffdTimelineOptionList.push_back(ffdTimelineOption);
    return true;
}

flatbuffers::Offset<TransformFrameOption> xmlToBinary::convertTransformFrameData(flatbuffers::FlatBufferBuilder &builder,TransformFrame *transformFrame)
{
    if (!transformFrame)
    {
        return 0;
    }

    Transform global = transformFrame->global;
    TransformOption globalOption(global.x,global.y,global.skewX,global.skewY,global.scaleX,global.scaleY);
    Transform transform = transformFrame->transform;
    TransformOption transformOption(transform.x,transform.y,transform.skewX,transform.skewY,transform.scaleX,transform.scaleY);
    dragonBones::Point pivot = transformFrame->pivot;
    PointOption pivotOption(pivot.x,pivot.y);
    
    dragonBones::Point scaleOffset = transformFrame->scaleOffset;
    PointOption scaleOffsetOption(scaleOffset.x,scaleOffset.y);
    
    auto frameOption = convertFrameData(builder, transformFrame);
    auto transformFrameOption = CreateTransformFrameOption(builder,
                                                           transformFrame->visible,
                                                           transformFrame->tweenScale,
                                                           transformFrame->tweenRotate,
                                                           transformFrame->tweenEasing,
                                                           &globalOption,
                                                           &transformOption,
                                                           &pivotOption,
                                                           &scaleOffsetOption,
                                                           frameOption
                                                           );
    return transformFrameOption;
}

flatbuffers::Offset<FrameOption> xmlToBinary::convertFrameData(flatbuffers::FlatBufferBuilder &builder,Frame *frame)
{

    std::string action = frame->action;
    std::string event = frame->event;
    std::string sound = frame->sound;
    
    std::vector<flatbuffers::Offset<Vec2Option>> pointOptionList;
    if (frame->curve)
    {
        for (size_t samIdx = 0; samIdx < frame->curve->_pointList.size();samIdx ++)
        {
            dragonBones::Point point = frame->curve->_pointList[samIdx];
            auto pointOption = CreateVec2Option(builder,point.x,point.y);
            pointOptionList.push_back(pointOption);
        }
        
    }
    
    auto curveDataOption = CreateCurveDataOption(builder,
                                                 pointOptionList.size() == 0 ? 0 : builder.CreateVector(pointOptionList));
    auto frameOption = CreateFrameOption(builder,
                                         frame->position,
                                         frame->duration,
                                         (uint8_t)frame->frameType,
                                         action.length() == 0 ? 0 : builder.CreateString(action),
                                         event.length() == 0 ? 0 : builder.CreateString(event),
                                         sound.length() == 0 ? 0 : builder.CreateString(sound),
                                         frame->curve == nullptr ? 0 : curveDataOption
                                         );
    return frameOption;
}

flatbuffers::Offset<FFDFrameOption> xmlToBinary::convertFFDFrameData(flatbuffers::FlatBufferBuilder &builder,FFDFrame *ffdFrame)
{
    auto frameOption = convertFrameData(builder, ffdFrame);
    auto ffdFrameOption = CreateFFDFrameOption(builder,
                                               ffdFrame->tweenEasing,
                                               ffdFrame->offset,
                                               ffdFrame->vertices.size() == 0 ? 0 : builder.CreateVector(ffdFrame->vertices),
                                               frameOption);
    return ffdFrameOption;
}

flatbuffers::Offset<SlotFrameOption> xmlToBinary::convertSlotFrameData(flatbuffers::FlatBufferBuilder &builder,SlotFrame *slotFrame)
{
    if (!slotFrame)
    {
        return 0;
    }
    auto colorTransformOption = convertColorTransformData(builder, slotFrame->color);
    auto frameOption = convertFrameData(builder, slotFrame);
    auto slotFrameOption = CreateSlotFrameOption(builder,
                                                 slotFrame->visible,
                                                 slotFrame->zOrder,
                                                 slotFrame->displayIndex,
                                                 slotFrame->tweenEasing,
                                                 colorTransformOption,
                                                 frameOption
                                                 );
    return slotFrameOption;
}

flatbuffers::Offset<ColorTransformOption> xmlToBinary::convertColorTransformData(flatbuffers::FlatBufferBuilder &builder,ColorTransform *colorTransform)
{
    if (colorTransform)
    {
        auto colorTransformOption = CreateColorTransformOption(builder,
                                                               colorTransform->alphaMultiplier,
                                                               colorTransform->redMultiplier,
                                                               colorTransform->greenMultiplier,
                                                               colorTransform->blueMultiplier,
                                                               colorTransform->alphaOffset,
                                                               colorTransform->redOffset,
                                                               colorTransform->greenOffset,
                                                               colorTransform->blueOffset
                                                               );
        return colorTransformOption;
    }
    return 0;
    
}

bool xmlToBinary::writeToPath(const char* data,size_t size)
{
    std::string binaryFilePath = m_filePath;
    std::string suffix = ".xmlb";
    if (readXMLB)
    {
        suffix += "b";
    }
    std::string outFilePath = "";
    size_t final_pos = 0;
    char mode = '/';
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    
    mode = '\\';
#else
    mode = '/';
#endif
    final_pos = m_outPath.rfind(mode);
    if (0 < final_pos) {
        outFilePath = m_outPath.substr(0, final_pos + 1);
    }
    
    std::string fileName = "";
    
	final_pos = m_filePath.rfind(mode);
    size_t dot_pos = m_filePath.rfind('.');
    if (0 < dot_pos) {
        final_pos = final_pos == 0 ? 0 : final_pos + 1;
		fileName = m_filePath.substr(final_pos, dot_pos - final_pos);
    }
    fileName += suffix;
    binaryFilePath = outFilePath + fileName;
    
    FILE *fp;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	
	fopen_s(&fp, binaryFilePath.c_str(), "wb");
#else
	fp = fopen(binaryFilePath.c_str(), "wb");
#endif
    
    if (!fp)
    {
        printf("\t%s write error !\n",binaryFilePath.c_str());
        return false;
    }
    fwrite(data, sizeof(unsigned char), size, fp);//(buffer,sizeof(unsigned char), *size,fp);
    fclose(fp);
	printf("\t=====>\t%s\n", binaryFilePath.c_str());
    return true;
}

