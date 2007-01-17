/*
Orbiter

Copyright (C) 2006 Pluto, Inc., a Florida Corporation

www.plutohome.com

Phone: +1 (877) 758-8648

This program is distributed according to the terms of the Pluto Public License, available at:
http://plutohome.com/index.php?section=public_license

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.

@author: "Ciprian Mustiata" ciprian.m at plutohome dot com, "Cristian Miron" chris.m at plutohome dot com 

*/
#include "MeshFrame.h"
#include "MeshPainter.h"
#include "DCE/Logger.h"

#define DEBUG_MESH_FRAMES

#ifdef DEBUG_MESH_FRAMES
	static int nMeshCounter = 0;
	static int nCloneMeshCounter = 0;
#endif

MeshFrame::MeshFrame(string Name, MeshContainer* Mesh) 
	: 
	m_pParent(NULL),
	m_bVisible(true),
	m_bVolatile(false),
	m_bDontReleaseTexture(false),
	Transform(),
	TextureTransform()
{
	m_pMeshContainer = Mesh;
	m_sName = Name;

#ifdef DEBUG_MESH_FRAMES
	if(m_sName.find("clone") != string::npos)
		++nCloneMeshCounter;

	DCE::g_pPlutoLogger->Write(LV_STATUS, "aaa MeshFrame constructor %p/%s, (count %d), (clones %d)", this, m_sName.c_str(), ++nMeshCounter, nCloneMeshCounter);
#endif
}

MeshFrame::~MeshFrame(void)
{
#ifdef DEBUG_MESH_FRAMES
	if(m_bDontReleaseTexture)
		DCE::g_pPlutoLogger->Write(LV_STATUS, "Not releasing texture for %p/%s, volatile %d", this, m_sName.c_str(), m_bVolatile);
#endif

	if(IsVolatile() && NULL != m_pMeshContainer && !m_bDontReleaseTexture)
		m_pMeshContainer->DisposeTextures();

	delete m_pMeshContainer;
	m_pMeshContainer = NULL;

	if(!m_bVolatile)
		TextureManager::Instance()->InvalidateItem(this);

#ifdef DEBUG_MESH_FRAMES
	if(m_sName.find("clone") != string::npos)
		--nCloneMeshCounter;

	DCE::g_pPlutoLogger->Write(LV_WARNING, "aaa MeshFrame destructor %p/%s, volatile %d, (count %d), (clones %d)", this, m_sName.c_str(), m_bVolatile, --nMeshCounter, nCloneMeshCounter);
#endif
}

void MeshFrame::MarkAsVolatile() 
{ 
	m_bVolatile = true; 
}

void MeshFrame::MarkAsVolatileRecursively() 
{ 
	m_bVolatile = true; 

	vector<MeshFrame*>::iterator Child;
	for(Child = Children.begin(); Child!=Children.end();++Child)
	{
		MeshFrame *pMeshFrame = *Child;
		pMeshFrame->MarkAsVolatileRecursively();
	}
}

/*virtual*/ void MeshFrame::CleanUp(bool VolatilesOnly/* = false*/)
{
#ifdef DEBUG_MESH_FRAMES
	DCE::g_pPlutoLogger->Write(LV_STATUS, "MeshFrame::CleanUp: %p/%s", this, m_sName.c_str());	
#endif

	vector<MeshFrame*>::iterator Child;
	for(Child = Children.begin(); Child!=Children.end();++Child)
	{
		MeshFrame *pMeshFrame = *Child;
		pMeshFrame->CleanUp(true);

		if(!VolatilesOnly || pMeshFrame->IsVolatile())
		{
			delete pMeshFrame;
			pMeshFrame = NULL;
		}
		else
		{
			pMeshFrame->ResetParent();
		}
	}	

	Children.clear();
}

void MeshFrame::SetMeshContainer(MeshContainer* Mesh)
{
	m_pMeshContainer = Mesh;
}

void MeshFrame::AddChild(MeshFrame* Frame)
{
	if(Frame == NULL)
	{
		DCE::g_pPlutoLogger->Write(LV_WARNING, "MeshFrame::AddChild: Frame is NULL!!!");	
		return;
	}

	if(NULL != Frame->m_pParent)
	{
		DCE::g_pPlutoLogger->Write(LV_WARNING, "MeshFrame::AddChild: Frame %p/%s already has a parent %p!",
			Frame, Frame->m_sName.c_str(), Frame->m_pParent);	

		//throw "Frame already has a parent";
	}

#ifdef DEBUG_MESH_FRAMES
	DCE::g_pPlutoLogger->Write(LV_STATUS, "MeshFrame::AddChild: Added %p/%s to %p/%s", Frame, Frame->m_sName.c_str(),
		this, m_sName.c_str());	
#endif

	Children.push_back(Frame);
	Frame->m_pParent = this;

	CheckIntegrity(Frame);
}

