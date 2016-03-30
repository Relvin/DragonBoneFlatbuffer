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
        if (suffix == "xml")
        {
            readXMLB = false;
            dragonBones::XMLDataParser parser;
            dragonBones::XMLDocument doc;
            doc.Parse(reinterpret_cast<char*>(buffer), readsize);
            auto dragonBonesData = parser.parseDragonBonesData(doc.RootElement());
            this->convertDragonboneToBinary(dragonBonesData);
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
    
    auto skinName = builder.CreateString(skinData->name);
    skinDataOptionList.push_back(
                                 CreateSkinDataOption(builder,
                                                      skinData->name.length() == 0 ? 0 : skinName,
                                                      slotDataOptionList.size() == 0 ? 0 : builder.CreateVector(slotDataOptionList)));
    return true;
}

bool xmlToBinary::convertSlotData(flatbuffers::FlatBufferBuilder &builder,SlotData* slotData,std::vector<flatbuffers::Offset<SlotDataOption>> &slotDataOptionList)
{
    
    
    std::vector<flatbuffers::Offset<DisplayDataOption>> displayDataArr;
    for (int disIdx = 0; disIdx < slotData->displayDataList.size(); disIdx++)
    {
        convertDisplayData(builder,slotData->displayDataList.at(disIdx), displayDataArr);
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
                                           displayDataArr.size() == 0 ? 0 : builder.CreateVector(displayDataArr));
    
    slotDataOptionList.push_back(slotOption);
    
    return true;
}


bool xmlToBinary::convertDisplayData(flatbuffers::FlatBufferBuilder &builder,DisplayData* displayData,std::vector<flatbuffers::Offset<DisplayDataOption>> &displayOptionList)
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
    
    displayOptionList.push_back(dispLayDataOption);
    
    return true;
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
                                                     slotTimelineOptionList.size() == 0 ? 0 : builder.CreateVector(slotTimelineOptionList)
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
    
    uint8_t dataChanged = 0;
    std::vector<const PointOption *> samplingOptionList;
    std::vector<const PointOption *> pointOptionList;
    if (frame->curve)
    {
        dataChanged = frame->curve->_dataChanged;
        for (size_t samIdx = 0; samIdx < frame->curve->sampling.size();samIdx ++)
        {
            dragonBones::Point *sampPoint = frame->curve->sampling[samIdx];
            PointOption pointOption(sampPoint->x,sampPoint->y);
            samplingOptionList.push_back(&pointOption);
        }
        
        for (size_t samIdx = 0; samIdx < frame->curve->_pointList.size();samIdx ++)
        {
            dragonBones::Point *point = frame->curve->_pointList[samIdx];
            PointOption pointOption(point->x,point->y);
            pointOptionList.push_back(&pointOption);
        }
        
    }
    
    auto curveDataOption = CreateCurveDataOption(builder,
                                                 dataChanged,
                                                 pointOptionList.size() == 0 ? 0 : builder.CreateVector(pointOptionList),
                                                 samplingOptionList.size() == 0 ? 0 : builder.CreateVector(samplingOptionList));
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


#if 0

bool xmlToBinary::convertTransformFrameData(flatbuffers::FlatBufferBuilder &builder,TransformFrame* tfFrame,std::vector<flatbuffers::Offset<TransformFrameOption>> &frameOptionArr)
{
    if (!tfFrame)
    {
        return false;
    }
    
    std::string action = tfFrame->action;
    std::string event = tfFrame->event;
    std::string sound = tfFrame->sound;
    std::string eventPs = tfFrame->eventParameters;
    
    auto frameOption = CreateFrameOption(builder,
                                         tfFrame->position,
                                         tfFrame->duration,
                                         (uint8_t)tfFrame->frameType,
                                         action.length() == 0 ? 0 : builder.CreateString(action),
                                         event.length() == 0 ? 0 : builder.CreateString(event),
                                         sound.length() == 0 ? 0 : builder.CreateString(sound),
                                         eventPs.length() == 0 ? 0 : builder.CreateString(eventPs));
    
    
    ColorTransform *color = tfFrame->color;
    flatbuffers::Offset<ColorTransformOption> colorTfOpt;
    if (color)
    {
        colorTfOpt = CreateColorTransformOption(builder,
                                                color->alphaMultiplier,
                                                color->redMultiplier,
                                                color->greenMultiplier,
                                                color->blueMultiplier,
                                                color->alphaOffset,
                                                color->redOffset,
                                                color->greenOffset,
                                                color->blueOffset
                                                );
    }
    
    Transform global = tfFrame->global;
    TransformOption frameglobalOption(global.x,global.y,global.skewX,global.skewY,global.scaleX,global.scaleY);
    Transform transform = tfFrame->transform;
    TransformOption frameTransformOption(transform.x,transform.y,transform.skewX,transform.skewY,transform.scaleX,transform.scaleY);
    dragonBones::Point pivot = tfFrame->pivot;
    Vec2Option pivotOpt(pivot.x, pivot.y);
    dragonBones::Point scaleOffset = tfFrame->scaleOffset;
    Vec2Option scaleOffsetOpt(scaleOffset.x, scaleOffset.y);
    auto tfFrameOption = CreateTransformFrameOption(builder,
                                                    tfFrame->visible,
                                                    tfFrame->tweenScale,
                                                    tfFrame->tweenRotate,
                                                    tfFrame->displayIndex,
                                                    tfFrame->zOrder,
                                                    tfFrame->tweenEasing,
                                                    &frameglobalOption,
                                                    &frameTransformOption,
                                                    &pivotOpt,
                                                    &scaleOffsetOpt,
                                                    color == nullptr ? 0 : colorTfOpt,
                                                    frameOption
                                                    );
    frameOptionArr.push_back(tfFrameOption);
    return true;
}

#endif

bool xmlToBinary::writeToPath(const char* data,size_t size)
{
	printf("%s\n", m_filePath.c_str());
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
    
	
	if (Platform::DirIsExist(outFilePath.c_str()) == false && notCreate == false)
    {
		printf("%s outFilePath not exit \n",outFilePath.c_str());
		Platform::create_multi_dir(outFilePath.c_str());
		notCreate = true;
    }
    
    FILE *fp;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	
	fopen_s(&fp, binaryFilePath.c_str(), "wb");
#else
	fp = fopen(binaryFilePath.c_str(), "wb");
#endif
    
    if (!fp)
    {
        printf("%s write error !\n",binaryFilePath.c_str());
        return false;
    }
    fwrite(data, sizeof(unsigned char), size, fp);//(buffer,sizeof(unsigned char), *size,fp);
    fclose(fp);
	printf("=====>%s\n", binaryFilePath.c_str());
    return true;
}

