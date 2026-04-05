JCLASS(BrawlerCharacter, _)
JEXPOSE_ENUM(CharacterLookingTo, lookingTo, CLT_Right, jedv_t_enum, 1, false)
JEXPOSE(int, health, 100, jedv_t_integer, 1, false)
JEXPOSE_TRANSFORM(XMFLOAT3, lookToSwapVector, ToXMFLOAT3, FromXMFLOAT3, XMFLOAT3(1.0f, 1.0f, 1.0f), jedv_t_float3, 1, false)
