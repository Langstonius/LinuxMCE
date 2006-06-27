#include "gl2deffectfadesfromunderneath.h"

#include "gl2deffectfactory.h"

#include "../Layers/GL2DEffectLayersCompose.h"

namespace GLEffect2D
{

GL2DEffectFadesFromUnderneath::GL2DEffectFadesFromUnderneath (EffectFactory * EffectsEngine, int StartAfter,
															  int TimeForCompleteEffect)
	: GL2DEffectTransit(EffectsEngine, StartAfter, TimeForCompleteEffect)
{
	FullScreen.Left = 0;
	FullScreen.Top = 0;
	FullScreen.Width = float(Effects->Widgets->GetWidth());
	FullScreen.Height = float(Effects->Widgets->GetHeight());

	//creating a basic window that merge the effect
	Destination = (TBasicWindow*)Effects->Widgets->CreateWidget(BASICWINDOW, 
		0, 0, 
		Effects->Widgets->GetWidth(), Effects->Widgets->GetHeight(), 
		"Destination");	
	Destination->SetVisible(true);
}


GL2DEffectFadesFromUnderneath::~GL2DEffectFadesFromUnderneath() {
	Effects->Widgets->DeleteWidget(Destination);
}

void GL2DEffectFadesFromUnderneath::Configure(PlutoRectangle* EffectSourceSize)
{
	ButtonSize.Left = 0;
	ButtonSize.Top = -FullScreen.Height;
	ButtonSize.Width = FullScreen.Width;
	ButtonSize.Height = FullScreen.Height;
	Configured = false;
}

/**
 * Paint the effect based of the current time
 * @param Now Gives the global time
 */
void GL2DEffectFadesFromUnderneath::Paint(int Now)
{
	GL2DEffectTransit::Paint();

	if(!Configured) {
		//Set up the textures for triangles
		OpenGLGraphic* OldScreenRenderGraphic =
			GLEffect2D::LayersCompose::Instance()->NewScreen->GetRenderGraphic();
		Destination->SetBackgroundImage(OldScreenRenderGraphic);
		
		float MaxCoordU = 1;
		float MaxCoordV = 1;

		Destination->SetTextureWraping(0.0, 0.0, 
			MaxCoordU, MaxCoordV);
	
		Start = Effects->MilisecondTimmer();

		Configured = true;
	}
	
	float Step = Stage(float(Now));
	
	FloatRect Animation = ButtonSize.Interpolate(FullScreen, Step);

	Destination->SetRectCoordinates(Animation);
	Destination->SetAlpha(Step);
}

}
