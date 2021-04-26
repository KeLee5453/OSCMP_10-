
#include <sysctl.h>
#include <defs.h>
#include <stdio.h>
#include <dmac.h>
#include <kpu.h>
#include <kmalloc.h>
#include <string.h>
#include <util.h>
#include <memlayout.h>
#define IOMEM 0x40000000

int kpu_load_kmodel(kpu_model_context_t *ctx, const uint8_t *buffer)
{
#if FIX_CACHE
    configASSERT(is_memory_cache((uintptr_t)buffer));
#endif
    uintptr_t base_addr = (uintptr_t)buffer;
    const kpu_kmodel_header_t *header = (const kpu_kmodel_header_t *)buffer;

    if (header->version == 3 && header->arch == 0)
    {
        ctx->is_nncase = 0;
        ctx->model_buffer = buffer;
        ctx->output_count = header->output_count;
        ctx->outputs = (const kpu_model_output_t *)(base_addr + sizeof(kpu_kmodel_header_t));
        ctx->layer_headers = (const kpu_model_layer_header_t *)((uintptr_t)ctx->outputs + sizeof(kpu_model_output_t) * ctx->output_count);
        ctx->layers_length = header->layers_length;
        ctx->body_start = (const uint8_t *)((uintptr_t)ctx->layer_headers + sizeof(kpu_model_layer_header_t) * header->layers_length);
        ctx->main_buffer = (uint8_t *)kmalloc(header->main_mem_usage);
        if (!ctx->main_buffer)
            return -1;
        uint32_t body_size = 0;
        for (int i = 0; i < ctx->layers_length; i++)
        {
            const kpu_model_layer_header_t *cnt_layer_header = ctx->layer_headers + i;
            body_size += cnt_layer_header->body_size;
        }
        cprintf("IOMEM begin\n");
        uint8_t *body_start_iomem = (uint8_t *)((uintptr_t)ctx->body_start - IOMEM << 1);
        cprintf("IOMEM end\n");
        cprintf("ctx->body_start: 0x%16x\n", ctx->body_start);
        const uint8_t *body_start_cache = ctx->body_start;
        memcpy(body_start_iomem, body_start_cache, body_size);
        for (int i = 0; i < body_size; i++)
        {
            configASSERT(body_start_iomem[i] == body_start_cache[i]);
        }
    }
    // else if (header->version == 'KMDL')
    // {
    //     return nncase_load_kmodel(ctx, buffer);
    // }
    else
    {
        return -1;
    }

    return 0;
}