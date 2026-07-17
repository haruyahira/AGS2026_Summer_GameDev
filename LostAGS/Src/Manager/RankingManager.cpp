#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include "RankingManager.h"

//--------------------------------------------------
// インスタンス取得
//--------------------------------------------------
RankingManager& RankingManager::GetInstance(void)
{
    static RankingManager instance;

    return instance;
}

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
RankingManager::RankingManager(void)
    :
    isLoaded_(false)
{
}

//--------------------------------------------------
// 初期化
//--------------------------------------------------
void RankingManager::Init(void)
{
    //---------------------------------
    // 読み込み済みなら何もしない
    //---------------------------------
    if (isLoaded_)
    {
        return;
    }

    Load();
}

//--------------------------------------------------
// 読み込み
//--------------------------------------------------
void RankingManager::Load(void)
{
    //---------------------------------
    // 現在のランキングを消去
    //---------------------------------
    ranking_.clear();

    //---------------------------------
    // ランキングファイルを開く
    //---------------------------------
    std::ifstream file(
        SAVE_FILE_PATH,
        std::ios::binary
    );

    //---------------------------------
    // ファイルが存在しない場合
    //---------------------------------
    if (!file.is_open())
    {
        isLoaded_ = true;

        return;
    }

    //---------------------------------
    // ファイル識別子を読み込む
    //---------------------------------
    unsigned int magic = 0;

    file.read(
        reinterpret_cast<char*>(
            &magic
            ),
        sizeof(magic)
    );

    //---------------------------------
    // 旧形式または壊れたファイル
    //---------------------------------
    if (!file ||
        magic != FILE_MAGIC)
    {
        file.close();

        ranking_.clear();
        isLoaded_ = true;

        return;
    }

    //---------------------------------
    // ファイルバージョンを読み込む
    //---------------------------------
    unsigned int version = 0;

    file.read(
        reinterpret_cast<char*>(
            &version
            ),
        sizeof(version)
    );

    //---------------------------------
    // バージョンが一致しない場合
    //---------------------------------
    if (!file ||
        version != FILE_VERSION)
    {
        file.close();

        ranking_.clear();
        isLoaded_ = true;

        return;
    }

    //---------------------------------
    // 登録件数を読み込む
    //---------------------------------
    int count = 0;

    file.read(
        reinterpret_cast<char*>(
            &count
            ),
        sizeof(count)
    );

    //---------------------------------
    // 登録件数が不正な場合
    //---------------------------------
    if (!file ||
        count < 0 ||
        count > MAX_RANKING_COUNT)
    {
        file.close();

        ranking_.clear();
        isLoaded_ = true;

        return;
    }

    //---------------------------------
    // ランキングデータを読み込む
    //---------------------------------
    for (int index = 0;
        index < count;
        ++index)
    {
        RankingRecord record;

        record.name.clear();
        record.money = 0;

        //---------------------------------
        // 名前のバイト数を読み込む
        //---------------------------------
        int nameLength = 0;

        file.read(
            reinterpret_cast<char*>(
                &nameLength
                ),
            sizeof(nameLength)
        );

        //---------------------------------
        // 名前の長さが不正な場合
        //---------------------------------
        if (!file ||
            nameLength < 0 ||
            nameLength > MAX_NAME_BYTE_LENGTH)
        {
            ranking_.clear();

            break;
        }

        //---------------------------------
        // 名前を読み込む
        //---------------------------------
        if (nameLength > 0)
        {
            record.name.resize(
                static_cast<std::size_t>(
                    nameLength
                    )
            );

            file.read(
                &record.name[0],
                static_cast<std::streamsize>(
                    nameLength
                    )
            );

            if (!file)
            {
                ranking_.clear();

                break;
            }
        }

        //---------------------------------
        // 金額を読み込む
        //---------------------------------
        file.read(
            reinterpret_cast<char*>(
                &record.money
                ),
            sizeof(record.money)
        );

        if (!file)
        {
            ranking_.clear();

            break;
        }

        //---------------------------------
        // 名前が空なら代替名を設定
        //---------------------------------
        if (record.name.empty())
        {
            record.name =
                "NO NAME";
        }

        //---------------------------------
        // 負の金額を防ぐ
        //---------------------------------
        if (record.money < 0)
        {
            record.money = 0;
        }

        //---------------------------------
        // 読み込んだ記録を追加
        //---------------------------------
        ranking_.push_back(
            record
        );
    }

    file.close();

    //---------------------------------
    // 金額が高い順に並べ替える
    //---------------------------------
    std::sort(
        ranking_.begin(),
        ranking_.end(),
        const RankingRecord & left,
        const RankingRecord & right
        {
            return left.money >
                right.money;
        }
    );

    //---------------------------------
    // 最大10件に制限
    //---------------------------------
    if (ranking_.size() >
        static_cast<std::size_t>(
            MAX_RANKING_COUNT
            ))
    {
        ranking_.resize(
            MAX_RANKING_COUNT
        );
    }

    isLoaded_ = true;
}

