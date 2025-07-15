#ifndef MODELS_SETTINGS_H_
#define MODELS_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

/* Camera frame size */
#define CAM_FRAMESIZE	FRAMESIZE_96X96

/* Input image shape */
#define IMG_W		96
#define IMG_H		96
#define IMG_CH	3

/* P-Net models scales */
#define PNET_1_SCALE	0.3333333f
#define PNET_2_SCALE	0.25f
#define PNET_3_SCALE	0.125f

#ifdef __cplusplus
}
#endif

#endif /* MODELS_SETTINGS_H_ */
