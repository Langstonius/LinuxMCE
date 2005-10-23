#ifndef DCEDEVICE_H
#define DCEDEVICE_H

#include "DeviceData_Impl.h"
#include "pluto_main/Table_Device.h"
#include "pluto_main/Table_DeviceTemplate.h"

class Row_Device_Device_Pipe;

namespace DCE
{

	// This indicates connectivity, that something is being sent somewhere else, such as audio/video connections
	// For example, the Movie Scaler is connected to the TV, using output ID 5 on the Scaler, and Input ID 3 on the TV
	// There would be 2 pipes, with a destination of the TV.  One an output with ID 5, one an Input with ID 3, both with Kind="Video"
	class Pipe
	{
	public:
		Row_Device_Device_Pipe *m_pRow_Device_Device_Pipe;
		// A plugin or something can set this flag, and then off's won't get propagated.  That way the pipe
		// that goes to the display devices for an on-screen orbiter, for example, can be set to true
		// so that only the 'off' from the on screen orbiter will turn the rendering device off.
		bool m_bDontSendOff;  
		Pipe(Row_Device_Device_Pipe *pRow_Device_Device_Pipe) { m_pRow_Device_Device_Pipe=pRow_Device_Device_Pipe; m_bDontSendOff=false; }
	};

	class Command
	{
	public:
		Command(int PK_Command,string sDescription,int PK_CommandCategory,string sCommandCategory_Description,bool bLog) { 
			m_dwPK_Command=PK_Command; m_sDescription=sDescription; m_bLog=bLog; 
			m_dwPK_CommandCategory=PK_CommandCategory;
			m_sCommandCategory_Description=sCommandCategory_Description;
		}
		bool m_bLog;
		int m_dwPK_Command,m_dwPK_CommandCategory;
		string m_sDescription,m_sCommandCategory_Description;

		list<int> m_listPipe;
	};

	class Event_Router
	{
	public:
		Event_Router(int PK_Event,string sDescription,bool bLog) { m_dwPK_Event=PK_Event; m_sDescription=sDescription; m_bLog=bLog; }
		bool m_bLog;
		int m_dwPK_Event;
		string m_sDescription;
	};

	class CommandGroup_Command
	{
	public:
		Command *m_pCommand;
		class DeviceData_Router *m_pDeviceData_Router;
		map<int,string> m_mapParameter;

		CommandGroup_Command(Command *pCommand,class DeviceData_Router *pDeviceData_Router)
		{
			m_pCommand=pCommand;
			m_pDeviceData_Router=pDeviceData_Router;
		}

		~CommandGroup_Command()
		{
			m_mapParameter.clear();
		}
	};

	class CommandGroup
	{
	public:
		int m_PK_CommandGroup;
		vector<class CommandGroup_Command *> m_vectCommandGroup_Command;
		string m_Description;
		int m_PK_C_Array;

		CommandGroup(int PK_CommandGroup,int PK_C_Array=0) : 
		m_PK_CommandGroup(PK_CommandGroup), m_PK_C_Array(PK_C_Array) {}

		~CommandGroup()
		{
			vector<class CommandGroup_Command *>::iterator iA;

			for (iA = m_vectCommandGroup_Command.begin(); iA != m_vectCommandGroup_Command.end(); ++iA)
			{
				delete (*iA); 
			}
		}
	};

	class Room
	{
	public:
		unsigned long m_dwPK_Room;
		string m_sDescription;
		vector<class CommandGroup *> m_vectCommandGroups;
		list<class DeviceData_Router *> m_listDevices;

		Room(unsigned long dwPK_Room,string sDescription)
		{	
			m_dwPK_Room = dwPK_Room;
			m_sDescription = sDescription;
		}
	};

	class DeviceRelation 
	{
	public:
		class DeviceData_Router *m_pDevice;
		string m_sParms;
		DeviceRelation(class DeviceData_Router *pDevice,string sParms)
		{
			m_pDevice=pDevice;
			m_sParms=sParms;
		}
	};

	class DeviceData_Router : public DeviceData_Impl
	{
	private:
		// The following are stored in the Device table so they persist
		string m_sStatus; // A device can set a generic status message, such as "triggered".  There is no predefined purpose.  Status is considered a constantly changing value
		string m_sState;  // This is user defined.  Examples are: on, off, 50%, bypassed, etc.  State is the value that the user set and wants the device to stay in

	public:

