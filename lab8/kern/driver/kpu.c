#include "kpu.h"
int kpu_load_kmodel(kpu_model_context_t *ctx, const uint8_t *buffer)
{
    ;
}
int kpu_run_kmodel(kpu_model_context_t *ctx, const uint8_t *src, dmac_channel_number_t dma_ch, kpu_done_callback_t done_callback, void *userdata)
{
    ;
}
int kpu_get_output(kpu_model_context_t *ctx, uint32_t index, uint8_t **data, size_t *size)
{
    ;
}