void MeshFrame::RemoveChild(MeshFrame* Frame)
{
	//CheckIntegrity(Frame);

	if(Frame == NULL)
	{
		//DCE::g_pPlutoLogger->Write(LV_CRITICAL, "MeshFrame::RemoveChild: Frame is NULL!!!");
		return;
	}

	if(NULL != Frame->m_pParent)
	{
		vector<MeshFrame*>::iterator Child = 
			find(Frame->m_pParent->Children.begin(), Frame->m_pParent->Children.end(), Frame);
		if(Child == Frame->m_pParent->Children.end())
		{
			DCE::g_pPlutoLogger->Write(LV_CRITICAL, "MeshFrame::RemoveChild: Got a parent, but doesn't have us as child!");
			//throw "Got a parent, but doesn't have us as child";
		}
		else
		{
			Frame->m_pParent->Children.erase(Child);

#ifdef DEBUG_MESH_FRAMES
			DCE::g_pPlutoLogger->Write(LV_STATUS, "MeshFrame::RemoveChild %p/%s from parent %p/%s", 
				Frame, Frame->m_sName.c_str(), Frame->m_pParent, Frame->m_pParent->m_sName.c_str());
#endif

			Frame->m_pParent = NULL;

			if(Frame->IsVolatile())
			{
				Frame->CleanUp(true);
				delete Frame;
				Frame = NULL;
			}
		}
	}
	else
	{
		DCE::g_pPlutoLogger->Write(LV_CRITICAL, "MeshFrame::RemoveChild: Got no parent! :(");
		//throw "Got no parent! :( I'm all alone!";
	}
}

MeshFrame* MeshFrame::ReplaceChild(MeshFrame* OldFrame, MeshFrame* NewFrame)
{
	CheckIntegrity(OldFrame);

	if(OldFrame == NULL)
	{
		//DCE::g_pPlutoLogger->Write(LV_CRITICAL, "MeshFrame::ReplaceChild: Frame is NULL!!!");
		return NULL;
	}

	if(NULL != OldFrame->m_pParent)
	{
		vector<MeshFrame*>::iterator Child = find(OldFrame->m_pParent->Children.begin(), 
			OldFrame->m_pParent->Children.end(), OldFrame);

		if(Child == OldFrame->m_pParent->Children.end())
		{
			DCE::g_pPlutoLogger->Write(LV_CRITICAL, "MeshFrame::ReplaceChild: Got a parent, but doesn't have us as child!");
			//throw "Got a parent, but doesn't have us as child";
		}
		else
		{
			NewFrame->m_pParent = OldFrame->m_pParent;
			*Child = NewFrame;

#ifdef DEBUG_MESH_FRAMES
			DCE::g_pPlutoLogger->Write(LV_STATUS, "ttt MeshFrame::ReplaceChild %p/%s from parent %p/%s with %p/%s", 
				OldFrame, OldFrame->m_sName.c_str(), OldFrame->m_pParent, OldFrame->m_pParent->m_sName.c_str(),
				NewFrame, NewFrame->m_sName.c_str());
#endif

			if(NewFrame != OldFrame && NULL != OldFrame && OldFrame->IsVolatile())
			{
				//DCE::g_pPlutoLogger->Write(LV_CRITICAL, "MeshFrame::ReplaceChild: replaced a volatile %p/%s",
				//	OldFrame, OldFrame->Name().c_str());

				OldFrame->CleanUp(true);
				delete OldFrame;
				OldFrame = NULL;
			}

			CheckIntegrity(NewFrame);

			return NewFrame;
		}
	}
	else
	{
		DCE::g_pPlutoLogger->Write(LV_CRITICAL, "MeshFrame::ReplaceChild: Got no parent! :(");
		//throw "Got no parent! :( I'm all alone!";
	}

	return OldFrame;
}

void MeshFrame::Paint(MeshTransform ChildTransform)
{
	if(!m_bVisible)
	{
		//DCE::g_pPlutoLogger->Write(LV_STATUS, "NOT Painting %p. It's invisible.", this);	
		return; 
	}
	
	MeshTransform Transform;
	Transform.ApplyTransform(this->Transform);
	Transform.ApplyTransform(ChildTransform);
	MeshPainter* Painter = MeshPainter::Instance();
	if(m_pMeshContainer!= NULL)
		Painter->PaintContainer(*m_pMeshContainer, Transform, TextureTransform);

	//DCE::g_pPlutoLogger->Write(LV_STATUS, "xxxxx Painting %p with %d children: ", this, Children.size());

	vector<MeshFrame *>::iterator Child, EndChild;
	for(Child = Children.begin(), EndChild = Children.end(); Child != EndChild; ++Child)
	{  
		MeshFrame *pMeshFrame = *Child;
		pMeshFrame->Paint(Transform);
	}

	//DCE::g_pPlutoLogger->Write(LV_STATUS, "xxxxx Painting %p END ", this);
}