//--------------------------------------------------
// 再読み込み
//--------------------------------------------------
void RankingManager::Reload(void)
{
    //---------------------------------
    // 未読み込み状態へ戻す
    //---------------------------------
    isLoaded_ = false;

    Load();
}

//--------------------------------------------------
// 保存
//--------------------------------------------------
void RankingManager::Save(void) const
{
    //---------------------------------
    // 保存先フォルダーを作成する
    //---------------------------------
    std::error_code errorCode;

    std::filesystem::create_directories(
        SAVE_DIRECTORY,
        errorCode
    );

    //---------------------------------
    // フォルダー作成に失敗した場合
    //---------------------------------
    if (errorCode)
    {
        return;
    }

    //---------------------------------
    // ランキングファイルを開く
    //---------------------------------
    std::ofstream file(
        SAVE_FILE_PATH,
        std::ios::binary |
        std::ios::trunc
    );

    if (!file.is_open())
    {
        return;
    }

    //---------------------------------
    // ファイル識別子を書き込む
    //---------------------------------
    const unsigned int magic =
        FILE_MAGIC;

    file.write(
        reinterpret_cast<const char*>(
            &magic
            ),
        sizeof(magic)
    );

    //---------------------------------
    // ファイルバージョンを書き込む
    //---------------------------------
    const unsigned int version =
        FILE_VERSION;

    file.write(
        reinterpret_cast<const char*>(
            &version
            ),
        sizeof(version)
    );

    //---------------------------------
    // 登録件数を書き込む
    //---------------------------------
    const int count =
        static_cast<int>(
            ranking_.size()
            );

    file.write(
        reinterpret_cast<const char*>(
            &count
            ),
        sizeof(count)
    );

    //---------------------------------
    // 各記録を書き込む
    //---------------------------------
    for (const RankingRecord& record :
        ranking_)
    {
        //---------------------------------
        // 名前の長さ
        //---------------------------------
        const int nameLength =
            static_cast<int>(
                record.name.size()
                );

        file.write(
            reinterpret_cast<const char*>(
                &nameLength
                ),
            sizeof(nameLength)
        );

        //---------------------------------
        // 名前
        //---------------------------------
        if (nameLength > 0)
        {
            file.write(
                record.name.data(),
                static_cast<std::streamsize>(
                    nameLength
                    )
            );
        }

        //---------------------------------
        // 金額
        //---------------------------------
        file.write(
            reinterpret_cast<const char*>(
                &record.money
                ),
            sizeof(record.money)
        );

        //---------------------------------
        // 書き込み失敗
        //---------------------------------
        if (!file)
        {
            file.close();

            return;
        }
    }

    file.close();
}

