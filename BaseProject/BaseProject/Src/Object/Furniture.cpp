#pragma once
#include "Common/Transform.h"
#include "../Manager/ResourceManager.h"
#include "Name.h"
#include "Furniture.h"

    // コンストラクタで必要な情報を全部もらう
Furniture::Furniture(NAME name, const Transform* trans)
        : name_(name), trans_(*trans) {
    }

    void Furniture::Init() {
        // 必要なら初期化処理
    }

    void Furniture::Update() {
        trans_.Update();
    }

    void Furniture::Draw() {
        MV1DrawModel(trans_.modelId);
    }