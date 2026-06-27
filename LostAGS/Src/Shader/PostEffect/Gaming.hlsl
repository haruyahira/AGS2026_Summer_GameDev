#include "../Common/Pixel/PixelShader2DHeader.hlsli"
 
// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_time;
}
 
float4 main(PS_INPUT PSInput) : SV_TARGET
{
 
	// UV座標とテクスチャを参照して、最適な色を取得する
    float4 srcCol = tex.Sample(texSampler, PSInput.uv);
    if (srcCol.a < 0.01f)
    {
		// 描画しない(アルファテスト)
        discard;
    }
    float4 dstCol = srcCol;
   // ゲーミング処理
    float t = g_time.x;

   // RGBを時間差で変化
    float r = (sin(t) + 1.0f) * 0.5f;
    float g = (sin(t + 2.0f) + 1.0f) * 0.5f;
    float b = (sin(t + 4.0f) + 1.0f) * 0.5f;

   // 元画像に掛ける
    dstCol.rgb = srcCol.rgb * float3(r, g, b);


    return dstCol;
 
}