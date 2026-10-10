#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "types.h"
#include "kms.h"

#define WEAK __attribute__((weak))

struct drm_fb {
	struct gbm_bo*		bo;
	u32			fb_id;
};

WEAK union gbm_bo_handle gbm_bo_get_handle_for_plane(struct gbm_bo *bo,
				int plane);
WEAK u64	gbm_bo_get_modifier(struct gbm_bo *bo);
WEAK int	gbm_bo_get_plane_count(struct gbm_bo *bo);
WEAK u32	gbm_bo_get_stride_for_plane(struct gbm_bo *bo, int plane);
WEAK u32	gbm_bo_get_offset(struct gbm_bo *bo, int plane);

static int add_property(DRMModeInfo* obj, drmModeAtomicReq* req, u32 obj_id,
		const char* name, u64 value)
{
	unsigned int i;
	int prop_id = -1;

	for(i = 0; i < obj->props->count_props; i++) {
		if(!strcmp(obj->props_info[i]->name, name)) {
			prop_id = obj->props_info[i]->prop_id;
			break;
		}
	}

	if(prop_id < 0) {
		fprintf(stderr, "No property: %s\n", name);
		return -EINVAL;
	}

	return drmModeAtomicAddProperty(req, obj_id, prop_id, value);
}

static int drm_atomic_commit(DRM* drm, u32 fb_id, u32 flags)
{
	drmModeAtomicReq* req;
	u32 plane_id = drm->plane->plane->plane_id;
	u32 blob_id;
	int ret;

	req = drmModeAtomicAlloc();

	if(flags & DRM_MODE_ATOMIC_ALLOW_MODESET) {
		if(add_property(&drm->connector->info, req, drm->connector_id,
					"CRTC_ID", drm->crtc_id) < 0)
			return -1;
		if(drmModeCreatePropertyBlob(drm->fd, drm->mode,
					sizeof(*drm->mode), &blob_id))
			return -1;

		if(add_property(&drm->crtc->info, req, drm->crtc_id, "MODE_ID",
					blob_id) < 0)
			return -1;
		if(add_property(&drm->crtc->info, req, drm->crtc_id, "ACTIVE",
					1) < 0)
			return -1;
	}

	add_property(&drm->plane->info, req, plane_id, "FB_ID", fb_id);
	add_property(&drm->plane->info, req, plane_id, "CRTC_ID", drm->crtc_id);
	add_property(&drm->plane->info, req, plane_id, "SRC_X", 0);
	add_property(&drm->plane->info, req, plane_id, "SRC_Y", 0);
	add_property(&drm->plane->info, req, plane_id, "SRC_W", drm->mode->hdisplay << 16);
	add_property(&drm->plane->info, req, plane_id, "SRC_H", drm->mode->vdisplay << 16);
	add_property(&drm->plane->info, req, plane_id, "CRTC_X", 0);
	add_property(&drm->plane->info, req, plane_id, "CRTC_Y", 0);
	add_property(&drm->plane->info, req, plane_id, "CRTC_W", drm->mode->hdisplay);
	add_property(&drm->plane->info, req, plane_id, "CRTC_H", drm->mode->vdisplay);

	if(drm->kms_in_fence_fd != -1) {
		add_property(&drm->crtc->info, req, drm->crtc_id,
				"OUT_FENCE_PTR", (u64) &drm->kms_out_fence_fd);
		add_property(&drm->plane->info, req, plane_id, "IN_FENCE_FD",
				drm->kms_in_fence_fd);
	}

	ret = drmModeAtomicCommit(drm->fd, req, flags, NULL);

	if(ret) {
		goto out;
	}

	if(drm->kms_in_fence_fd != -1) {
		close(drm->kms_in_fence_fd);
		drm->kms_in_fence_fd = -1;
	}

out:
	drmModeAtomicFree(req);

	return ret;
}

/* Pick a plane ... something that at a minimum can be connected to the chosen
 * crtc, but prefer primary plane.
 */
