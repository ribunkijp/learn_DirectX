/**********************************************************************************
    AnimationData.h

                                                                LI WENHUI
                                                                2025/07/8

**********************************************************************************/
#ifndef ANIMATIONDATA_H
#define ANIMATIONDATA_H

#include <string>

struct AnimationData {
    std::wstring texturePath;
    int totalFrames;
    int columns;
    int rows;
    float fps;
};


#endif