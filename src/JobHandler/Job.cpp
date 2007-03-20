/*
 Main

 Copyright (C) 2004 Pluto, Inc., a Florida Corporation

 www.plutohome.com
 

 Phone: +1 (877) 758-8648


 This program is distributed according to the terms of the Pluto Public License, available at:
 http://plutohome.com/index.php?section=public_license

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.

 */
#include "Job.h"
#include "Task.h"
#include "JobHandler.h"
#include "DCE/Logger.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"
#include "DCE/Command_Impl.h"

using namespace DCE;
using namespace nsJobHandler;

int Job::m_NextJobID=0;

Job::Job(JobHandler *pJobHandler,string sName)
{ 
	m_pJobHandler=pJobHandler;
	m_eJobStatus=job_WaitingToStart;
	m_sName=sName; 
	m_iMaxTasks=0; // No limit to number of tasks
	m_iID=m_NextJobID++;
	m_NextTaskID=0; // Unique id's for each task within this job
	m_tNextRunAttempt=0;
}

Job::~Job()
{
	for(list<Task *>::iterator it=m_listTask.begin();it!=m_listTask.end();++it)
		delete *it;
}

// cancel all tasks in this job
bool Job::Abort()
{
	m_bQuit=true;
	if( m_eJobStatus==job_Aborted )
		return true;
	bool bAbortedOk=true;
	PLUTO_SAFETY_LOCK(jm,m_ThreadMutex);

	for(list<class Task *>::iterator it=m_listTask.begin();it!=m_listTask.end();++it)
	{
		Task *pTask = *it;
		if( !pTask->Abort() )
		{
			LoggerWrapper::GetInstance()->Write(LV_WARNING,"Job::Abort %s cannot abort task %s", m_sName.c_str(), pTask->m_sName.c_str());
			bAbortedOk=false;
		}
	}
	if( m_eJobStatus==job_WaitingToStart || m_eJobStatus==job_InProgress )
		m_eJobStatus=job_Aborted;
	return bAbortedOk;
}

// return a task that was not started yet
Task *Job::GetNextTask()
{
	PLUTO_SAFETY_LOCK(jm,m_ThreadMutex);
	for(list<class Task *>::iterator it=m_listTask.begin();it!=m_listTask.end();++it)
	{
		Task *pTask = *it;
		if (pTask->m_eTaskStatus_get() == TASK_NOT_STARTED || pTask->m_eTaskStatus_get() == TASK_IN_PROGRESS )
			return pTask;
		if (pTask->m_eTaskStatus_get() == TASK_FAILED )
			return NULL;  // Don't continue if this task failed
	}
	return NULL;
}

void Job::AddTask(Task *pTask)
{
	PLUTO_SAFETY_LOCK(jm,m_ThreadMutex);
	m_listTask.push_back(pTask);
}

// return number of pending (running, not attempted) tasks
int Job::PendingTasks()
{
	int Tasks=0;
	PLUTO_SAFETY_LOCK(jm,m_ThreadMutex);
	for(list<class Task *>::iterator it=m_listTask.begin();it!=m_listTask.end();++it)
	{
		Task *pTask = *it;
		int eTaskStatus = pTask->m_eTaskStatus_get();
		if (eTaskStatus == TASK_NOT_STARTED || eTaskStatus == TASK_IN_PROGRESS)
			Tasks++;
	}
	return Tasks;
}

bool Job::StartThread()
{
	m_eJobStatus=job_InProgress;
	return ThreadedClass::StartThread();
}

bool Job::StopThread(int iTimeout)
{
	if( !Abort() )
	{
		LoggerWrapper::GetInstance()->Write(LV_WARNING,"Job::StopThread %s cannot abort tasks", m_sName.c_str());
		return false;
	}
	return ThreadedClass::StopThread();
}

// run each task
void Job::Run()
{
	PLUTO_SAFETY_LOCK(jm,m_ThreadMutex);
	Task * pTask;
	while (m_bQuit==false && (pTask = GetNextTask())!=NULL)
	{
		jm.Release();
		if( pTask->m_eTaskStatus_get()==TASK_NOT_STARTED )
			pTask->m_eTaskStatus_set(TASK_IN_PROGRESS);
		int iResult = pTask->Run();
		if( iResult==0 && pTask->m_eTaskStatus_get()==TASK_IN_PROGRESS )
			pTask->m_eTaskStatus_set(TASK_COMPLETED);
		else if( iResult )
		{
			int iSeconds = iResult / 1000;
			int iNanoSeconds = (iResult % 1000) * 1000000;
			jm.Relock();  // we want to be holding the mutex before starting a cond wait
			jm.TimedCondWait(iSeconds,iNanoSeconds);
			jm.Release(); // We will relock it below
		}
		jm.Relock();
	}
	m_eJobStatus=job_Done;
	m_pJobHandler->BroadcastCond();
}

void Job::Reset(bool bDelete)
{	
	if (bDelete)
	{
		for(list<class Task *>::iterator it=m_listTask.begin();it!=m_listTask.end();++it)
		{
			delete *it;
		}
	}
	m_listTask.clear();
}

Task *Job::FindTask(int taskID)
{
	for(list<Task *>::iterator it=m_listTask.begin();it!=m_listTask.end();++it)
	{
		Task *pTask = *it;
		if( pTask->m_iID_get()==taskID )
			return pTask;
	}
	return NULL;
}

bool Job::ReportPendingTasks(PendingTaskList *pPendingTaskList)
{
	PLUTO_SAFETY_LOCK(jm,m_ThreadMutex);
	if( m_eJobStatus==job_WaitingToStart || m_eJobStatus==job_InProgress )
	{
		if( pPendingTaskList )
		{
			pPendingTaskList->m_listPendingTask.push_back(new PendingTask(m_iID,0,0,
				"generic_job","Job: " + m_sName,-1,0,true));
		}

		return true;
	}
	else
		return false;
}