static int get_plane_id(DRM* drm)
{
	drmModePlaneResPtr plane_resources;
	u32 i, j;
	int ret = -EINVAL;
	bool found_primary = false;

	plane_resources = drmModeGetPlaneResources(drm->fd);
	if(!plane_resources) {
		fprintf(stderr, "drmModeGetPlaneResources failed: %s\n",
				strerror(errno));
		return -1;
	}

	for(i = 0; (i < plane_resources->count_planes) && !found_primary; i++) {
		u32 id = plane_resources->planes[i];
		drmModePlanePtr plane = drmModeGetPlane(drm->fd, id);
		if(!plane) {
			fprintf(stderr, "drmModeGetPlane(%u) failed: %s\n", id,
					strerror(errno));
			return -1;
		}

		if(plane->possible_crtcs & (1 << drm->crtc_index)) {
			drmModeObjectPropertiesPtr props =
				drmModeObjectGetProperties(drm->fd, id,
						DRM_MODE_OBJECT_PLANE);

			/* primary or not, this plane is good enough to use */
			ret = id;

			for(j = 0; j < props->count_props; j++) {
				drmModePropertyPtr p = drmModeGetProperty(drm->fd,
						props->props[j]);

				if(!strcmp(p->name, "type") &&
						props->prop_values[j] == DRM_PLANE_TYPE_PRIMARY) {
					/* found our primary plane, let's use
					 * that */
					found_primary = 1;
				}

				drmModeFreeProperty(p);
			}

			drmModeFreeObjectProperties(props);
		}

		drmModeFreePlane(plane);
	}

	drmModeFreePlaneResources(plane_resources);

	return ret;
}

static u32 find_crtc_for_encoder(const drmModeRes* resources,
		const drmModeEncoder* encoder)
{
	int i;

	for(i = 0; i < resources->count_crtcs; i++) {
		/* possible_crtcs is a bitmask as described here:
		 * https://dvdhrm.wordpress.com/2012/09/13/linux-drm-mode-setting-api
		 */
		const u32 crtc_mask = 1 << i;
		const u32 crtc_id = resources->crtcs[i];
		if(encoder->possible_crtcs & crtc_mask) {
			return crtc_id;
		}
	}

	/* no match found */
	return -1;
}

static u32 find_crtc_for_connector(const DRM* drm, const drmModeRes* resources,
		const drmModeConnector* connector)
{
	int i;

	for(i = 0; i < connector->count_encoders; i++) {
		const u32 encoder_id = connector->encoders[i];
		drmModeEncoder* encoder = drmModeGetEncoder(drm->fd, encoder_id);

		if(encoder) {
			const u32 crtc_id = find_crtc_for_encoder(resources,
					encoder);

			drmModeFreeEncoder(encoder);
			if(crtc_id) {
				return crtc_id;
			}
		}
	}

	/* no match found */
	return -1;
}

static int get_resources(int fd, drmModeRes** resources)
{
	*resources = drmModeGetResources(fd);
	if(!*resources) {
		return -1;
	}
	return 0;
}

#define MAX_DRM_DEVICES 64

static int find_drm_device(drmModeRes** resources)
{
	drmDevicePtr devices[MAX_DRM_DEVICES] = { NULL };
	int num_devices, fd = -1, i;

	num_devices = drmGetDevices2(0, devices, MAX_DRM_DEVICES);
	if(num_devices < 0) {
		fprintf(stderr, "drmGetDevices2 failed: %s\n",
				strerror(-num_devices));
		return -1;
	}

	for(i = 0; i < num_devices; i++) {
		drmDevicePtr device = devices[i];
		int ret;

		if(!(device->available_nodes & (1 << DRM_NODE_PRIMARY))) {
			continue;
		}

		/* OK, it's a primary device. If we can get the
		 * drmModeResources, it means it's also a KMS-capable device.
		 */
		fd = open(device->nodes[DRM_NODE_PRIMARY], O_RDWR);
		if(fd < 0) {
			continue;
		}
		ret = get_resources(fd, resources);
#ifdef DEBUG
		printf("using drm device %s\n", device->nodes[DRM_NODE_PRIMARY]);
#endif
		if(!ret) {
			break;
		}
		close(fd);
		fd = -1;
	}

	drmFreeDevices(devices, num_devices);

	if(fd < 0)
		fprintf(stderr, "no drm device found!\n");

	return fd;
}

