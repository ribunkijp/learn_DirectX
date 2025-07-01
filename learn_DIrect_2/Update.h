/**********************************************************************************
    Update.h

                                                                LI WENHUI
                                                                2025/06/30

**********************************************************************************/
#ifndef UPDATE_H
#define UPDATE_H
#include "StateInfo.h"







//
void UpdateAllObjects(StateInfo* pState, float deltaTime);

//
void UpdatePlayer(StateInfo* state, float deltaTime, bool leftPressed, bool rightPressed);

//
void UpdateCamera(StateInfo* state);

#endif 
