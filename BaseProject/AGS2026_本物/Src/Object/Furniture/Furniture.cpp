#pragma once
#include "../Common/Transform.h"
#include "../../Manager/ResourceManager.h"
#include "../Name.h"
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

        for (const auto& box : colliders_) {
            box.DrawDebug(GetColor(0, 255, 0));
        }
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