static int init_drm(DRM* drm, const char* device, const char* mode_str,
		unsigned int vrefresh)
{
	drmModeRes* resources;
	drmModeConnector* connector = NULL;
	drmModeEncoder* encoder = NULL;
	int i, ret, area;

	if(device) {
		drm->fd = open(device, O_RDWR);
		ret = get_resources(drm->fd, &resources);
		if(ret < 0 && errno == EOPNOTSUPP)
			fprintf(stderr, "%s does not look like a modeset device\n",
					device);
	} else {
		drm->fd = find_drm_device(&resources);
	}

	if(drm->fd < 0) {
		fprintf(stderr, "could not open drm device\n");
		return -1;
	}

	if(!resources) {
		fprintf(stderr, "drmModeGetResources failed: %s\n",
				strerror(errno));
		return -1;
	}

	/* find a connected connector */
	for(i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(drm->fd,
				resources->connectors[i]);
		if(connector->connection == DRM_MODE_CONNECTED) {
			/* it's connected, let's use this! */
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}

	if(!connector) {
		/* we could be fancy and listen for hotplug events and wait for
		 * a connector ...
		 */
		fprintf(stderr, "no connected connector!\n");
		return -1;
	}

	/* find user requested mode */
	if(mode_str && *mode_str) {
		for(i = 0; i < connector->count_modes; i++) {
			drmModeModeInfo* current_mode = &connector->modes[i];
			if(!strcmp(current_mode->name, mode_str)) {
				if(vrefresh == 0 || current_mode->vrefresh == vrefresh) {
					drm->mode = current_mode;
					break;
				}
			}
		}
		if(!drm->mode)
			fprintf(stderr, "requested mode not found, using default mode!\n");
	}

	/* find preferred mode or the highest resolution mode */
	if(!drm->mode) {
		for(i = 0, area = 0; i < connector->count_modes; i++) {
			int current_area;
			drmModeModeInfo* current_mode = &connector->modes[i];
			if(current_mode->type & DRM_MODE_TYPE_PREFERRED) {
				drm->mode = current_mode;
				break;
			}

			current_area = current_mode->hdisplay *
				current_mode->vdisplay;
			if(current_area > area) {
				drm->mode = current_mode;
				area = current_area;
			}
		}
	}

	if(!drm->mode) {
		fprintf(stderr, "could not find mode!\n");
		return -1;
	}

	/* find encoder */
	for(i = 0; i < resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(drm->fd, resources->encoders[i]);
		if(encoder->encoder_id == connector->encoder_id)
			break;
		drmModeFreeEncoder(encoder);
		encoder = NULL;
	}

	if(encoder) {
		drm->crtc_id = encoder->crtc_id;
	} else {
		u32 crtc_id = find_crtc_for_connector(drm, resources, connector);
		if(!crtc_id) {
			fprintf(stderr, "no crtc found!\n");
			return -1;
		}

		drm->crtc_id = crtc_id;
	}

	for(i = 0; i < resources->count_crtcs; i++) {
		if(resources->crtcs[i] == drm->crtc_id) {
			drm->crtc_index = i;
			break;
		}
	}

	drmModeFreeResources(resources);

	drm->connector_id = connector->connector_id;

	drm->con = connector;
	drm->encoder = encoder;

	return 0;
}

DRM* DRMInit(const char* device, const char* mode_str,
		unsigned int vrefresh)
{
	u32 plane_id;
	int ret;

	DRM* drm = (DRM*) malloc(sizeof(DRM));
	memset(drm, 0, sizeof(DRM));

	drm->kms_out_fence_fd = -1;

	ret = init_drm(drm, device, mode_str, vrefresh);
	if(ret)
		return NULL;

	ret = drmSetClientCap(drm->fd, DRM_CLIENT_CAP_ATOMIC, 1);
	if(ret) {
		fprintf(stderr, "no atomic modesetting support: %s\n",
				strerror(errno));
		return NULL;
	}

	ret = get_plane_id(drm);
	if(!ret) {
		fprintf(stderr, "could not find a suitable plane\n");
		return NULL;
	} else {
		plane_id = ret;
	}

	/* We only do single plane to single crtc to single connector, no fancy
	 * multi-monitor or multi-plane stuff. So just grab the
	 * plane/crtc/connector property info for one of each
	 */
	drm->plane = (Plane*) malloc(sizeof(Plane));
	drm->crtc = (CRTC*) malloc(sizeof(CRTC));
	drm->connector = (Connector*) malloc(sizeof(Connector));

#define get_resource(type, Type, id) {					       \
		drm->type->type = drmModeGet##Type(drm->fd, id);	       \
		if(!drm->type->type) {					       \
			fprintf(stderr, "could not get " #type " %i: %s\n",    \
					id, strerror(errno));		       \
			return NULL;					       \
		}							       \
	}

	get_resource(plane, Plane, plane_id);
	get_resource(crtc, Crtc, drm->crtc_id);
	get_resource(connector, Connector, drm->connector_id);

#define get_properties(type, TYPE, id) {				       \
		u32 i;							       \
		drm->type->info.props = drmModeObjectGetProperties(drm->fd,    \
				id, DRM_MODE_OBJECT_##TYPE);		       \
		if(!drm->type->info.props) {				       \
			fprintf(stderr, "could not get " #type		       \
					" %u properties: %s\n", id,	       \
					strerror(errno));		       \
			return NULL;					       \
		}							       \
		drm->type->info.props_info = calloc(drm->type->info.props->count_props, \
				sizeof(*drm->type->info.props_info));	       \
		for(i = 0; i < drm->type->info.props->count_props; i++) {      \
			drm->type->info.props_info[i] = drmModeGetProperty(drm->fd, \
					drm->type->info.props->props[i]);      \
		}							       \
	}

	get_properties(plane, PLANE, plane_id);
	get_properties(crtc, CRTC, drm->crtc_id);
	get_properties(connector, CONNECTOR, drm->connector_id);

	return drm;
}

