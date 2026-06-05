/* SPDX-License-Identifier: GPL-2.0 */
/*
 * linuxab/drivers/gpu/drm.h
 * Simplified Direct Rendering Manager
 */

#ifndef _LINUXAB_DRM_H
#define _LINUXAB_DRM_H

#include <stdint.h>
#include <stdbool.h>

#define DRM_MODE_CONNECTED      1
#define DRM_MODE_DISCONNECTED   2

#define DRM_MODE_TYPE_BUILTIN   (1<<0)
#define DRM_MODE_TYPE_CLOCK_C   (1<<1)
#define DRM_MODE_TYPE_CRTC_C    (1<<2)
#define DRM_MODE_TYPE_PREFERRED (1<<3)
#define DRM_MODE_TYPE_DEFAULT   (1<<4)
#define DRM_MODE_TYPE_USERDEF   (1<<5)
#define DRM_MODE_TYPE_DRIVER    (1<<6)

#define DRM_MODE_FLAG_PHSYNC    (1<<0)
#define DRM_MODE_FLAG_NHSYNC    (1<<1)
#define DRM_MODE_FLAG_PVSYNC    (1<<2)
#define DRM_MODE_FLAG_NVSYNC    (1<<3)
#define DRM_MODE_FLAG_INTERLACE (1<<4)
#define DRM_MODE_FLAG_DBLSCAN   (1<<5)
#define DRM_MODE_FLAG_CSYNC     (1<<6)
#define DRM_MODE_FLAG_PCSYNC    (1<<7)
#define DRM_MODE_FLAG_NCSYNC    (1<<8)
#define DRM_MODE_FLAG_HSKEW     (1<<9)
#define DRM_MODE_FLAG_BCAST     (1<<10)
#define DRM_MODE_FLAG_PIXMUX    (1<<11)
#define DRM_MODE_FLAG_DBLCLK    (1<<12)
#define DRM_MODE_FLAG_CLKDIV2   (1<<13)

struct drm_display_mode {
    char    name[32];
    uint32_t clock;         /* kHz */
    uint16_t hdisplay;
    uint16_t hsync_start;
    uint16_t hsync_end;
    uint16_t htotal;
    uint16_t vdisplay;
    uint16_t vsync_start;
    uint16_t vsync_end;
    uint16_t vtotal;
    uint16_t flags;
    uint32_t type;
    int32_t  hskew;
};

struct drm_framebuffer {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t depth;
    uint64_t base;
    uint32_t size;
    uint32_t obj_id;
};

struct drm_crtc {
    bool enabled;
    struct drm_display_mode *mode;
    struct drm_framebuffer *fb;
    uint32_t x, y;
};

struct drm_connector {
    uint32_t connector_type;
    uint32_t connector_id;
    uint32_t connection;
    struct drm_display_mode *modes;
    uint32_t num_modes;
    struct drm_crtc *crtc;
};

struct drm_device {
    bool initialized;
    uint64_t mmio_base;
    uint32_t num_crtcs;
    uint32_t num_connectors;
    struct drm_crtc crtcs[4];
    struct drm_connector connectors[4];
    struct drm_framebuffer *fb;
    void (*set_mode)(struct drm_crtc *crtc, struct drm_display_mode *mode);
    void (*fb_set)(struct drm_crtc *crtc, struct drm_framebuffer *fb);
};

int drm_init(struct drm_device *dev);
int drm_mode_set(struct drm_device *dev, uint32_t crtc_id,
                 struct drm_display_mode *mode,
                 struct drm_framebuffer *fb);
int drm_framebuffer_init(struct drm_device *dev, struct drm_framebuffer *fb,
                         uint32_t width, uint32_t height, uint32_t bpp);
void drm_framebuffer_clear(struct drm_framebuffer *fb, uint32_t color);

/* Simple EDID parser */
int drm_edid_parse(uint8_t *edid, struct drm_display_mode *modes, int max_modes);

#endif /* _LINUXAB_DRM_H */
