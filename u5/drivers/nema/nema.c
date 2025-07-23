

#define DT_DRV_COMPAT st_nema_gfx

#include <zephyr/device.h>

#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/irq.h>

#include <stm32u5xx_hal.h>
#include <app/drivers/nema_gfx.h>


LOG_MODULE_REGISTER(nema_gfx, CONFIG_NEMA_LOG_LEVEL);

GPU2D_HandleTypeDef hgpu2d;

struct nema_gfx_data {
	GPU2D_HandleTypeDef *hgpu2d;
};

struct nema_gfx_config {
};


static void nema_stm32_isr(const void *arg)
{
	HAL_GPU2D_IRQHandler(&hgpu2d);
}

static void nema_stm32_isr_err(const void *arg)
{
	HAL_GPU2D_ER_IRQHandler(&hgpu2d);
}


static int nema_gfx_init(const struct device *dev)
{
    struct nema_gfx_data *data = dev->data;

    data->hgpu2d = &hgpu2d;
    hgpu2d.Instance = GPU2D;

    __HAL_RCC_GPU2D_CLK_ENABLE();
    __HAL_RCC_DCACHE2_CLK_ENABLE();

    HAL_NVIC_SetPriority(GPU2D_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPU2D_IRQn);
    HAL_NVIC_SetPriority(GPU2D_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPU2D_ER_IRQn);

    IRQ_CONNECT(GPU2D_IRQn, 0,
                nema_stm32_isr, 0, 0);

    IRQ_CONNECT(GPU2D_ER_IRQn, 0,
                nema_stm32_isr_err, 0, 0);

    if (HAL_GPU2D_Init(&hgpu2d) != HAL_OK)
    {
        printk("Failed: HAL_GPU2D_Init\n");
        return -1;
    }
    printk("st GPU2D, Nema gfx OK\n");
    return 0;
}

static void *nema_gfx_get_inst (const struct device *dev)
{
    struct nema_gfx_data *data = dev->data;
    return data->hgpu2d;
}

static DEVICE_API(nema, nema_driver_api) = {
    .get_instance = nema_gfx_get_inst
};


#define NEMA_GFX_DEFINE(inst)                                            \
	static struct nema_gfx_data data##inst;                          \
                                                                               \
	static const struct nema_gfx_config config##inst = {             \
	};                                                                     \
                                                                               \
	DEVICE_DT_INST_DEFINE(inst, nema_gfx_init, NULL, &data##inst,    \
			      &config##inst, POST_KERNEL,                      \
			      CONFIG_NEMA_INIT_PRIORITY,                      \
			      &nema_driver_api);

DT_INST_FOREACH_STATUS_OKAY(NEMA_GFX_DEFINE)