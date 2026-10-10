#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "kms.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static struct gbm_bo* init_bo(GBM* gbm, u64 modifier)
{
	struct gbm_bo* bo = NULL;

	bo = gbm_bo_create_with_modifiers(gbm->dev, gbm->width, gbm->height,
			gbm->format, &modifier, 1);

	if(!bo) {
		if(modifier != DRM_FORMAT_MOD_LINEAR) {
			fprintf(stderr, "Modifiers requested but support isn't available\n");
			return NULL;
		}

		bo = gbm_bo_create(gbm->dev, gbm->width, gbm->height,
				gbm->format, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	}

	if(!bo) {
		fprintf(stderr, "failed to create gbm bo\n");
		return NULL;
	}

	return bo;
}

static GBM* init_surfaceless(GBM* gbm, u64 modifier)
{
	for(unsigned int i = 0; i < ARRAY_SIZE(gbm->bos); i++) {
		gbm->bos[i] = init_bo(gbm, modifier);
		if(!gbm->bos[i]) {
			return NULL;
		}
	}
	return gbm;
}

static GBM* init_surface(GBM* gbm, u64 modifier)
{
	gbm->surface = gbm_surface_create_with_modifiers(gbm->dev, gbm->width,
			gbm->height, gbm->format, &modifier, 1);

	if(!gbm->surface) {
		if(modifier != DRM_FORMAT_MOD_LINEAR) {
			fprintf(stderr, "Modifiers requested but support isn't available\n");
			return NULL;
		}

		gbm->surface = gbm_surface_create(gbm->dev, gbm->width,
				gbm->height, gbm->format,
				GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	}

	if(!gbm->surface) {
		fprintf(stderr, "failed to create gbm surface\n");
		return NULL;
	}

	return gbm;
}

GBM* GBMInit(int drm_fd, int w, int h, u32 format, u64 modifier,
		bool surfaceless)
{
	GBM* gbm = (GBM*) malloc(sizeof(GBM));

	memset(gbm, 0, sizeof(GBM));

	gbm->dev = gbm_create_device(drm_fd);
	gbm->format = format;
	gbm->surface = NULL;

	gbm->width = w;
	gbm->height = h;

	if(surfaceless) {
		return init_surfaceless(gbm, modifier);
	} else {
		return init_surface(gbm, modifier);
	}
}

void GBMDestroy(GBM* gbm)
{
	if(gbm->surface) {
		gbm_surface_destroy(gbm->surface);
	} else {
		for(unsigned int i = 0; i < ARRAY_SIZE(gbm->bos); i++) {
			gbm_bo_destroy(gbm->bos[i]);
		}
	}

	gbm_device_destroy(gbm->dev);

	free(gbm);
}
