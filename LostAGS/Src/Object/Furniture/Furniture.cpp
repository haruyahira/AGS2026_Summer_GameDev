#pragma once
#include "../Common/Transform.h"
#include "../../Manager/ResourceManager.h"
#include "../Name.h"
#include "Furniture.h"

    // コンストラクタで必要な情報を全部もらう
Furniture::Furniture(NAME name, const Transform* trans)
        : name_(name), trans_(*trans) {
    }

Furniture::~Furniture()
{

    if (trans_.modelId != -1)
    {
        ResourceManager::GetInstance().DeleteDuplicateModel(trans_.modelId);
    }

}

    void Furniture::Init() {
        // 必要なら初期化処理
    }

    void Furniture::Update() {
        trans_.Update();
    }

    void Furniture::Draw() {
        MV1DrawModel(trans_.modelId);

#ifdef _DEBUG
        for (const auto& box : colliders_) {
            box.DrawDebug(GetColor(0, 255, 0));
        }
#endif
    }

    bool Furniture::ResolveCollision(
        VECTOR& pos,
        float radius,
        float bottomY,
        float topY)
    {
        return false;
    }

    bool Furniture::ResolveCameraCollision(
        VECTOR& cameraPos,
        float radius)
    {
        for (const auto& box : colliders_)
        {
            VECTOR closest = box.GetClosestPoint(cameraPos);
            VECTOR diff = VSub(cameraPos, closest);

            float distSq = VDot(diff, diff);

            if (distSq < radius * radius)
            {
                if (distSq > 0.0001f)
                {
                    float dist = sqrtf(distSq);
                    VECTOR dir = VScale(diff, 1.0f / dist);

                    cameraPos = VAdd(
                        closest,
                        VScale(dir, radius)
                    );
                }
                else
                {
                    cameraPos.y += radius;
                }

                return true;
            }
        }

        return false;
    }

    bool Furniture::IsBlockingSight(VECTOR start, VECTOR end) const
    {
        return false;
    }


    bool Furniture::IsUnder(VECTOR pos) const
    {
        // 隠れポイントじゃないなら無視
        if (!isHideSpot_) return false;

        // 一番上のY（天板）を探す
        float topY = -99999.0f;

        for (const auto& box : colliders_)
        {
            float boxTop = box.center.y + box.halfSize.y;
            if (boxTop > topY)
            {
                topY = boxTop;
            }
        }

        // プレイヤーが天板より下ならOK
        if (pos.y > topY) return false;

        // XZ範囲内にいるか
        for (const auto& box : colliders_)
        {
            float minX = box.center.x - box.halfSize.x;
            float maxX = box.center.x + box.halfSize.x;
            float minZ = box.center.z - box.halfSize.z;
            float maxZ = box.center.z + box.halfSize.z;

            if (pos.x >= minX && pos.x <= maxX &&
                pos.z >= minZ && pos.z <= maxZ)
            {
                return true;
            }
        }

        return false;
    }

    int Furniture::GetModelId(void) const
    {
        return trans_.modelId;
    }