//--------------------------------------------------
// 名前と金額を登録
//--------------------------------------------------
int RankingManager::RegisterRecord(
    const std::string& name,
    int money)
{
    //---------------------------------
    // ランキングを読み込んでいない場合
    //---------------------------------
    if (!isLoaded_)
    {
        Load();
    }

    //---------------------------------
    // 新しいランキング記録
    //---------------------------------
    RankingRecord newRecord;

    //---------------------------------
    // 名前を設定
    //---------------------------------
    if (name.empty())
    {
        newRecord.name =
            "NO NAME";
    }
    else
    {
        newRecord.name =
            name;
    }

    //---------------------------------
    // 名前が異常に長い場合は切り詰める
    //
    // 日本語は複数バイトで構成されるため、
    // 通常は入力側で長さを制限する
    //---------------------------------
    if (newRecord.name.size() >
        static_cast<std::size_t>(
            MAX_NAME_BYTE_LENGTH
            ))
    {
        newRecord.name.resize(
            MAX_NAME_BYTE_LENGTH
        );
    }

    //---------------------------------
    // 金額を設定
    //---------------------------------
    if (money < 0)
    {
        newRecord.money = 0;
    }
    else
    {
        newRecord.money = money;
    }

    //---------------------------------
    // 挿入位置を探す
    //
    // 同額の場合は既存記録の後ろへ入れる
    //---------------------------------
    int insertIndex = 0;

    while (
        insertIndex <
        static_cast<int>(
            ranking_.size()
            ) &&
        ranking_[
            insertIndex
        ].money >=
        newRecord.money)
    {
        ++insertIndex;
    }

    //---------------------------------
    // 今回の順位
    //---------------------------------
    const int newRank =
        insertIndex + 1;

    //---------------------------------
    // ランキングへ追加
    //---------------------------------
    ranking_.insert(
        ranking_.begin() +
        insertIndex,
        newRecord
    );

    //---------------------------------
    // 最大10件に制限
    //---------------------------------
    if (ranking_.size() >
        static_cast<std::size_t>(
            MAX_RANKING_COUNT
            ))
    {
        ranking_.resize(
            MAX_RANKING_COUNT
        );
    }

    //---------------------------------
    // 10位より下ならランキング外
    //---------------------------------
    if (newRank >
        MAX_RANKING_COUNT)
    {
        return -1;
    }

    return newRank;
}

//--------------------------------------------------
// 指定順位の記録取得
//--------------------------------------------------
const RankingRecord*
RankingManager::GetRecord(
    int rank) const
{
    //---------------------------------
    // 順位を配列番号へ変換する
    //---------------------------------
    const int index =
        rank - 1;

    //---------------------------------
    // 範囲外
    //---------------------------------
    if (index < 0 ||
        index >=
        static_cast<int>(
            ranking_.size()
            ))
    {
        return nullptr;
    }

    return &ranking_[
        index
    ];
}

//--------------------------------------------------
// 指定順位の名前取得
//--------------------------------------------------
std::string RankingManager::GetName(
    int rank) const
{
    const RankingRecord* record =
        GetRecord(
            rank
        );

    if (record == nullptr)
    {
        return "";
    }

    return record->name;
}

//--------------------------------------------------
// 指定順位の金額取得
//--------------------------------------------------
int RankingManager::GetMoney(
    int rank) const
{
    const RankingRecord* record =
        GetRecord(
            rank
        );

    if (record == nullptr)
    {
        return 0;
    }

    return record->money;
}

//--------------------------------------------------
// 全ランキング取得
//--------------------------------------------------
const std::vector<RankingRecord>&
RankingManager::GetRanking(void) const
{
    return ranking_;
}

//--------------------------------------------------
// 件数取得
//--------------------------------------------------
int RankingManager::GetRankingCount(void) const
{
    return static_cast<int>(
        ranking_.size()
        );
}

//--------------------------------------------------
// 全記録削除
//--------------------------------------------------
void RankingManager::Clear(void)
{
    ranking_.clear();

    isLoaded_ = true;

    Save();
}