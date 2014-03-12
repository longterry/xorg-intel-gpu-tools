#ifndef INTEL_BATCHBUFFER_H
#define INTEL_BATCHBUFFER_H

#include <assert.h>
#include "intel_bufmgr.h"

#define BATCH_SZ 4096
#define BATCH_RESERVED 16

struct intel_batchbuffer {
	drm_intel_bufmgr *bufmgr;
	uint32_t devid;

	drm_intel_bo *bo;

	uint8_t buffer[BATCH_SZ];
	uint8_t *ptr;
	uint8_t *state;
};

struct intel_batchbuffer *intel_batchbuffer_alloc(drm_intel_bufmgr *bufmgr,
						  uint32_t devid);

void intel_batchbuffer_free(struct intel_batchbuffer *batch);


void intel_batchbuffer_flush(struct intel_batchbuffer *batch);
void intel_batchbuffer_flush_on_ring(struct intel_batchbuffer *batch, int ring);
void intel_batchbuffer_flush_with_context(struct intel_batchbuffer *batch,
					  drm_intel_context *context);

void intel_batchbuffer_reset(struct intel_batchbuffer *batch);

void intel_batchbuffer_data(struct intel_batchbuffer *batch,
                            const void *data, unsigned int bytes);

void intel_batchbuffer_emit_reloc(struct intel_batchbuffer *batch,
				  drm_intel_bo *buffer,
				  uint32_t delta,
				  uint32_t read_domains,
				  uint32_t write_domain,
				  int fenced);

/* Inline functions - might actually be better off with these
 * non-inlined.  Certainly better off switching all command packets to
 * be passed as structs rather than dwords, but that's a little bit of
 * work...
 */
#pragma GCC diagnostic ignored "-Winline"
static inline unsigned int
intel_batchbuffer_space(struct intel_batchbuffer *batch)
{
	return (BATCH_SZ - BATCH_RESERVED) - (batch->ptr - batch->buffer);
}


static inline void
intel_batchbuffer_emit_dword(struct intel_batchbuffer *batch, uint32_t dword)
{
	assert(intel_batchbuffer_space(batch) >= 4);
	*(uint32_t *) (batch->ptr) = dword;
	batch->ptr += 4;
}

static inline void
intel_batchbuffer_require_space(struct intel_batchbuffer *batch,
                                unsigned int sz)
{
	assert(sz < BATCH_SZ - BATCH_RESERVED);
	if (intel_batchbuffer_space(batch) < sz)
		intel_batchbuffer_flush(batch);
}

/* Here are the crusty old macros, to be removed:
 */
#define BATCH_LOCALS

#define BEGIN_BATCH(n) do {						\
	intel_batchbuffer_require_space(batch, (n)*4);			\
} while (0)

#define OUT_BATCH(d) intel_batchbuffer_emit_dword(batch, d)

#define OUT_RELOC_FENCED(buf, read_domains, write_domain, delta) do {		\
	assert((delta) >= 0);						\
	intel_batchbuffer_emit_reloc(batch, buf, delta,			\
				     read_domains, write_domain, 1);	\
} while (0)

#define OUT_RELOC(buf, read_domains, write_domain, delta) do {		\
	assert((delta) >= 0);						\
	intel_batchbuffer_emit_reloc(batch, buf, delta,			\
				     read_domains, write_domain, 0);	\
} while (0)

#define ADVANCE_BATCH() do {						\
} while(0)

#define BLIT_COPY_BATCH_START(devid, flags) do { \
	if (intel_gen(devid) >= 8) { \
		BEGIN_BATCH(10); \
		OUT_BATCH(XY_SRC_COPY_BLT_CMD | \
				XY_SRC_COPY_BLT_WRITE_ALPHA | \
				XY_SRC_COPY_BLT_WRITE_RGB | \
				(flags) | 8); \
	} else { \
		BEGIN_BATCH(8); \
		OUT_BATCH(XY_SRC_COPY_BLT_CMD | \
				XY_SRC_COPY_BLT_WRITE_ALPHA | \
				XY_SRC_COPY_BLT_WRITE_RGB | \
				(flags) | 6); \
	} \
} while(0)

#define COLOR_BLIT_COPY_BATCH_START(devid, flags) do { \
	if (intel_gen(devid) >= 8) { \
		BEGIN_BATCH(8); \
		OUT_BATCH(MI_NOOP); \
		OUT_BATCH(XY_COLOR_BLT_CMD_NOLEN | 0x5 | \
				COLOR_BLT_WRITE_ALPHA | \
				XY_COLOR_BLT_WRITE_RGB); \
	} else { \
		BEGIN_BATCH(6); \
		OUT_BATCH(XY_COLOR_BLT_CMD_NOLEN | 0x4 | \
				COLOR_BLT_WRITE_ALPHA | \
				XY_COLOR_BLT_WRITE_RGB); \
	} \
} while(0)

#define BLIT_RELOC_UDW(devid) do { \
	if (intel_gen(devid) >= 8) { \
		OUT_BATCH(0); \
	} \
} while(0)

void
intel_blt_copy(struct intel_batchbuffer *batch,
	      drm_intel_bo *src_bo, int src_x1, int src_y1, int src_pitch,
	      drm_intel_bo *dst_bo, int dst_x1, int dst_y1, int dst_pitch,
	      int width, int height, int bpp);
void intel_copy_bo(struct intel_batchbuffer *batch,
		   drm_intel_bo *dst_bo, drm_intel_bo *src_bo,
		   int width, int height);

#endif