		// **** SERIALIZED VALUES FROM THE DATABASE ****

		// General ID's for this Device
		// These are in the base-> int m_dwPK_Device,m_dwPK_Installation,m_dwPK_DeviceTemplate,m_dwPK_DeviceCategory,m_dwPK_Room,;
		// string m_sDescription;

		// If the device is running on a different installation, this will point to the IP address
		// of the Router to send the messages to
		string m_sForeignRouter;

		// The command line to execute for this device
		string m_sCommandLine; 

		// If set to true, the first time this device connects we will send it a reload.  This is good for 
		// controllers which try to continually reconnect and may not realize the server has been restarted
		bool m_bForceReloadOnFirstConnect;

		map<int, class Pipe *> m_mapPipe_Available; // The available pipes
		map<int, class Pipe *> m_mapPipe_Active; // The currently activated pipes
		Pipe *m_mapPipe_Active_Find(int PK_Pipe) { map<int,class Pipe *>::iterator it = m_mapPipe_Active.find(PK_Pipe); return it==m_mapPipe_Active.end() ? NULL : (*it).second; }

		// All the groups, parameters, inputs, etc.
		map<int,class DeviceRelation *> m_mapDeviceRelation;
		map<int,class DeviceGroup *> m_mapDeviceGroup;
		vector<DeviceData_Router *> m_vectDevices_SendingPipes;  // The devices that send this one something

		// **** FLAGS AND INFORMATION SET AT RUNTIME ****

		bool m_bIsRegistered,m_bIsReady; // Some devices will set this by sending a ready message/event
		bool m_bBusy; // A device can be marked busy.  It's messages will be queued until this flag is cleared
		bool m_bAlert;  // A device with this flag set is having a problem

		string m_IPAddr;
		time_t m_tCanReceiveNextCommand,m_tLastused;

		char *m_pMySerializedData;  // A copy of the device serialized so we don't have to do it for each request
		int m_iConfigSize;

		string m_sStatus_get() { return m_sStatus; }
		string m_sState_get() { return m_sState; }
		void m_sStatus_set(string sStatus) { m_pRow_Device->Reload(); m_sStatus=sStatus; m_pRow_Device->Status_set(m_sStatus); m_pRow_Device->Table_Device_get()->Commit(); }
		void m_sState_set(string sState) { m_pRow_Device->Reload(); m_sState=sState; m_pRow_Device->State_set(m_sState); m_pRow_Device->Table_Device_get()->Commit(); }

		// **** POINTERS CREATED BY THE SERIALIZED ID'S ****

		Room *m_pRoom;
		// A virtual device that doesn't really exist, but serves as a placeholder will have it's messages routed to another device
		// For example, a Television may have several tuners.  Each tuner must be a separate device so the user
		// can tune on any one of them.  They are all marked as RouteTo the television itself.
		class DeviceData_Router *m_pDevice_RouteTo;
		class DeviceData_Router *m_pDevice_Audio,*m_pDevice_Video;
		Row_Device *m_pRow_Device;

		map<int,class ServerSocket *> m_mapSocket_Event;
		class ServerSocket *m_pSocket_Command;


		DeviceData_Router(Row_Device *pRow_Device, Room *pRoom, string sCommandLine)
			: DeviceData_Impl(pRow_Device->PK_Device_get(),pRow_Device->FK_Installation_get(),pRow_Device->FK_DeviceTemplate_get(),pRow_Device->FK_Device_ControlledVia_get(),pRow_Device->FK_DeviceTemplate_getrow()->FK_DeviceCategory_get(),pRoom ? pRoom->m_dwPK_Room : 0,
			pRow_Device->FK_DeviceTemplate_getrow()->ImplementsDCE_get()==1,
			pRow_Device->FK_DeviceTemplate_getrow()->IsEmbedded_get()==1,sCommandLine,pRow_Device->FK_DeviceTemplate_getrow()->IsPlugIn_get()==1,pRow_Device->Description_get(),pRow_Device->IPaddress_get(),pRow_Device->MACaddress_get(),pRow_Device->FK_DeviceTemplate_getrow()->InheritsMacFromPC_get()==1 )
		{
			m_pRow_Device=pRow_Device;
			m_bForceReloadOnFirstConnect=m_bIsRegistered=m_bIsReady=m_bBusy=m_bAlert=false;
			m_tLastused=m_tCanReceiveNextCommand=0;
			m_sState = m_pRow_Device->State_get();
			m_sStatus = m_pRow_Device->Status_get();

			m_pRoom=pRoom;
			m_pDevice_ControlledVia=m_pDevice_RouteTo=NULL;
			m_pDevice_Audio=m_pDevice_Video=NULL;
			m_pMySerializedData=NULL;
			m_pSocket_Command=NULL;
			m_iConfigSize=0;
		}

