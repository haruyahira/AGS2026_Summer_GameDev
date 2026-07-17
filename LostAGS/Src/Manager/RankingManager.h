#pragma once

#include <string>
#include <vector>

struct RankingRecord
{
    //---------------------------------
    // プレイヤー名
    //---------------------------------
    std::string name;

    //---------------------------------
    // 獲得金額
    //---------------------------------
    int money;
};

class RankingManager
{
public:
    //---------------------------------
    // 最大10位
    //---------------------------------
    static constexpr int MAX_RANKING_COUNT = 10;

public:
    static RankingManager& GetInstance(void);

    void Init(void);
    void Load(void);
    void Reload(void);
    void Save(void) const;
    void Clear(void);

    //---------------------------------
    // 名前と金額を登録する
    //
    // 戻り値:
    // 1～10 : ランクイン
    // -1    : ランキング外
    //---------------------------------
    int RegisterRecord(
        const std::string& name,
        int money
    );

    //---------------------------------
    // 指定順位の記録を取得
    //---------------------------------
    const RankingRecord* GetRecord(
        int rank
    ) const;

    const std::vector<RankingRecord>&
        GetRanking(void) const;

    int GetRankingCount(void) const;

private:
    RankingManager(void);
    ~RankingManager(void) = default;

    RankingManager(
        const RankingManager&) = delete;

    RankingManager& operator=(
        const RankingManager&) = delete;

private:
    static constexpr const char* SAVE_DIRECTORY =
        "Data/Save";

    static constexpr const char* SAVE_FILE_PATH =
        "Data/Save/Ranking.dat";

    //---------------------------------
    // セーブデータ識別子
    //---------------------------------
    static constexpr unsigned int FILE_MAGIC =
        0x52414E4B;

    //---------------------------------
    // セーブデータ形式のバージョン
    //---------------------------------
    static constexpr unsigned int FILE_VERSION =
        2;

    std::vector<RankingRecord> ranking_;

    bool isLoaded_;
};