/*virtual*/ void MeshFrame::SetTransform(MeshTransform& Transform)
{
	this->Transform.CopyFrom(Transform);
}

/*virtual*/ void MeshFrame::ApplyTransform(MeshTransform& Transform)
{
	this->Transform.ApplyTransform(Transform);
}

/*virtual*/ void MeshFrame::SetVisible(bool Visible)
{
	m_bVisible = Visible;
}

/*virtual*/ void MeshFrame::Paint()
{
    MeshTransform Transform;
	Paint(Transform);
}

/*virtual*/ void MeshFrame::SetTextureTransform(MeshTransform& TextureTransform)
{
	this->TextureTransform = TextureTransform;
}

MeshContainer* MeshFrame::GetMeshContainer()
{
	return m_pMeshContainer;
}

/*virtual*/ void MeshFrame::SetAlpha(float Alpha, string ExcludePattern /* = "" */)
{
	//we don't want to setup alpha for Frames that have their name starting with exclude pattern
	if(!ExcludePattern.empty())
	{
		vector<string> vectPatterns;
		StringUtils::Tokenize(ExcludePattern, "|", vectPatterns);

		for(vector<string>::iterator it = vectPatterns.begin(); it != vectPatterns.end(); ++it)
		{
			string sPattern = *it;
			if(m_sName.find(sPattern) == 0)
				return;
		}
	}

	if(m_pMeshContainer)
		m_pMeshContainer->SetAlpha(Alpha);

	vector<MeshFrame *>::iterator Child, EndChild;
	for(Child = Children.begin(), EndChild = Children.end(); Child != EndChild; ++Child)
	{  
		MeshFrame *pMeshFrame = *Child;
		pMeshFrame->SetAlpha(Alpha, ExcludePattern);
	}
}

/*virtual*/ void MeshFrame::SetColor(Point3D& Color)
{
	if(m_pMeshContainer)
		m_pMeshContainer->SetColor(Color);
	vector<MeshFrame *>::iterator Child, EndChild;
	for(Child = Children.begin(), EndChild = Children.end(); Child != EndChild; ++Child)
	{  
		MeshFrame *pMeshFrame = *Child;
		pMeshFrame->SetColor(Color);
	}
}

MeshFrame *MeshFrame::Clone()
{
	MeshFrame *Result = new MeshFrame(m_sName + " clone");

	Result->MarkAsVolatile();
	
	if(m_sName.find("datagrid-thumb") == string::npos)
		Result->m_bDontReleaseTexture = true;

	Result->m_bVisible = m_bVisible;
	if(NULL != m_pMeshContainer)
		Result->m_pMeshContainer = m_pMeshContainer->Clone();

	Result->Transform = Transform;
	Result->TextureTransform = TextureTransform;

	for(vector<MeshFrame*>::iterator it = Children.begin(), end = Children.end(); it != end; ++it)
	{
		Result->AddChild((*it)->Clone());
	}
	return Result;
}

void MeshFrame::Print(string sIndent)
{
	//if(m_sName.find("rectangle") != string::npos)
	//	return;

	//if(m_sName.find("text") != string::npos)
	//	return;

	DCE::g_pPlutoLogger->Write(LV_STATUS, "%s%s '%s' %s %p", sIndent.c_str(), 
		Children.size() ? "+ " : "- ", m_sName.c_str(), m_bVolatile ? "(v)" : "", this);

	for(vector<MeshFrame*>::iterator it = Children.begin(), end = Children.end(); it != end; ++it)
	{
		(*it)->Print(sIndent + "`-");
	}
}

MeshFrame *MeshFrame::FindChild(string Name)
{
	MeshFrame *ChildMesh = NULL;

	if(m_sName == Name)
		ChildMesh = this;
	else
	{
		for(vector<MeshFrame*>::iterator it = Children.begin(), end = Children.end(); it != end; ++it)
		{
			ChildMesh = (*it)->FindChild(Name);

			if(NULL != ChildMesh)
				return ChildMesh;
		}
	}

	return ChildMesh;
}

void MeshFrame::Stealth()
{	
	m_sName = m_sName + " stealth";

	for(vector<MeshFrame*>::iterator it = Children.begin(), end = Children.end(); it != end; ++it)
		(*it)->Stealth();
}

bool MeshFrame::CheckIntegrity(MeshFrame *Frame)
{
	bool Result = false;

	if(NULL != Frame && NULL != Frame->m_pParent)
	{
		vector<MeshFrame*>::iterator Child = find(Frame->m_pParent->Children.begin(), 
			Frame->m_pParent->Children.end(), Frame);

        Result = Child != Frame->m_pParent->Children.end();	
	}

	if(!Result)
	{
		DCE::g_pPlutoLogger->Write(LV_CRITICAL, "MeshFrame::CheckIntegrity failed for %p/%s",
			Frame, NULL != Frame ? Frame->Name().c_str() : "null frame");
	}

	return Result;
}