void DRMDestroy(DRM* drm)
{
#define free_properties(type) {						       \
		unsigned int i;						       \
		for(i = 0; i < drm->type->info.props->count_props; i++)	       \
			drmModeFreeProperty(drm->type->info.props_info[i]);    \
		drmModeFreeObjectProperties(drm->type->info.props);	       \
	}

	free_properties(connector);
	free_properties(crtc);
	free_properties(plane);

	drmModeFreeEncoder(drm->encoder);
	drmModeFreeConnector(drm->con);

	drmModeFreeConnector(drm->connector->connector);
	drmModeFreeCrtc(drm->crtc->crtc);
	drmModeFreePlane(drm->plane->plane);

	free(drm->plane->info.props_info);
	free(drm->crtc->info.props_info);
	free(drm->connector->info.props_info);

	free(drm->connector);
	free(drm->crtc);
	free(drm->plane);

	if(drm->fd >= 0)
		close(drm->fd);

	free(drm);
}

static void
drm_fb_destroy_callback(struct gbm_bo *bo, void *data)
{
	int drm_fd = gbm_device_get_fd(gbm_bo_get_device(bo));
	struct drm_fb *fb = data;

	if(fb->fb_id)
		drmModeRmFB(drm_fd, fb->fb_id);

	free(fb);
}

struct drm_fb* drm_fb_get_from_bo(struct gbm_bo* bo)
{
	int drm_fd = gbm_device_get_fd(gbm_bo_get_device(bo));	
	struct drm_fb* fb = (struct drm_fb*) gbm_bo_get_user_data(bo);

	u32 width;
	u32 height;
	u32 format;
	u32 strides[4] = { 0 };
	u32 handles[4] = { 0 };
	u32 offsets[4] = { 0 };
	u32 flags = 0;

	int ret = -1;

	if(fb)
		return fb;

	fb = calloc(1, sizeof(struct drm_fb));
	fb->bo = bo;

	width = gbm_bo_get_width(bo);
	height = gbm_bo_get_height(bo);
	format = gbm_bo_get_format(bo);

	if(gbm_bo_get_handle_for_plane && gbm_bo_get_modifier &&
			gbm_bo_get_plane_count && gbm_bo_get_stride_for_plane &&
			gbm_bo_get_offset) {
		u64 modifiers[4] = { 0 };
		modifiers[0] = gbm_bo_get_modifier(bo);
		const int num_planes = gbm_bo_get_plane_count(bo);
		assert(num_planes <= 4);
		for(int i = 0; i < num_planes; i++) {
			handles[i] = gbm_bo_get_handle_for_plane(bo, i).u32;
			strides[i] = gbm_bo_get_stride_for_plane(bo, i);
			offsets[i] = gbm_bo_get_offset(bo, i);
			modifiers[i] = modifiers[0];
		}

		if(modifiers[0]) {
			flags = DRM_MODE_FB_MODIFIERS;
#ifdef DEBUG
			printf("Using modifier %" PRIx64 "\n", modifiers[0]);
#endif
		}

		ret = drmModeAddFB2WithModifiers(drm_fd, width, height, format,
				handles, strides, offsets, modifiers,
				&fb->fb_id, flags);
	}

