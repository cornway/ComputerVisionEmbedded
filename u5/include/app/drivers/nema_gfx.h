

#ifndef APP_DRIVERS_NEMA_H_
#define APP_DRIVERS_NEMA_H_

#include <zephyr/device.h>
#include <zephyr/toolchain.h>


__subsystem struct nema_driver_api {
    void *(*get_instance)(const struct device *dev);
};

//__syscall int nema_gfx_get_gpu_inst(const struct device *dev, void **gpu_inst);

static inline int nema_gfx_get_gpu_inst(const struct device *dev, void **gpu_inst)
{
	__ASSERT_NO_MSG(DEVICE_API_IS(nema, dev));

	*gpu_inst = DEVICE_API_GET(nema, dev)->get_instance(dev);
    return 0;
}

#endif /*APP_DRIVERS_NEMA_H_*/