#include "kpu.h"
int kpu_load_kmodel(kpu_model_context_t *ctx, const uint8_t *buffer)
{
#if FIX_CACHE
    configASSERT(is_memory_cache((uintptr_t)buffer));
#endif
    uintptr_t base_addr = (uintptr_t)buffer;
    const kpu_kmodel_header_t *header = (const kpu_kmodel_header_t *)buffer;

    if(header->version == 3 && header->arch == 0)
    {
        ctx->is_nncase = 0;
        ctx->model_buffer = buffer;
        ctx->output_count = header->output_count;
        ctx->outputs = (const kpu_model_output_t *)(base_addr + sizeof(kpu_kmodel_header_t));
        ctx->layer_headers = (const kpu_model_layer_header_t *)((uintptr_t)ctx->outputs + sizeof(kpu_model_output_t) * ctx->output_count);
        ctx->layers_length = header->layers_length;
        ctx->body_start = (const uint8_t *)((uintptr_t)ctx->layer_headers + sizeof(kpu_model_layer_header_t) * header->layers_length);
        ctx->main_buffer = (uint8_t *)kmalloc(header->main_mem_usage);//需要考虑malloc的实现
        if(!ctx->main_buffer)
            return -1;
        uint32_t body_size = 0;
        for(int i=0; i<ctx->layers_length; i++)
        {
            const kpu_model_layer_header_t *cnt_layer_header = ctx->layer_headers + i;
            body_size += cnt_layer_header->body_size;
        }
        uint8_t *body_start_iomem = (uint8_t *)((uintptr_t)ctx->body_start - IOMEM);
        const uint8_t *body_start_cache = ctx->body_start;
        memcpy(body_start_iomem, body_start_cache, body_size);
        for(int i=0; i<body_size; i++)
        {
            configASSERT(body_start_iomem[i] == body_start_cache[i]);
        }
        
    } 
    //还需要移值神经网络加速器nncase？ 
    // else if(header->version == 'KMDL')
    // {
    //     return nncase_load_kmodel(ctx, buffer);
    // } 
    else
    {
        return -1;
    }

    return 0;
}
int kpu_run_kmodel(kpu_model_context_t *ctx, const uint8_t *src, dmac_channel_number_t dma_ch, kpu_done_callback_t done_callback, void *userdata)
{
    ;
}
int kpu_get_output(kpu_model_context_t *ctx, uint32_t index, uint8_t **data, size_t *size)
{
    ;
}