float4 PSMain( float4 pos : SV_POSITION ) : SV_TARGET
{
    //寒色がかった黒
    return float4(0.05, 0.07, 0.10, 1.0);
}