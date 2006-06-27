#include "MeshBuilder.h"

#include "../GLMathUtils.h"

MeshBuilder::MeshBuilder(void) : Blended_(false)
{
	this->GlobalRed = 1.0f;
	this->GlobalGreen = 1.0f;
	this->GlobalBlue = 1.0f;
	this->GlobalAlpha = 1.0f;
	this->GlobalU = 1.0f;
	this->GlobalV = 1.0f;
	this->GlobalMaterial = NULL;
}

MeshBuilder::~MeshBuilder(void)
{
	CleanUp();
}

void MeshBuilder::CleanUp()
{
	Vertexes.clear();
	Triangles.clear();	
}

void MeshBuilder::Begin(unsigned char BuildMode)
{
	CleanUp();
	this->BuildMode = BuildMode;
}

/*virtual*/ MeshContainer* MeshBuilder::End()
{
	int Count;
	
	MeshContainer* Result = new MeshContainer();
	Result->NoVertexes = int(Vertexes.size());
	Result->Vertexes = new MeshVertex[Result->NoVertexes];
	
	for(Count = 0; Count < Result->NoVertexes; Count++)
		Result->Vertexes[Count] = Vertexes[Count];

	Result->NoTriangles = int(Triangles.size());
	Result->Triangles = new MeshTriangle[Result->NoTriangles];
	for(Count = 0; Count < Result->NoTriangles; Count++)
		Result->Triangles[Count] = Triangles[Count];
			
	Result->Blended_ = Blended_;

	CleanUp();
	return Result;
}

/*virtual*/ void MeshBuilder::AddVertexFloat(float X, float Y, float Z)
{
	MeshVertex Vertex;
	Vertex.X = X; Vertex.Y = Y; Vertex.Z = Z; 
	Vertex.UVW.X = GlobalU; Vertex.UVW.Y = GlobalV; 
	Vertex.Alpha = GlobalAlpha;
	Vertex.Color.X = GlobalRed; Vertex.Color.Y = GlobalGreen; Vertex.Color.Z = GlobalBlue;
	Vertexes.push_back(Vertex);
	BuildTriangle();
}

/*virtual*/ void MeshBuilder::AddVertexFloatUV(float X, float Y, float Z, float U, float V)
{
	MeshVertex Vertex;
	Vertex.X = X; Vertex.Y = Y; Vertex.Z = Z; 
	Vertex.UVW.X = U; Vertex.UVW.Y = V; 
	Vertex.Alpha = GlobalAlpha;
	Vertex.Color.X = GlobalRed; Vertex.Color.Y = GlobalGreen; Vertex.Color.Z = GlobalBlue;
	
	Vertexes.push_back(Vertex);
	BuildTriangle();
}

/*virtual*/ void MeshBuilder::BuildTriangle()
{
	int NoVertexes = int(Vertexes.size());
	switch(BuildMode)
	{
	case MBMODE_TRIANGLES:
		if(NoVertexes % 3 == 0)
			AddTriangle(
				NoVertexes-3,		
				NoVertexes-2, 
				NoVertexes-1);	
		
	break;
	case MBMODE_TRIANGLE_STRIP:
		if(NoVertexes >= 3)
			if(NoVertexes % 2 == 0)
			AddTriangle(
				NoVertexes-3,
				NoVertexes-2, 
				NoVertexes-1);
			else
				AddTriangle(
				NoVertexes-3,
				NoVertexes-1,
				NoVertexes-2
				);

	break;
	}
}

/*virtual*/ void MeshBuilder::AddTriangle(int Vertex1, int Vertex2, int Vertex3)
{
	MeshTriangle Triangle;
	Triangle.Vertex1 = Vertex1;
	Triangle.Vertex2 = Vertex2;
	Triangle.Vertex3 = Vertex3;
	Triangle.Texture = GlobalMaterial;
	Triangles.push_back(Triangle);
}

/*virtual*/ void MeshBuilder::SetTexture2D(float U, float V)
{
	GlobalU = U;
	GlobalV = V;
}
/*virtual*/ void MeshBuilder::SetTexture(OpenGLGraphic*Texture)
{
	GlobalMaterial = Texture;	
}

/*virtual*/ void MeshBuilder::SetColor(float Red, float Green, float Blue)
{
	this->GlobalRed = Red;	
	this->GlobalGreen = Green;
	this->GlobalBlue = Blue;
}

/*virtual*/ void MeshBuilder::SetAlpha(float Alpha)
{
	this->GlobalAlpha = Alpha;
	SetBlended(true);
}

/*virtual*/ void MeshBuilder::SetBlended(bool Blended)
{
	Blended_ = Blended;
}
