JCLASS(PumpkinBomb, _)
JEXPOSE(JUUID, explosion, "", jedv_t_te_mold, 1, false)
JEXPOSE_TRANSFORM(XMFLOAT3, explosion_offset, ToXMFLOAT3, FromXMFLOAT3, XMFLOAT3(0.0f, 0.0f, 0.0f), jedv_t_float3, 1, false)
JEXPOSE_TRANSFORM(ControllerBinding, enemy, ToControllerBinding, FromControllerBinding, {}, jedv_t_so_controller_instance, 1, false)