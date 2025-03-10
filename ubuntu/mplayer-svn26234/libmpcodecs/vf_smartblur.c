/*
    Copyright (C) 2002 Michael Niedermayer <michaelni@gmx.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "config.h"
#include "mp_msg.h"

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "libavutil/avutil.h"
#include "img_format.h"
#include "mp_image.h"
#include "vf.h"
#include "libswscale/swscale.h"
#include "vf_scale.h"

//===========================================================================//

typedef struct FilterParam{
	float radius;
	float strength;
	int threshold;
	float quality;
	struct SwsContext *filterContext;
}FilterParam;

struct vf_priv_s {
	FilterParam luma;
	FilterParam chroma;
};


/***************************************************************************/

//FIXME stupid code duplication
static void getSubSampleFactors(int *h, int *v, int format){
	switch(format){
	case IMGFMT_YV12:
	case IMGFMT_I420:
		*h=1;
		*v=1;
		break;
	case IMGFMT_YVU9:
		*h=2;
		*v=2;
		break;
	case IMGFMT_444P:
		*h=0;
		*v=0;
		break;
	case IMGFMT_422P:
		*h=1;
		*v=0;
		break;
	case IMGFMT_411P:
		*h=2;
		*v=0;
		break;
	}
}

static int allocStuff(FilterParam *f, int width, int height){
	SwsVector *vec;
	SwsFilter swsF;

	vec = sws_getGaussianVec(f->radius, f->quality);
	sws_scaleVec(vec, f->strength);
	vec->coeff[vec->length/2]+= 1.0 - f->strength;
	swsF.lumH= swsF.lumV= vec;
	swsF.chrH= swsF.chrV= NULL;
	f->filterContext= sws_getContext(
		width, height, PIX_FMT_GRAY8, width, height, PIX_FMT_GRAY8, get_sws_cpuflags(), &swsF, NULL, NULL);

	sws_freeVec(vec);

	return 0;
}

static int config(struct vf_instance_s* vf,
        int width, int height, int d_width, int d_height,
	unsigned int flags, unsigned int outfmt){
	
	int sw, sh;

	allocStuff(&vf->priv->luma, width, height);
	
	getSubSampleFactors(&sw, &sh, outfmt);
	allocStuff(&vf->priv->chroma, width>>sw, height>>sh);

	return vf_next_config(vf,width,height,d_width,d_height,flags,outfmt);
}

static void freeBuffers(FilterParam *f){
	if(f->filterContext) sws_freeContext(f->filterContext);
	f->filterContext=NULL;
}

static void uninit(struct vf_instance_s* vf){
	if(!vf->priv) return;

	freeBuffers(&vf->priv->luma);
	freeBuffers(&vf->priv->chroma);

	free(vf->priv);
	vf->priv=NULL;
}

static inline void blur(uint8_t *dst, uint8_t *src, int w, int h, int dstStride, int srcStride, FilterParam *fp){
	int x, y;
	FilterParam f= *fp;
	uint8_t *srcArray[3]= {src, NULL, NULL};
	uint8_t *dstArray[3]= {dst, NULL, NULL};
	int srcStrideArray[3]= {srcStride, 0, 0};
	int dstStrideArray[3]= {dstStride, 0, 0};

	sws_scale(f.filterContext, srcArray, srcStrideArray, 0, h, dstArray, dstStrideArray);
	
	if(f.threshold > 0){
		for(y=0; y<h; y++){
			for(x=0; x<w; x++){
				const int orig= src[x + y*srcStride];
				const int filtered= dst[x + y*dstStride];
				const int diff= orig - filtered;
				
				if(diff > 0){
					if(diff > 2*f.threshold){
						dst[x + y*dstStride]= orig;
					}else if(diff > f.threshold){
						dst[x + y*dstStride]= filtered + diff - f.threshold;
					}
				}else{
					if(-diff > 2*f.threshold){
						dst[x + y*dstStride]= orig;
					}else if(-diff > f.threshold){
						dst[x + y*dstStride]= filtered + diff + f.threshold;
					}
				}
			}
		}
	}else if(f.threshold < 0){
		for(y=0; y<h; y++){
			for(x=0; x<w; x++){
				const int orig= src[x + y*srcStride];
				const int filtered= dst[x + y*dstStride];
				const int diff= orig - filtered;
				
				if(diff > 0){
					if(diff > -2*f.threshold){
					}else if(diff > -f.threshold){
						dst[x + y*dstStride]= orig - diff - f.threshold;
					}else
						dst[x + y*dstStride]= orig;
				}else{
					if(diff < 2*f.threshold){
					}else if(diff < f.threshold){
						dst[x + y*dstStride]= orig - diff + f.threshold;
					}else
						dst[x + y*dstStride]= orig;
				}
			}
		}
	}
}

static int put_image(struct vf_instance_s* vf, mp_image_t *mpi, double pts){
	int cw= mpi->w >> mpi->chroma_x_shift;
	int ch= mpi->h >> mpi->chroma_y_shift;
	FilterParam *f= &vf->priv;

	mp_image_t *dmpi=vf_get_image(vf->next,mpi->imgfmt,
		MP_IMGTYPE_TEMP, MP_IMGFLAG_ACCEPT_STRIDE|
		(f->threshold) ? MP_IMGFLAG_READABLE : 0,
		mpi->w,mpi->h);

	assert(mpi->flags&MP_IMGFLAG_PLANAR);
	
	blur(dmpi->planes[0], mpi->planes[0], mpi->w,mpi->h, dmpi->stride[0], mpi->stride[0], &vf->priv->luma);
	blur(dmpi->planes[1], mpi->planes[1], cw    , ch   , dmpi->stride[1], mpi->stride[1], &vf->priv->chroma);
	blur(dmpi->planes[2], mpi->planes[2], cw    , ch   , dmpi->stride[2], mpi->stride[2], &vf->priv->chroma);
    
	return vf_next_put_image(vf,dmpi, pts);
}

//===========================================================================//

static int query_format(struct vf_instance_s* vf, unsigned int fmt){
	switch(fmt)
	{
	case IMGFMT_YV12:
	case IMGFMT_I420:
	case IMGFMT_IYUV:
	case IMGFMT_YVU9:
	case IMGFMT_444P:
	case IMGFMT_422P:
	case IMGFMT_411P:
		return vf_next_query_format(vf, fmt);
	}
	return 0;
}

static int open(vf_instance_t *vf, char* args){
	int e;

	vf->config=config;
	vf->put_image=put_image;
//	vf->get_image=get_image;
	vf->query_format=query_format;
	vf->uninit=uninit;
	vf->priv=malloc(sizeof(struct vf_priv_s));
	memset(vf->priv, 0, sizeof(struct vf_priv_s));

	if(args==NULL) return 0;
	
	e=sscanf(args, "%f:%f:%d:%f:%f:%d",
		&vf->priv->luma.radius,
		&vf->priv->luma.strength,
		&vf->priv->luma.threshold,
		&vf->priv->chroma.radius,
		&vf->priv->chroma.strength,
		&vf->priv->chroma.threshold
		);

	vf->priv->luma.quality = vf->priv->chroma.quality= 3.0;
	
	if(e==3){
		vf->priv->chroma.radius= vf->priv->luma.radius;
		vf->priv->chroma.strength= vf->priv->luma.strength;
		vf->priv->chroma.threshold = vf->priv->luma.threshold;
	}else if(e!=6)
		return 0;

	return 1;
}

const vf_info_t vf_info_smartblur = {
    "smart blur",
    "smartblur",
    "Michael Niedermayer",
    "",
    open,
    NULL
};

//===========================================================================//