		// Another constructor for dynamically loaded plug-ins
		DeviceData_Router(int PK_Device,int PK_DeviceTemplate,int PK_Installation, int PK_Device_ControlledVia)
			: DeviceData_Impl(PK_Device,PK_Installation,PK_DeviceTemplate,PK_Device_ControlledVia,0 /* category */,0 /* room */,true /* implements dce */,false /* is embedded */,
			"" /* Command line */,true /* Is Plugin */,"" /* Description */,"localhost","" /* Mac Address */, false)
		{
			m_pRow_Device=NULL;
			m_bForceReloadOnFirstConnect=m_bIsRegistered=m_bIsReady=m_bBusy=m_bAlert=false;
			m_tLastused=m_tCanReceiveNextCommand=0;

			m_pRoom=NULL;
			m_pDevice_ControlledVia=m_pDevice_RouteTo=NULL;
			m_pDevice_Audio=m_pDevice_Video=NULL;
			m_pMySerializedData=NULL;
			m_pSocket_Command=NULL;
			m_iConfigSize=0;
		}

		~DeviceData_Router()
		{
			delete[] m_pMySerializedData;
			map<int,class DeviceRelation *>::iterator itMD;
			for(itMD=m_mapDeviceRelation.begin();itMD!=m_mapDeviceRelation.end();++itMD)
			{
				class DeviceRelation *pR = (*itMD).second;
				delete pR;
			}
		}

		DeviceRelation *mapDeviceRelation_Find(int PK_Device)
		{
			map<int,class DeviceRelation *>::iterator it = m_mapDeviceRelation.find(PK_Device);
			return it==m_mapDeviceRelation.end() ? NULL : (*it).second;
		}

		void FindSibblingsWithinCategory(DeviceCategory *pDeviceCategory,vector<DeviceData_Router *> &vectDeviceData_Router)
		{
			if( !pDeviceCategory || !m_pDevice_ControlledVia )
				return;
			for( size_t s=0;s<((DeviceData_Router *)m_pDevice_ControlledVia)->m_vectDeviceData_Impl_Children.size();++s )
			{
				DeviceData_Router *pDevice = (DeviceData_Router *) ((DeviceData_Router *)m_pDevice_ControlledVia)->m_vectDeviceData_Impl_Children[s];
				if( pDevice->WithinCategory(pDeviceCategory) )
					vectDeviceData_Router.push_back(pDevice);
			}

		}

		void FindSibblingsWithinCategory(int PK_DeviceCategory,vector<DeviceData_Router *> &vectDeviceData_Router)
		{
			if( !PK_DeviceCategory || !m_pDevice_ControlledVia )
				return;
			for( size_t s=0;s<((DeviceData_Router *)m_pDevice_ControlledVia)->m_vectDeviceData_Impl_Children.size();++s )
			{
				DeviceData_Router *pDevice = (DeviceData_Router *) ((DeviceData_Router *)m_pDevice_ControlledVia)->m_vectDeviceData_Impl_Children[s];
				if( pDevice->WithinCategory(PK_DeviceCategory) )
					vectDeviceData_Router.push_back(pDevice);
			}
		}

		void FindChildrenWithinCategory(int PK_DeviceCategory,vector<DeviceData_Router *> &vectDeviceData_Router)
		{
			if( !PK_DeviceCategory )
				return;
			for( size_t s=0;s<m_vectDeviceData_Impl_Children.size();++s )
			{
				DeviceData_Router *pDevice = (DeviceData_Router *) m_vectDeviceData_Impl_Children[s];
				if( pDevice->WithinCategory(PK_DeviceCategory) )
					vectDeviceData_Router.push_back(pDevice);
			}
		}

		// We're not going to use this, since we're not creating actual devices.  Implement this pure virtual function from our base class
		class DeviceData_Impl *CreateData(DeviceData_Impl *Parent,char *pDataBlock,unsigned long AllocatedSize,char *CurrentPosition) { return NULL; };
	};

	typedef list<DeviceData_Router *> ListDeviceData_Router;
}

#endif

