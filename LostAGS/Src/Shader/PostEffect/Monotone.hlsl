#include "../Common/Pixel/PixelShader2DHeader.hlsli"
 
// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color;
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
    
    // まずシェーダーとは、
    // このピクセルを何色で塗るのかをGPUが計算している
    // モノクロ処理
    // 黄金比でモノクロの明るさを計算する
    float gray = (srcCol.r * 0.299f) + 
    (srcCol.g * 0.587f) + 
    (srcCol.b * 0.114f);
    
    // 元画像に掛ける
    dstCol.rgb = gray;
    //dstCol.rgb = lerp(srcCol.rgb, gray, 0.3f);



    return dstCol;
 
}