	if(ret) {
		if(flags) {
			printf("Modifiers failed!\n");
		}

		memset(handles, 0, sizeof(u32) * 4);
		memset(strides, 0, sizeof(u32) * 4);
		memset(offsets, 0, sizeof(u32) * 4);

		handles[0] = gbm_bo_get_handle(bo).u32;
		strides[0] = gbm_bo_get_stride(bo);

		ret = drmModeAddFB2(drm_fd, width, height, format, handles,
				strides, offsets, &fb->fb_id, 0);
	}

	if(ret) {
		printf("failed to create fb: %s\n", strerror(errno));
		free(fb);

		return NULL;
	}

	gbm_bo_set_user_data(bo, fb, drm_fb_destroy_callback);

	return fb;
}

static EGLSyncKHR create_fence(const EGL* egl, int fd)
{
	EGLint attrib_list[] = {
		EGL_SYNC_NATIVE_FENCE_FD_ANDROID, fd,
		EGL_NONE
	};

	EGLSyncKHR fence = eglCreateSyncKHR(egl->display,
			EGL_SYNC_NATIVE_FENCE_ANDROID, attrib_list);

	assert(fence);
	return fence;
}

void EGLBegin(EGL* egl)
{
	const GBM* gbm = egl->gbm;
	DRM* drm= egl->drm;

	if(drm->kms_out_fence_fd != -1) {
		egl->kms_fence = create_fence(egl, drm->kms_out_fence_fd);
		assert(egl->kms_fence);

		/* driver now has ownership of the fence fd */
		drm->kms_out_fence_fd = -1;

		/* wait "on the gpu" (ie. this won't necessarily block, but will
		 * block the rendering until fence is signaled), until the
		 * previous pageflip completes so we don't render into the
		 * buffer that is still on screen.
		 */
		eglWaitSyncKHR(egl->display, egl->kms_fence, 0);
	}

	if(!gbm->surface) {
		glBindFramebuffer(GL_FRAMEBUFFER, egl->fbs[egl->fb].fb);
	}
}

bool EGLEnd(EGL* egl)
{
	int result;
	struct gbm_bo* next_bo;
	struct drm_fb* fb;
	const GBM* gbm = egl->gbm;
	DRM* drm= egl->drm;
	EGLSyncKHR gpu_fence = NULL; /* out-fence from gpu, in-fence to kms */

	/* insert fence to be singled in cmdstream, this fence will be signaled
	 * when gpu rendering is done
	 */
	gpu_fence = create_fence(egl, EGL_NO_NATIVE_FENCE_FD_ANDROID);
	assert(gpu_fence);

	if(gbm->surface) {
		eglSwapBuffers(egl->display, egl->surface);
	}

	/* after swapbuffers, gpu_fence should be flushed, so safe to get fd */
	drm->kms_in_fence_fd = eglDupNativeFenceFDANDROID(egl->display, gpu_fence);
	eglDestroySyncKHR(egl->display, gpu_fence);
	assert(drm->kms_in_fence_fd != -1);

	if(gbm->surface) {
		next_bo = gbm_surface_lock_front_buffer(gbm->surface);
	} else {
		next_bo = gbm->bos[egl->fb];
	}

	if(!next_bo) {
		printf("Failed to lock frontbuffer\n");
		return false;
	}

	fb = drm_fb_get_from_bo(next_bo);
	if(!fb) {
		printf("Failed to get a new framebuffer BO\n");
		return false;
	}

	if(egl->kms_fence) {
		EGLint status;

		/* Wait on the CPU side for the _previous_ commit to complete
		 * before we post the flip through KMS, as atomic will reject
		 * the commit if we post a new one whilst the previous one is
		 * still pending
		 */
		do {
			status = eglClientWaitSyncKHR(egl->display,
					egl->kms_fence, 0, EGL_FOREVER_KHR);
		} while(status != EGL_CONDITION_SATISFIED_KHR);

		eglDestroySyncKHR(egl->display, egl->kms_fence);
	}

	/* Here you could also update DRM plane layers if you want HW
	 * composition
	 */
	result = drm_atomic_commit(drm, fb->fb_id, egl->flags);
	if(result) {
		printf("Failed to commit: %s\n", strerror(errno));
		return false;
	}

	/* release last buffer to render on again */
	if(egl->bo && gbm->surface) {
		gbm_surface_release_buffer(gbm->surface, egl->bo);
	}

	egl->bo = next_bo;

	/* Allow a modeset change for the first commit only */
	egl->flags &= ~DRM_MODE_ATOMIC_ALLOW_MODESET;

	egl->fb = (egl->fb + 1) % NUM_BUFFERS;

	return true;
}
