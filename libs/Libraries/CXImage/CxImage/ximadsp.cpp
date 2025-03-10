// xImaDsp.cpp : DSP functions
/* 07/08/2001 v1.00 - ing.davide.pizzolato@libero.it
 * CxImage version 5.00 23/Aug/2002
 */

#include "ximage.h"

#if CXIMAGE_SUPPORT_DSP

////////////////////////////////////////////////////////////////////////////////
bool CxImage::Threshold(BYTE level)
{
	if (!pDib) return false;
	if (head.biBitCount == 1) return true;

	GrayScale();

	CxImage tmp(head.biWidth,head.biHeight,1);

	for (long y=0;y<head.biHeight;y++){
		info.nProgress = (long)(100*y/head.biHeight);
		if (info.nEscape) break;
		for (long x=0;x<head.biWidth;x++){
			if (GetPixelIndex(x,y)>level)
				tmp.SetPixelIndex(x,y,1);
			else
				tmp.SetPixelIndex(x,y,0);
		}
	}
	tmp.SetPaletteIndex(0,0,0,0);
	tmp.SetPaletteIndex(1,255,255,255);
	Transfer(tmp);
	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::SplitRGB(CxImage* r,CxImage* g,CxImage* b)
{
	if (!pDib) return false;
	if (r==NULL && g==NULL && b==NULL) return false;

	CxImage tmpr(head.biWidth,head.biHeight,8);
	CxImage tmpg(head.biWidth,head.biHeight,8);
	CxImage tmpb(head.biWidth,head.biHeight,8);

	RGBQUAD color;
	for(long y=0; y<head.biHeight; y++){
		for(long x=0; x<head.biWidth; x++){
			color = GetPixelColor(x,y);
			if (r) tmpr.SetPixelIndex(x,y,color.rgbRed);
			if (g) tmpg.SetPixelIndex(x,y,color.rgbGreen);
			if (b) tmpb.SetPixelIndex(x,y,color.rgbBlue);
		}
	}

	if (r) tmpr.SetGrayPalette();
	if (g) tmpg.SetGrayPalette();
	if (b) tmpb.SetGrayPalette();

	/*for(long j=0; j<256; j++){
		BYTE i=(BYTE)j;
		if (r) tmpr.SetPaletteIndex(i,i,0,0);
		if (g) tmpg.SetPaletteIndex(i,0,i,0);
		if (b) tmpb.SetPaletteIndex(i,0,0,i);
	}*/

	if (r) r->Transfer(tmpr);
	if (g) g->Transfer(tmpg);
	if (b) b->Transfer(tmpb);

	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::SplitYUV(CxImage* y,CxImage* u,CxImage* v)
{
	if (!pDib) return false;
	if (y==NULL && u==NULL && v==NULL) return false;

	CxImage tmpy(head.biWidth,head.biHeight,8);
	CxImage tmpu(head.biWidth,head.biHeight,8);
	CxImage tmpv(head.biWidth,head.biHeight,8);

	RGBQUAD color;
	for(long yy=0; yy<head.biHeight; yy++){
		for(long x=0; x<head.biWidth; x++){
			color = RGBtoYUV(GetPixelColor(x,yy));
			if (y) tmpy.SetPixelIndex(x,yy,color.rgbRed);
			if (u) tmpu.SetPixelIndex(x,yy,color.rgbGreen);
			if (v) tmpv.SetPixelIndex(x,yy,color.rgbBlue);
		}
	}

	if (y) tmpy.SetGrayPalette();
	if (u) tmpu.SetGrayPalette();
	if (v) tmpv.SetGrayPalette();

	if (y) y->Transfer(tmpy);
	if (u) u->Transfer(tmpu);
	if (v) v->Transfer(tmpv);

	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::SplitYIQ(CxImage* y,CxImage* i,CxImage* q)
{
	if (!pDib) return false;
	if (y==NULL && i==NULL && q==NULL) return false;

	CxImage tmpy(head.biWidth,head.biHeight,8);
	CxImage tmpi(head.biWidth,head.biHeight,8);
	CxImage tmpq(head.biWidth,head.biHeight,8);

	RGBQUAD color;
	for(long yy=0; yy<head.biHeight; yy++){
		for(long x=0; x<head.biWidth; x++){
			color = RGBtoYIQ(GetPixelColor(x,yy));
			if (y) tmpy.SetPixelIndex(x,yy,color.rgbRed);
			if (i) tmpi.SetPixelIndex(x,yy,color.rgbGreen);
			if (q) tmpq.SetPixelIndex(x,yy,color.rgbBlue);
		}
	}

	if (y) tmpy.SetGrayPalette();
	if (i) tmpi.SetGrayPalette();
	if (q) tmpq.SetGrayPalette();

	if (y) y->Transfer(tmpy);
	if (i) i->Transfer(tmpi);
	if (q) q->Transfer(tmpq);

	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::SplitXYZ(CxImage* x,CxImage* y,CxImage* z)
{
	if (!pDib) return false;
	if (x==NULL && y==NULL && z==NULL) return false;

	CxImage tmpx(head.biWidth,head.biHeight,8);
	CxImage tmpy(head.biWidth,head.biHeight,8);
	CxImage tmpz(head.biWidth,head.biHeight,8);

	RGBQUAD color;
	for(long yy=0; yy<head.biHeight; yy++){
		for(long x=0; x<head.biWidth; x++){
			color = RGBtoXYZ(GetPixelColor(x,yy));
			if (x) tmpx.SetPixelIndex(x,yy,color.rgbRed);
			if (y) tmpy.SetPixelIndex(x,yy,color.rgbGreen);
			if (z) tmpz.SetPixelIndex(x,yy,color.rgbBlue);
		}
	}

	if (x) tmpx.SetGrayPalette();
	if (y) tmpy.SetGrayPalette();
	if (z) tmpz.SetGrayPalette();

	if (x) x->Transfer(tmpx);
	if (y) y->Transfer(tmpy);
	if (z) z->Transfer(tmpz);

	return true;
}////////////////////////////////////////////////////////////////////////////////
bool CxImage::SplitHSL(CxImage* h,CxImage* s,CxImage* l)
{
	if (!pDib) return false;
	if (h==NULL && s==NULL && l==NULL) return false;

	CxImage tmph(head.biWidth,head.biHeight,8);
	CxImage tmps(head.biWidth,head.biHeight,8);
	CxImage tmpl(head.biWidth,head.biHeight,8);

	RGBQUAD color;
	for(long y=0; y<head.biHeight; y++){
		for(long x=0; x<head.biWidth; x++){
			color = RGBtoHSL(GetPixelColor(x,y));
			if (h) tmph.SetPixelIndex(x,y,color.rgbRed);
			if (s) tmps.SetPixelIndex(x,y,color.rgbGreen);
			if (l) tmpl.SetPixelIndex(x,y,color.rgbBlue);
		}
	}

	if (h) tmph.SetGrayPalette();
	if (s) tmps.SetGrayPalette();
	if (l) tmpl.SetGrayPalette();

	/* pseudo-color generator for hue channel (visual debug)
	if (h) for(long j=0; j<256; j++){
		BYTE i=(BYTE)j;
		RGBQUAD hsl={120,240,i,0};
		tmph.SetPaletteIndex(i,HSLtoRGB(hsl));
	}*/

	if (h) h->Transfer(tmph);
	if (s) s->Transfer(tmps);
	if (l) l->Transfer(tmpl);

	return true;
}
////////////////////////////////////////////////////////////////////////////////
#define  HSLMAX   255	/* H,L, and S vary over 0-HSLMAX */
#define  RGBMAX   255   /* R,G, and B vary over 0-RGBMAX */
                        /* HSLMAX BEST IF DIVISIBLE BY 6 */
                        /* RGBMAX, HSLMAX must each fit in a BYTE. */
/* Hue is undefined if Saturation is 0 (grey-scale) */
/* This value determines where the Hue scrollbar is */
/* initially set for achromatic colors */
#define UNDEFINED (HSLMAX*2/3)
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::RGBtoHSL(RGBQUAD lRGBColor)
{
	BYTE R,G,B;					/* input RGB values */
	BYTE H,L,S;					/* output HSL values */
	BYTE cMax,cMin;				/* max and min RGB values */
	WORD Rdelta,Gdelta,Bdelta;	/* intermediate value: % of spread from max*/

	R = lRGBColor.rgbRed;	/* get R, G, and B out of DWORD */
	G = lRGBColor.rgbGreen;
	B = lRGBColor.rgbBlue;

	cMax = max( max(R,G), B);	/* calculate lightness */
	cMin = min( min(R,G), B);
	L = (BYTE)((((cMax+cMin)*HSLMAX)+RGBMAX)/(2*RGBMAX));

	if (cMax==cMin){			/* r=g=b --> achromatic case */
		S = 0;					/* saturation */
		H = UNDEFINED;			/* hue */
	} else {					/* chromatic case */
		if (L <= (HSLMAX/2))	/* saturation */
			S = (BYTE)((((cMax-cMin)*HSLMAX)+((cMax+cMin)/2))/(cMax+cMin));
		else
			S = (BYTE)((((cMax-cMin)*HSLMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin));
		/* hue */
		Rdelta = (WORD)((((cMax-R)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Gdelta = (WORD)((((cMax-G)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Bdelta = (WORD)((((cMax-B)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));

		if (R == cMax)
			H = (BYTE)(Bdelta - Gdelta);
		else if (G == cMax)
			H = (BYTE)((HSLMAX/3) + Rdelta - Bdelta);
		else /* B == cMax */
			H = (BYTE)(((2*HSLMAX)/3) + Gdelta - Rdelta);

//		if (H < 0) H += HSLMAX;     //always false
		if (H > HSLMAX) H -= HSLMAX;
	}
	RGBQUAD hsl={L,S,H,0};
	return hsl;
}
////////////////////////////////////////////////////////////////////////////////
float CxImage::HueToRGB(float n1,float n2, float hue)
{
	//<F. Livraghi> fixed implementation for HSL2RGB routine
	float rValue;

	if (hue > 360)
		hue = hue - 360;
	else if (hue < 0)
		hue = hue + 360;

	if (hue < 60)
		rValue = n1 + (n2-n1)*hue/60.0f;
	else if (hue < 180)
		rValue = n2;
	else if (hue < 240)
		rValue = n1+(n2-n1)*(240-hue)/60;
	else
		rValue = n1;

	return rValue;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::HSLtoRGB(COLORREF cHSLColor)
{
	return HSLtoRGB(RGBtoRGBQUAD(cHSLColor));
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::HSLtoRGB(RGBQUAD lHSLColor)
{ 
	//<F. Livraghi> fixed implementation for HSL2RGB routine
	float h,s,l;
	float m1,m2;
	BYTE r,g,b;

	h = (float)lHSLColor.rgbRed * 360.0f/255.0f;
	s = (float)lHSLColor.rgbGreen/255.0f;
	l = (float)lHSLColor.rgbBlue/255.0f;

	if (l <= 0.5)	m2 = l * (1+s);
	else			m2 = l + s - l*s;

	m1 = 2 * l - m2;

	if (s == 0) {
		r=g=b=(BYTE)(l*255.0f);
	} else {
		r = (BYTE)(HueToRGB(m1,m2,h+120) * 255.0f);
		g = (BYTE)(HueToRGB(m1,m2,h) * 255.0f);
		b = (BYTE)(HueToRGB(m1,m2,h-120) * 255.0f);
	}

	RGBQUAD rgb = {b,g,r,0};
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::YUVtoRGB(RGBQUAD lYUVColor)
{
	int Y,U,V,R,G,B;
	Y = lYUVColor.rgbRed;
	U = lYUVColor.rgbGreen - 128;
	V = lYUVColor.rgbBlue - 128;

//	R = (int)(1.164 * Y + 2.018 * U);
//	G = (int)(1.164 * Y - 0.813 * V - 0.391 * U);
//	B = (int)(1.164 * Y + 1.596 * V);
	R = (int)( Y + 1.403 * V);
	G = (int)(Y - 0.344 * U - 0.714 * V);
	B = (int)(Y + 1.770 * U);

	R= min(255,max(0,R));
	G= min(255,max(0,G));
	B= min(255,max(0,B));
	RGBQUAD rgb={(BYTE)B,(BYTE)G,(BYTE)R,0};
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::RGBtoYUV(RGBQUAD lRGBColor)
{
	int Y,U,V,R,G,B;
	R = lRGBColor.rgbRed;
	G = lRGBColor.rgbGreen;
	B = lRGBColor.rgbBlue;

//	Y = (int)( 0.257 * R + 0.504 * G + 0.098 * B);
//	U = (int)( 0.439 * R - 0.368 * G - 0.071 * B + 128);
//	V = (int)(-0.148 * R - 0.291 * G + 0.439 * B + 128);
	Y = (int)(0.299 * R + 0.587 * G + 0.114 * B);
	U = (int)((B-Y) * 0.565 + 128);
	V = (int)((R-Y) * 0.713 + 128);

	Y= min(255,max(0,Y));
	U= min(255,max(0,U));
	V= min(255,max(0,V));
	RGBQUAD yuv={(BYTE)V,(BYTE)U,(BYTE)Y,0};
	return yuv;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::YIQtoRGB(RGBQUAD lYIQColor)
{
	int Y,I,Q,R,G,B;
	Y = lYIQColor.rgbRed;
	I = lYIQColor.rgbGreen - 128;
	Q = lYIQColor.rgbBlue - 128;

	R = (int)( Y + 0.956 * I + 0.621 * Q);
	G = (int)( Y - 0.273 * I - 0.647 * Q);
	B = (int)( Y - 1.104 * I + 1.701 * Q);

	R= min(255,max(0,R));
	G= min(255,max(0,G));
	B= min(255,max(0,B));
	RGBQUAD rgb={(BYTE)B,(BYTE)G,(BYTE)R,0};
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::RGBtoYIQ(RGBQUAD lRGBColor)
{
	int Y,I,Q,R,G,B;
	R = lRGBColor.rgbRed;
	G = lRGBColor.rgbGreen;
	B = lRGBColor.rgbBlue;

	Y = (int)( 0.2992 * R + 0.5868 * G + 0.1140 * B);
	I = (int)( 0.5960 * R - 0.2742 * G - 0.3219 * B + 128);
	Q = (int)( 0.2109 * R - 0.5229 * G + 0.3120 * B + 128);

	Y= min(255,max(0,Y));
	I= min(255,max(0,I));
	Q= min(255,max(0,Q));
	RGBQUAD yiq={(BYTE)Q,(BYTE)I,(BYTE)Y,0};
	return yiq;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::XYZtoRGB(RGBQUAD lXYZColor)
{
	int X,Y,Z,R,G,B;
	X = lXYZColor.rgbRed;
	Y = lXYZColor.rgbGreen;
	Z = lXYZColor.rgbBlue;
	double k=1.088751;

	R = (int)(  3.240479 * X - 1.537150 * Y - 0.498535 * Z * k);
	G = (int)( -0.969256 * X + 1.875992 * Y + 0.041556 * Z * k);
	B = (int)(  0.055648 * X - 0.204043 * Y + 1.057311 * Z * k);

	R= min(255,max(0,R));
	G= min(255,max(0,G));
	B= min(255,max(0,B));
	RGBQUAD rgb={(BYTE)B,(BYTE)G,(BYTE)R,0};
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::RGBtoXYZ(RGBQUAD lRGBColor)
{
	int X,Y,Z,R,G,B;
	R = lRGBColor.rgbRed;
	G = lRGBColor.rgbGreen;
	B = lRGBColor.rgbBlue;

	X = (int)( 0.412453 * R + 0.357580 * G + 0.180423 * B);
	Y = (int)( 0.212671 * R + 0.715160 * G + 0.072169 * B);
	Z = (int)((0.019334 * R + 0.119193 * G + 0.950227 * B)*0.918483657);

	//X= min(255,max(0,X));
	//Y= min(255,max(0,Y));
	//Z= min(255,max(0,Z));
	RGBQUAD xyz={(BYTE)Z,(BYTE)Y,(BYTE)X,0};
	return xyz;
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::HuePalette(float correction)
{
	if (head.biClrUsed==0) return;

	for(DWORD j=0; j<head.biClrUsed; j++){
		BYTE i=(BYTE)(j*correction*(255/(head.biClrUsed-1)));
		RGBQUAD hsl={120,240,i,0};
		SetPaletteIndex((BYTE)j,HSLtoRGB(hsl));
	}
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Colorize(BYTE hue, BYTE sat)
{
	if (!pDib) return false;

	RGBQUAD color;
	if (head.biClrUsed==0){

		long xmin,xmax,ymin,ymax;
		if (pSelection){
			xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
			ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
		} else {
			xmin = ymin = 0;
			xmax = head.biWidth; ymax=head.biHeight;
		}

		for(long y=ymin; y<ymax; y++){
			for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
				if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
				{
					color = RGBtoHSL(GetPixelColor(x,y));
					color.rgbRed=hue;
					color.rgbGreen=sat;
					SetPixelColor(x,y,HSLtoRGB(color));
				}
			}
		}
	} else {
		for(DWORD j=0; j<head.biClrUsed; j++){
			color = RGBtoHSL(GetPaletteColor((BYTE)j));
			color.rgbRed=hue;
			color.rgbGreen=sat;
			SetPaletteIndex((BYTE)j,HSLtoRGB(color));
		}
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Light(long level, long contrast)
{
	if (!pDib) return false;
	RGBQUAD color;
	float c=contrast/100.0f;
	if (head.biClrUsed==0){

		long xmin,xmax,ymin,ymax;
		if (pSelection){
			xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
			ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
		} else {
			xmin = ymin = 0;
			xmax = head.biWidth; ymax=head.biHeight;
		}

		for(long y=ymin; y<ymax; y++){
			for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
				if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
				{
					color = GetPixelColor(x,y);
					color.rgbRed = (BYTE)max(0,min(255,(int)((color.rgbRed-128)*c + 128 + level)));
					color.rgbGreen = (BYTE)max(0,min(255,(int)((color.rgbGreen-128)*c + 128 + level)));
					color.rgbBlue = (BYTE)max(0,min(255,(int)((color.rgbBlue-128)*c + 128 + level)));
					SetPixelColor(x,y,color);
				}
			}
		}
	} else {
		for(DWORD j=0; j<head.biClrUsed; j++){
			color = GetPaletteColor((BYTE)j);
			color.rgbRed = (BYTE)max(0,min(255,(int)((color.rgbRed-128)*c + 128 + level)));
			color.rgbGreen = (BYTE)max(0,min(255,(int)((color.rgbGreen-128)*c + 128 + level)));
			color.rgbBlue = (BYTE)max(0,min(255,(int)((color.rgbBlue-128)*c + 128 + level)));
			SetPaletteIndex((BYTE)j,color);
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
float CxImage::Mean()
{
	if (!pDib) return 0;
	CxImage tmp(*this,true);
	tmp.GrayScale();
	float sum=0;

	long xmin,xmax,ymin,ymax;
	if (pSelection){
		xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
		ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
	} else {
		xmin = ymin = 0;
		xmax = head.biWidth; ymax=head.biHeight;
	}
	if (xmin==xmax || ymin==ymax) return (float)0.0;

	BYTE *iSrc=tmp.info.pImage;
	for(long y=ymin; y<ymax; y++){
		for(long x=xmin; x<xmax; x++){
			sum+=iSrc[x];
		}
		iSrc+=tmp.info.dwEffWidth;
	}
	return sum/(xmax-xmin)/(ymax-ymin);
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Filter(long* kernel, long Ksize, long Kfactor, long Koffset)
{
	if (!pDib) return false;

	long k2 = Ksize/2;
	long kmax= Ksize-k2;
	long r,g,b,i;
	RGBQUAD c;
	CxImage tmp(*this,pSelection!=0,true,true);

	long xmin,xmax,ymin,ymax;
	if (pSelection){
		xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
		ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
	} else {
		xmin = ymin = 0;
		xmax = head.biWidth; ymax=head.biHeight;
	}

	for(long y=ymin; y<ymax; y++){
		info.nProgress = (long)(100*y/head.biHeight);
		if (info.nEscape) break;
		for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
			if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
				{
				r=b=g=0;
				for(long j=-k2;j<kmax;j++){
					for(long k=-k2;k<kmax;k++){
						c=GetPixelColor(x+j,y+k);
						i=kernel[(j+k2)+Ksize*(k+k2)];
						r += c.rgbRed * i;
						g += c.rgbGreen * i;
						b += c.rgbBlue * i;
					}
				}
				if (Kfactor==0){
					c.rgbRed   = (BYTE)min(255, max(0,(int)(r + Koffset)));
					c.rgbGreen = (BYTE)min(255, max(0,(int)(g + Koffset)));
					c.rgbBlue  = (BYTE)min(255, max(0,(int)(b + Koffset)));
				} else {
					c.rgbRed   = (BYTE)min(255, max(0,(int)(r/Kfactor + Koffset)));
					c.rgbGreen = (BYTE)min(255, max(0,(int)(g/Kfactor + Koffset)));
					c.rgbBlue  = (BYTE)min(255, max(0,(int)(b/Kfactor + Koffset)));
				}
				tmp.SetPixelColor(x,y,c);
			}
		}
	}
	Transfer(tmp);
	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Erode(long Ksize)
{
	if (!pDib) return false;

	long k2 = Ksize/2;
	long kmax= Ksize-k2;
	BYTE r,g,b;
	RGBQUAD c;
	CxImage tmp(*this,pSelection!=0,true,true);

	long xmin,xmax,ymin,ymax;
	if (pSelection){
		xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
		ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
	} else {
		xmin = ymin = 0;
		xmax = head.biWidth; ymax=head.biHeight;
	}

	for(long y=ymin; y<ymax; y++){
		info.nProgress = (long)(100*y/head.biHeight);
		if (info.nEscape) break;
		for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
			if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
			{
				r=b=g=255;
				for(long j=-k2;j<kmax;j++){
					for(long k=-k2;k<kmax;k++){
						c=GetPixelColor(x+j,y+k);
						if (c.rgbRed < r) r=c.rgbRed;
						if (c.rgbGreen < g) g=c.rgbGreen;
						if (c.rgbBlue < b) b=c.rgbBlue;
					}
				}
				c.rgbRed   = r;
				c.rgbGreen = g;
				c.rgbBlue  = b;
				tmp.SetPixelColor(x,y,c);
			}
		}
	}
	Transfer(tmp);
	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Dilate(long Ksize)
{
	if (!pDib) return false;

	long k2 = Ksize/2;
	long kmax= Ksize-k2;
	BYTE r,g,b;
	RGBQUAD c;
	CxImage tmp(*this,pSelection!=0,true,true);

	long xmin,xmax,ymin,ymax;
	if (pSelection){
		xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
		ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
	} else {
		xmin = ymin = 0;
		xmax = head.biWidth; ymax=head.biHeight;
	}

	for(long y=ymin; y<ymax; y++){
		info.nProgress = (long)(100*y/head.biHeight);
		if (info.nEscape) break;
		for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
			if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
			{
				r=b=g=0;
				for(long j=-k2;j<kmax;j++){
					for(long k=-k2;k<kmax;k++){
						c=GetPixelColor(x+j,y+k);
						if (c.rgbRed > r) r=c.rgbRed;
						if (c.rgbGreen > g) g=c.rgbGreen;
						if (c.rgbBlue > b) b=c.rgbBlue;
					}
				}
				c.rgbRed   = r;
				c.rgbGreen = g;
				c.rgbBlue  = b;
				tmp.SetPixelColor(x,y,c);
			}
		}
	}
	Transfer(tmp);
	return true;

}
////////////////////////////////////////////////////////////////////////////////
// thanks to Mwolski <mpwolski@hotmail.com>
void CxImage::Mix(CxImage & imgsrc2, ImageOpType op, long lXOffset, long lYOffset)
{
    long lWide = min(GetWidth(),imgsrc2.GetWidth()-lXOffset);
    long lHeight = min(GetHeight(),imgsrc2.GetHeight()-lYOffset);

    RGBQUAD rgbBackgrnd = GetTransColor();
    RGBQUAD rgb1, rgb2, rgbDest;

    for(long lY=0;lY<lHeight;lY++)
    {
		info.nProgress = (long)(100*lY/head.biHeight);
		if (info.nEscape) break;

        for(long lX=0;lX<lWide;lX++)
        {
#if CXIMAGE_SUPPORT_SELECTION
			if (IsInsideSelection(lX,lY) && imgsrc2.IsInsideSelection(lX+lXOffset,lY+lYOffset))
#endif //CXIMAGE_SUPPORT_SELECTION
			{
				rgb1 = GetPixelColor(lX,lY);
				rgb2 = imgsrc2.GetPixelColor(lX+lXOffset,lY+lYOffset);
				switch(op)
				{
					case OpAdd:
						rgbDest.rgbBlue = (BYTE)max(0,min(255,rgb1.rgbBlue+rgb2.rgbBlue));
						rgbDest.rgbGreen = (BYTE)max(0,min(255,rgb1.rgbGreen+rgb2.rgbGreen));
						rgbDest.rgbRed = (BYTE)max(0,min(255,rgb1.rgbRed+rgb2.rgbRed));
					break;
					case OpSub:
						rgbDest.rgbBlue = (BYTE)max(0,min(255,rgb1.rgbBlue-rgb2.rgbBlue));
						rgbDest.rgbGreen = (BYTE)max(0,min(255,rgb1.rgbGreen-rgb2.rgbGreen));
						rgbDest.rgbRed = (BYTE)max(0,min(255,rgb1.rgbRed-rgb2.rgbRed));
					break;
					case OpAnd:
						rgbDest.rgbBlue = (rgb1.rgbBlue&rgb2.rgbBlue);
						rgbDest.rgbGreen = (rgb1.rgbGreen&rgb2.rgbGreen);
						rgbDest.rgbRed = (rgb1.rgbRed&rgb2.rgbRed);
					break;
					case OpXor:
						rgbDest.rgbBlue = (rgb1.rgbBlue^rgb2.rgbBlue);
						rgbDest.rgbGreen = (rgb1.rgbGreen^rgb2.rgbGreen);
						rgbDest.rgbRed = (rgb1.rgbRed^rgb2.rgbRed);
					break;
					case OpOr:
						rgbDest.rgbBlue = (rgb1.rgbBlue|rgb2.rgbBlue);
						rgbDest.rgbGreen = (rgb1.rgbGreen|rgb2.rgbGreen);
						rgbDest.rgbRed = (rgb1.rgbRed|rgb2.rgbRed);
					break;
					case OpMask:
						if(rgb2.rgbBlue==0 && rgb2.rgbGreen==0 && rgb2.rgbRed==0)
							rgbDest = rgbBackgrnd;
						else
							rgbDest = rgb1;
						break;
					case OpSrcCopy:
						if(memcmp(&rgb1,&rgbBackgrnd,sizeof(RGBQUAD))==0)
							rgbDest = rgb2;
						else // copy straight over
							rgbDest = rgb1;
						break;
					case OpDstCopy:
						if(memcmp(&rgb2,&rgbBackgrnd,sizeof(RGBQUAD))==0)
							rgbDest = rgb1;
						else // copy straight over
							rgbDest = rgb2;
						break;
					case OpSrcBlend:
						if(memcmp(&rgb1,&rgbBackgrnd,sizeof(RGBQUAD))==0)
							rgbDest = rgb2;
						else
						{
							long lBDiff = abs(rgb1.rgbBlue - rgbBackgrnd.rgbBlue);
							long lGDiff = abs(rgb1.rgbGreen - rgbBackgrnd.rgbGreen);
							long lRDiff = abs(rgb1.rgbRed - rgbBackgrnd.rgbRed);

							double lAverage = (lBDiff+lGDiff+lRDiff)/3;
							double lThresh = 16;
							double dLarge = lAverage/lThresh;
							double dSmall = (lThresh-lAverage)/lThresh;
							double dSmallAmt = dSmall*((double)rgb2.rgbBlue);

							if( lAverage < lThresh+1){
								rgbDest.rgbBlue = (BYTE)max(0,min(255,(int)(dLarge*((double)rgb1.rgbBlue) +
												dSmallAmt)));
								rgbDest.rgbGreen = (BYTE)max(0,min(255,(int)(dLarge*((double)rgb1.rgbGreen) +
												dSmallAmt)));
								rgbDest.rgbRed = (BYTE)max(0,min(255,(int)(dLarge*((double)rgb1.rgbRed) +
												dSmallAmt)));
							}
							else
								rgbDest = rgb1;
						}
						break;
						default:
						return;
				}
				SetPixelColor(lX,lY,rgbDest);
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::ShiftRGB(long r, long g, long b)
{
	if (!pDib) return false;
	RGBQUAD color;
	if (head.biClrUsed==0){

		long xmin,xmax,ymin,ymax;
		if (pSelection){
			xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
			ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
		} else {
			xmin = ymin = 0;
			xmax = head.biWidth; ymax=head.biHeight;
		}

		for(long y=ymin; y<ymax; y++){
			for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
				if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
				{
					color = GetPixelColor(x,y);
					color.rgbRed = (BYTE)max(0,min(255,(int)(color.rgbRed + r)));
					color.rgbGreen = (BYTE)max(0,min(255,(int)(color.rgbGreen + g)));
					color.rgbBlue = (BYTE)max(0,min(255,(int)(color.rgbBlue + b)));
					SetPixelColor(x,y,color);
				}
			}
		}
	} else {
		for(DWORD j=0; j<head.biClrUsed; j++){
			color = GetPaletteColor((BYTE)j);
			color.rgbRed = (BYTE)max(0,min(255,(int)(color.rgbRed + r)));
			color.rgbGreen = (BYTE)max(0,min(255,(int)(color.rgbGreen + g)));
			color.rgbBlue = (BYTE)max(0,min(255,(int)(color.rgbBlue + b)));
			SetPaletteIndex((BYTE)j,color);
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Gamma(float gamma)
{
	if (!pDib) return false;
	RGBQUAD color;

	double dinvgamma = 1/gamma;
	double dMax = pow(255.0, dinvgamma) / 255.0;
	
	if (head.biClrUsed==0){

		long xmin,xmax,ymin,ymax;
		if (pSelection){
			xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
			ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
		} else {
			xmin = ymin = 0;
			xmax = head.biWidth; ymax=head.biHeight;
		}

		for(long y=ymin; y<ymax; y++){
			for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
				if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
				{
					color = GetPixelColor(x,y);
					color.rgbRed = (BYTE)max(0,min(255,(int)( pow(float(color.rgbRed), float(dinvgamma)) / dMax)));
					color.rgbGreen = (BYTE)max(0,min(255,(int)( pow(float(color.rgbGreen), float(dinvgamma)) / dMax)));
					color.rgbBlue = (BYTE)max(0,min(255,(int)( pow(float(color.rgbBlue), float(dinvgamma)) / dMax)));
					SetPixelColor(x,y,color);
				}
			}
		}
	} else {
		for(DWORD j=0; j<head.biClrUsed; j++){
			color = GetPaletteColor((BYTE)j);
			color.rgbRed = (BYTE)max(0,min(255,(int)( pow(float(color.rgbRed), float(dinvgamma)) / dMax)));
			color.rgbGreen = (BYTE)max(0,min(255,(int)( pow(float(color.rgbGreen), float(dinvgamma)) / dMax)));
			color.rgbBlue = (BYTE)max(0,min(255,(int)( pow(float(color.rgbBlue), float(dinvgamma)) / dMax)));
			SetPaletteIndex((BYTE)j,color);
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_WINCE == 0
bool CxImage::Median(long Ksize)
{
	if (!pDib) return false;

	long k2 = Ksize/2;
	long kmax= Ksize-k2;
	long i,j,k;

	RGBQUAD* kernel = (RGBQUAD*)malloc(Ksize*Ksize*sizeof(RGBQUAD));

	CxImage tmp(*this,pSelection!=0,true,true);

	long xmin,xmax,ymin,ymax;
	if (pSelection){
		xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
		ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
	} else {
		xmin = ymin = 0;
		xmax = head.biWidth; ymax=head.biHeight;
	}

	for(long y=ymin; y<ymax; y++){
		info.nProgress = (long)(100*y/head.biHeight);
		if (info.nEscape) break;
		for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
			if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
				{
				for(j=-k2, i=0;j<kmax;j++)
					for(k=-k2;k<kmax;k++, i++)
						kernel[i]=GetPixelColor(x+j,y+k);

				qsort(kernel, i, sizeof(RGBQUAD), CompareColors);
				tmp.SetPixelColor(x,y,kernel[i/2]);
			}
		}
	}
	free(kernel);
	Transfer(tmp);
	return true;
}
#endif //CXIMAGE_SUPPORT_WINCE
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Noise(long level)
{
	if (!pDib) return false;
	RGBQUAD color;

	long xmin,xmax,ymin,ymax,n;
	if (pSelection){
		xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
		ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
	} else {
		xmin = ymin = 0;
		xmax = head.biWidth; ymax=head.biHeight;
	}

	for(long y=ymin; y<ymax; y++){
		for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
			if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
			{
				color = GetPixelColor(x,y);
				n=(long)((rand()/(float)RAND_MAX - 0.5)*level);
				color.rgbRed = (BYTE)max(0,min(255,(int)(color.rgbRed + n)));
				n=(long)((rand()/(float)RAND_MAX - 0.5)*level);
				color.rgbGreen = (BYTE)max(0,min(255,(int)(color.rgbGreen + n)));
				n=(long)((rand()/(float)RAND_MAX - 0.5)*level);
				color.rgbBlue = (BYTE)max(0,min(255,(int)(color.rgbBlue + n)));
				SetPixelColor(x,y,color);
			}
		}
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
long CxImage::Histogram(long* red, long* green, long* blue, long* gray, long colorspace)
{
	if (!pDib) return 0;
	RGBQUAD color;

	if (red) memset(red,0,256*sizeof(long));
	if (green) memset(green,0,256*sizeof(long));
	if (blue) memset(blue,0,256*sizeof(long));
	if (gray) memset(gray,0,256*sizeof(long));

	long xmin,xmax,ymin,ymax;
	if (pSelection){
		xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
		ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
	} else {
		xmin = ymin = 0;
		xmax = head.biWidth; ymax=head.biHeight;
	}

	for(long y=ymin; y<ymax; y++){
		for(long x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
			if (IsInsideSelection(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
			{
				switch (colorspace){
				case 1:
					color = HSLtoRGB(GetPixelColor(x,y));
					break;
				case 2:
					color = YUVtoRGB(GetPixelColor(x,y));
					break;
				case 3:
					color = YIQtoRGB(GetPixelColor(x,y));
					break;
				case 4:
					color = XYZtoRGB(GetPixelColor(x,y));
					break;
				default:
					color = GetPixelColor(x,y);
				}

				if (red) red[color.rgbRed]++;
				if (green) green[color.rgbGreen]++;
				if (blue) blue[color.rgbBlue]++;
				if (gray) gray[RGB2GRAY(color.rgbRed,color.rgbGreen,color.rgbBlue)]++;
			}
		}
	}

	long n=0;
	for (int i=0; i<256; i++){
		if (red && red[i]>n) n=red[i];
		if (green && green[i]>n) n=green[i];
		if (blue && blue[i]>n) n=blue[i];
		if (gray && gray[i]>n) n=gray[i];
	}

	return n;
}
////////////////////////////////////////////////////////////////////////////////
#ifndef __BORLANDC__
////////////////////////////////////////////////////////////////////////////////
bool CxImage::FFT2(CxImage* srcReal, CxImage* srcImag, CxImage* dstReal, CxImage* dstImag,
				   long direction, bool bForceFFT, bool bMagnitude)
{
	//check if there is something to convert
	if (srcReal==NULL && srcImag==NULL) return false;

	long w,h;
	//get width and height
	if (srcReal) {
		w=srcReal->GetWidth();
		h=srcReal->GetHeight();
	} else {
		w=srcImag->GetWidth();
		h=srcImag->GetHeight();
	}

	bool bXpow2 = IsPowerof2(w);
	bool bYpow2 = IsPowerof2(h);
	//if bForceFFT, width AND height must be powers of 2
	if (bForceFFT && !(bXpow2 && bYpow2)) {
		long i;
		
		i=0;
		while((1<<i)<w) i++;
		w=1<<i;
		bXpow2=true;

		i=0;
		while((1<<i)<h) i++;
		h=1<<i;
		bYpow2=true;
	}

	// I/O images for FFT
	CxImage *tmpReal,*tmpImag;

	// select output
	tmpReal = (dstReal) ? dstReal : srcReal;
	tmpImag = (dstImag) ? dstImag : srcImag;

	// src!=dst -> copy the image
	if (srcReal && dstReal) tmpReal->Copy(*srcReal,true,false,false);
	if (srcImag && dstImag) tmpImag->Copy(*srcImag,true,false,false);

	// dst&&src are empty -> create new one, else turn to GrayScale
	if (srcReal==0 && dstReal==0){
		tmpReal = new CxImage(w,h,8);
		tmpReal->Clear(0);
		tmpReal->SetGrayPalette();
	} else {
		if (!tmpReal->IsGrayScale()) tmpReal->GrayScale();
	}
	if (srcImag==0 && dstImag==0){
		tmpImag = new CxImage(w,h,8);
		tmpImag->Clear(0);
		tmpImag->SetGrayPalette();
	} else {
		if (!tmpImag->IsGrayScale()) tmpImag->GrayScale();
	}

	if (!(tmpReal->IsValid() && tmpImag->IsValid())){
		if (srcReal==0 && dstReal==0) delete tmpReal;
		if (srcImag==0 && dstImag==0) delete tmpImag;
		return false;
	}

	//resample for FFT, if necessary 
	tmpReal->Resample(w,h,0);
	tmpImag->Resample(w,h,0);

	//ok, here we have 2 (w x h), grayscale images ready for a FFT

	double* real;
	double* imag;
	long j,k,m;

	_complex **grid;
	//double mean = tmpReal->Mean();
	/* Allocate memory for the grid */
	grid = (_complex **)malloc(w * sizeof(_complex));
	for (k=0;k<w;k++) {
		grid[k] = (_complex *)malloc(h * sizeof(_complex));
	}
	for (j=0;j<h;j++) {
		for (k=0;k<w;k++) {
			grid[k][j].x = tmpReal->GetPixelIndex(k,j)-128;
			grid[k][j].y = tmpImag->GetPixelIndex(k,j)-128;
		}
	}

	//DFT buffers
	double *real2,*imag2;
	real2 = (double*)malloc(max(w,h) * sizeof(double));
	imag2 = (double*)malloc(max(w,h) * sizeof(double));

	/* Transform the rows */
	real = (double *)malloc(w * sizeof(double));
	imag = (double *)malloc(w * sizeof(double));

	m=0;
	while((1<<m)<w) m++;

	for (j=0;j<h;j++) {
		for (k=0;k<w;k++) {
			real[k] = grid[k][j].x;
			imag[k] = grid[k][j].y;
		}

		if (bXpow2) FFT(direction,m,real,imag);
		else		DFT(direction,w,real,imag,real2,imag2);

		for (k=0;k<w;k++) {
			grid[k][j].x = real[k];
			grid[k][j].y = imag[k];
		}
	}
	free(real);
	free(imag);

	/* Transform the columns */
	real = (double *)malloc(h * sizeof(double));
	imag = (double *)malloc(h * sizeof(double));

	m=0;
	while((1<<m)<h) m++;

	for (k=0;k<w;k++) {
		for (j=0;j<h;j++) {
			real[j] = grid[k][j].x;
			imag[j] = grid[k][j].y;
		}

		if (bYpow2) FFT(direction,m,real,imag);
		else		DFT(direction,h,real,imag,real2,imag2);

		for (j=0;j<h;j++) {
			grid[k][j].x = real[j];
			grid[k][j].y = imag[j];
		}
	}
	free(real);
	free(imag);

	free(real2);
	free(imag2);

	/* converting from double to byte, there is a HUGE loss in the dynamics
	  "nn" tries to keep an acceptable SNR, but 8bit=48dB: don't ask more */
	double nn=pow(float(2),float(log(float(max(w,h)))/log(float(2))-4));
	//reversed gain for reversed transform
	if (direction==-1) nn=1/nn;
	//bMagnitude : just to see it on the screen
	if (bMagnitude) nn*=4;

	for (j=0;j<h;j++) {
		for (k=0;k<w;k++) {
			if (bMagnitude){
				tmpReal->SetPixelIndex(k,j,(BYTE)max(0,min(255,(nn*(3+log(_cabs(grid[k][j])))))));
				if (grid[k][j].x==0){
					tmpImag->SetPixelIndex(k,j,(BYTE)max(0,min(255,(128+(atan(grid[k][j].y/0.0000000001)*nn)))));
				} else {
					tmpImag->SetPixelIndex(k,j,(BYTE)max(0,min(255,(128+(atan(grid[k][j].y/grid[k][j].x)*nn)))));
				}
			} else {
				tmpReal->SetPixelIndex(k,j,(BYTE)max(0,min(255,(128 + grid[k][j].x*nn))));
				tmpImag->SetPixelIndex(k,j,(BYTE)max(0,min(255,(128 + grid[k][j].y*nn))));
			}
		}
	}

	for (k=0;k<w;k++) free (grid[k]);
	free (grid);

	if (srcReal==0 && dstReal==0) delete tmpReal;
	if (srcImag==0 && dstImag==0) delete tmpImag;

	return true;
}
////////////////////////////////////////////////////////////////////////////////
#endif //__BORLANDC__
////////////////////////////////////////////////////////////////////////////////
bool CxImage::IsPowerof2(long x)
{
	long i=0;
	while ((1<<i)<x) i++;
	if (x==(1<<i)) return true;
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/*
   This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of n=2^m points.
   o(n)=n*log2(n)
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
   Written by Paul Bourke, July 1998
   FFT algorithm by Cooley and Tukey, 1965 
*/
bool CxImage::FFT(int dir,int m,double *x,double *y)
{
	long nn,i,i1,j,k,i2,l,l1,l2;
	double c1,c2,tx,ty,t1,t2,u1,u2,z;

	/* Calculate the number of points */
	nn = 1<<m;

	/* Do the bit reversal */
	i2 = nn >> 1;
	j = 0;
	for (i=0;i<nn-1;i++) {
		if (i < j) {
			tx = x[i];
			ty = y[i];
			x[i] = x[j];
			y[i] = y[j];
			x[j] = tx;
			y[j] = ty;
		}
		k = i2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	/* Compute the FFT */
	c1 = -1.0;
	c2 = 0.0;
	l2 = 1;
	for (l=0;l<m;l++) {
		l1 = l2;
		l2 <<= 1;
		u1 = 1.0;
		u2 = 0.0;
		for (j=0;j<l1;j++) {
			for (i=j;i<nn;i+=l2) {
				i1 = i + l1;
				t1 = u1 * x[i1] - u2 * y[i1];
				t2 = u1 * y[i1] + u2 * x[i1];
				x[i1] = x[i] - t1;
				y[i1] = y[i] - t2;
				x[i] += t1;
				y[i] += t2;
			}
			z =  u1 * c1 - u2 * c2;
			u2 = u1 * c2 + u2 * c1;
			u1 = z;
		}
		c2 = sqrt((1.0 - c1) / 2.0);
		if (dir == 1)
			c2 = -c2;
		c1 = sqrt((1.0 + c1) / 2.0);
	}

	/* Scaling for forward transform */
	if (dir == 1) {
		for (i=0;i<nn;i++) {
			x[i] /= (double)nn;
			y[i] /= (double)nn;
		}
	}

   return true;
}
////////////////////////////////////////////////////////////////////////////////
/*
   Direct fourier transform o(n)=n^2
   Written by Paul Bourke, July 1998 
*/
bool CxImage::DFT(int dir,long m,double *x1,double *y1,double *x2,double *y2)
{
   long i,k;
   double arg;
   double cosarg,sinarg;
   
   for (i=0;i<m;i++) {
      x2[i] = 0;
      y2[i] = 0;
      arg = - dir * 2.0 * 3.14159265358 * i / (double)m;
      for (k=0;k<m;k++) {
         cosarg = cos(k * arg);
         sinarg = sin(k * arg);
         x2[i] += (x1[k] * cosarg - y1[k] * sinarg);
         y2[i] += (x1[k] * sinarg + y1[k] * cosarg);
      }
   }
   
   /* Copy the data back */
   if (dir == 1) {
      for (i=0;i<m;i++) {
         x1[i] = x2[i] / m;
         y1[i] = y2[i] / m;
      }
   } else {
      for (i=0;i<m;i++) {
         x1[i] = x2[i];
         y1[i] = y2[i];
      }
   }
   
   return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Combine(CxImage* r,CxImage* g,CxImage* b,CxImage* a, long colorspace)
{
	if (r==0 || g==0 || b==0) return false;

	long w = r->GetWidth();
	long h = r->GetHeight();

	Create(w,h,24);

	g->Resample(w,h);
	b->Resample(w,h);

	if (a) {
		a->Resample(w,h);
#if CXIMAGE_SUPPORT_ALPHA
		AlphaCreate();
#endif //CXIMAGE_SUPPORT_ALPHA
	}

	RGBQUAD c;
	for (long y=0;y<h;y++){
		for (long x=0;x<w;x++){
			c.rgbRed=r->GetPixelIndex(x,y);
			c.rgbGreen=g->GetPixelIndex(x,y);
			c.rgbBlue=b->GetPixelIndex(x,y);
			switch (colorspace){
			case 1:
				SetPixelColor(x,y,HSLtoRGB(c));
				break;
			case 2:
				SetPixelColor(x,y,YUVtoRGB(c));
				break;
			case 3:
				SetPixelColor(x,y,YIQtoRGB(c));
				break;
			case 4:
				SetPixelColor(x,y,XYZtoRGB(c));
				break;
			default:
				SetPixelColor(x,y,c);
			}
#if CXIMAGE_SUPPORT_ALPHA
			if (a) AlphaSet(x,y,a->GetPixelIndex(x,y));
#endif //CXIMAGE_SUPPORT_ALPHA
		}
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Repair(float radius, long niterations, long colorspace)
{
	if (!IsValid()) return false;

	long w = GetWidth();
	long h = GetHeight();

	CxImage r,g,b;

	r.Create(w,h,8);
	g.Create(w,h,8);
	b.Create(w,h,8);

	switch (colorspace){
	case 1:
		SplitHSL(&r,&g,&b);
		break;
	case 2:
		SplitYUV(&r,&g,&b);
		break;
	case 3:
		SplitYIQ(&r,&g,&b);
		break;
	case 4:
		SplitXYZ(&r,&g,&b);
		break;
	default:
		SplitRGB(&r,&g,&b);
	}
	
	for (int i=0; i<niterations; i++){
		RepairChannel(&r,radius);
		RepairChannel(&g,radius);
		RepairChannel(&b,radius);
	}

	CxImage* a=NULL;
#if CXIMAGE_SUPPORT_ALPHA
	if (HasAlpha()){
		a = new CxImage();
		AlphaSplit(a);
	}
#endif

	Combine(&r,&g,&b,a,colorspace);

	delete a;

	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::RepairChannel(CxImage *ch, float radius)
{
	if (ch==NULL) return false;
	CxImage tmp(*ch);
	long w = ch->GetWidth()-1;
	long h = ch->GetHeight()-1;

	double correction,ix,iy,ixx,ixy,iyy,den,num;
	int x,y,xy0,xp1,xm1,yp1,ym1;
	for(x=1; x<w; x++){
		for(y=1; y<h; y++){

			xy0 = ch->GetPixelIndex(x,y);
			xm1 = ch->GetPixelIndex(x-1,y);
			xp1 = ch->GetPixelIndex(x+1,y);
			ym1 = ch->GetPixelIndex(x,y-1);
			yp1 = ch->GetPixelIndex(x,y+1);

			ix= (xp1-xm1)/2.0;
			iy= (yp1-ym1)/2.0;
			ixx= xp1 - 2.0* xy0 + xm1;
			iyy= yp1 - 2.0* xy0 + ym1;
			ixy=(ch->GetPixelIndex(x+1,y+1)+ch->GetPixelIndex(x-1,y-1)-
				 ch->GetPixelIndex(x-1,y+1)-ch->GetPixelIndex(x+1,y-1))/4.0;

			num= (1.0+iy*iy)*ixx - ix*iy*ixy + (1.0+ix*ix)*iyy;
			den= 1.0+ix*ix+iy*iy;
			correction = num/den;

			tmp.SetPixelIndex(x,y,(BYTE)min(255,max(0,(xy0 + radius * correction))));
		}
	}

	for (x=0;x<=w;x++){
		tmp.SetPixelIndex(x,0,ch->GetPixelIndex(x,0));
		tmp.SetPixelIndex(x,h,ch->GetPixelIndex(x,h));
	}
	for (y=0;y<=h;y++){
		tmp.SetPixelIndex(0,y,ch->GetPixelIndex(0,y));
		tmp.SetPixelIndex(w,y,ch->GetPixelIndex(w,y));
	}
	ch->Transfer(tmp);
	return true;
}
////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_DSP
