#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <string>
using namespace std;
// these should be split into multiple headers?

#include "Orbiter/DesignObj_Data.h"
#include "SDL.h"
#include "Orbiter/RendererImage.h"
#include "Orbiter/RendererMNG.h"

struct SDL_Surface;
class RendererImage;

#ifdef OrbiterGen
    class DesignObj_Generator;
#endif
//------------------------------------------------------------------------
pair<int, int> GetWordWidth(string Word, string FontPath, TextStyle *pTextStyle, RendererImage * & RI, bool NewSurface = true);
int DoRenderToScreen(list<RendererImage *> &RI, int posX, int posY);
int DoRenderToSurface(SDL_Surface * Surface, list<RendererImage *> &RI, int posX, int posY);
void extDeleteRendererImage(RendererImage * & RI);
//------------------------------------------------------------------------
class TextObj
{
    public:
        enum AlignFlags {Left = 0, Center = 1, Right = 2, Top = 4, Middle = 8, Bottom = 16};

        PlutoPoint position;
        string text;
        bool bold, italic, underline;
        AlignFlags Align;

        TextObj() : position(0, 0), text(""), bold(false), italic(false), underline(false) {}
};

//------------------------------------------------------------------------
// TODO: write API docs for this one
class Renderer
{
public:
	string m_sFontPath,m_sOutputDirectory;
	int m_Width,m_Height;
	static int m_Rotate;
	SDL_Surface * Screen;

	Renderer(string FontPath,string OutputDirectory,int Width,int Height,bool bDisableVideo=false);
	~Renderer();

	static RendererImage * CreateBlankCanvas(PlutoSize size,bool bFillIt=false);
	PlutoSize RealRenderText(RendererImage * pRenderImage, DesignObjText *pDesignObjText, TextStyle *pTextStyle, PlutoPoint pos);

	// This comes from the Size table,ScaleMenuBg & ScaleOtherGraphics.  For both, options are: 'Y', make the Y axis fit, crop the X, 'X' is opposite
	// 'S', means scale according to the ScaleX and ScaleY, which come from the table
	// 'F', means stretch the image so it fits
	char m_cDefaultScaleForMenuBackground,m_cDefaultScaleForOtherGraphics; 
	static float m_fScaleX,m_fScaleY;

#ifndef ORBITER
	void RenderObject(RendererImage *pRenderImage,DesignObj_Generator *pDesignObj_Generator,
		PlutoPoint Position,int iRenderStandard,int iOnlyVersion=-999);
	void RenderObjectsChildren(RendererImage *pRenderImage,DesignObj_Generator *pDesignObj_Generator,PlutoPoint pos,int iOnlyVersion);
	void RenderObjectsText(RendererImage *pRenderImage,DesignObj_Generator *pDesignObj_Generator,PlutoPoint pos,int iIteration);
	static void SaveImageToFile(RendererImage * pRendererImage, string sSaveToFile, bool bUseOCG=false);
	static void SaveImageToPNGFile(RendererImage * pRendererImage, FILE * File, bool Signature = true);
	static RendererImage *Subset(RendererImage *pRenderImage,PlutoRectangle rect);
	// If Crop is true and PreserveAspectRatio is true, then instead of shrinking to fit within the given space, it will
	// fill the target space, and any excess will be cropped
	static RendererImage *CreateFromFile(string sFilename, PlutoSize size=PlutoSize(0,0),bool bPreserveAspectRatio=true,bool bCrop=false,char cScale=0,PlutoRectangle offset=PlutoRectangle(0,0), bool bUseAntiAliasing=true);
	static RendererImage *CreateFromFile(FILE * File, PlutoSize size=PlutoSize(0,0),bool bPreserveAspectRatio=true,bool bCrop=false,char cScale=0,PlutoRectangle offset=PlutoRectangle(0,0), bool bUseAntiAliasing=true);
	static RendererImage *CreateFromRWops(SDL_RWops * rw, bool bFreeRWops = true, PlutoSize size=PlutoSize(0,0),bool bPreserveAspectRatio=true,bool bCrop=false,char cScale='F',PlutoRectangle offset=PlutoRectangle(0,0), bool bUseAntiAliasing=true);  // if cUseAxis is 'X' or 'Y', then that axis will be used whether it's a crop or not
	static void CompositeImage(RendererImage *pRenderImage_Parent,RendererImage *pRenderImage_Child,PlutoPoint pos);
	static void CompositeAlpha(RendererImage *pRenderImage_Parent,RendererImage *pRenderImage_Child,PlutoPoint pos);
	static RendererImage * DuplicateImage(RendererImage * pRendererImage);
	void RenderText(RendererImage *pRenderImage, DesignObjText *pDesignObjText, TextStyle *pTextStyle, DesignObj_Generator *pDesignObj_Generator, PlutoPoint pos);

	static RendererMNG * CreateMNGFromFile(string FileName, PlutoSize Size);
	static RendererMNG * CreateMNGFromFiles(const vector<string> & FileNames, PlutoSize Size);
	static void SaveMNGToFile(string FileName, RendererMNG * MNG);
    static void SetTransparentColor(SDL_Surface *pSurface, int R, int G, int B);
#endif

protected:
	static Uint32 getpixel(RendererImage * RIsurface, int x, int y);
	static void putpixel(RendererImage * RIsurface, int x, int y, Uint32 pixel_color);
};

//------------------------------------------------------------------------

#endif
