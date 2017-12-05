#include "DxLib.h"
#include "Simulator.h"
#include <iostream>
#include <cstdlib>

bool Initialize(int, int, int);
bool CanLoop();


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    int screenWidth = 600;
    int screenHeight = 600;
    if ( !Initialize(screenWidth, screenHeight, screenHeight) ) {
        return 0;
    }
    Simulator *s = new Simulator(__argc, __argv, screenWidth, screenHeight, screenHeight);
    while ( CanLoop() && s->Update() ) {
        //描写対象が無くなるまたはescボタン押下までループ
    }
    delete s;
    DxLib_End();    // DXライブラリ終了処理
    return 0;
}

bool Initialize(int w, int h, int d) {
    SetMainWindowText("Gravity3D");
    SetGraphMode(w, h, 32, 60);
    if ( ChangeWindowMode(true) != 0 ) {
        return false;
    }
    if ( DxLib_Init() != 0 ) {
        return false;
    }
    if ( SetDrawScreen(DX_SCREEN_BACK) != 0 ) {
        DxLib_End();
        return false;
    }

    SetupCamera_Perspective(1.2f);
    SetCameraNearFar(0.1f, 5000.0f);
    ChangeLightTypeDir(VGet(-1, 0, -1));
    SetCameraPositionAndTarget_UpVecY(VGet(-w/2, h/2, -d/2), VGet(0.0f, 1.0f, 0.0f));
    return true;
}


bool CanLoop() {
    if ( ScreenFlip() != 0 || ClearDrawScreen() != 0 || ProcessMessage() != 0 ) {
        return false;
    }
    if ( CheckHitKey(KEY_INPUT_ESCAPE) == 1 ) {
        return false;
    }
    return true;
}
