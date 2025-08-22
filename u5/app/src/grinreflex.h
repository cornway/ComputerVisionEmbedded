
#ifndef GRINREFLEX_APPLICATION_H
#define GRINREFLEX_APPLICATION_H

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NAND_PATH(_path) ("/NAND:" _path)

int init();

int loop();

#ifdef __cplusplus
}
#endif

#endif /* GRINREFLEX_APPLICATION_H */