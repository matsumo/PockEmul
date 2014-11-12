#include <QDebug>

#include "clink.h"
#include "Connect.h"
#include "Log.h"
#include "ccable.h"

#ifdef P_AVOID
#include "libavoid.h"
#endif

CDirectLink::CDirectLink(void) {

}

void CDirectLink::addLink(Cconnector *A, Cconnector *B, bool close)
{
    addlinkMutex.lock();
    if (close) {

        AConnList.append(A);
        BConnList.append(B);
        closeList.append(close);
    }
    else {  // Create a link object
#if 1
        AConnList.append(A);
        BConnList.append(B);
        closeList.append(close);
#ifdef AVOID
        Avoid::ConnEnd srcEnd(mainwindow->shapeRefList[A->Parent],A->Id+1);
        Avoid::ConnEnd dstEnd(mainwindow->shapeRefList[B->Parent],B->Id+1);
        new Avoid::ConnRef(mainwindow->router, srcEnd, dstEnd);
        mainwindow->router->processTransaction();
        mainwindow->router->outputInstanceToSVG("test-connectionpin01");
#endif
#else
        // 1 - create the CCable object
         Ccable *pPC = (Ccable*)mainwindow->LoadPocket(CABLE11Pins);
        // 2 - define its size and position
         bool reverse = pPC->Adapt(A,B);
         pPC->standard = true;
         addLink(A,pPC->ConnList.at(reverse?1:0),true);
         pPC->ConnList.at(reverse?1:0)->setOppDir(A->getDir());
         addLink(pPC->ConnList.at(reverse?0:1),B,true);
         pPC->ConnList.at(reverse?0:1)->setOppDir(B->getDir());
#endif
    }
    addlinkMutex.unlock();
}

void CDirectLink::removeLink(int ind)
{
    addlinkMutex.lock();
    AConnList.removeAt(ind);
    BConnList.removeAt(ind);
    closeList.removeAt(ind);
    addlinkMutex.unlock();
}

// Return the first connected object found
CPObject * CDirectLink::findObj(CPObject * search)
{
	for (int i = 0;i < search->ConnList.size(); i++)
 	{
        int found = AConnList.indexOf(search->ConnList.at(i));
        if (found >= 0)	return BConnList.at(found)->Parent;
        found = BConnList.indexOf(search->ConnList.at(i));
        if (found >= 0)	return AConnList.at(found)->Parent;

	}


	return 0;
}

// return a list of all connected  objects
void CDirectLink::findAllObj(CPObject * search, QList<CPObject *> * liste, bool onlyclosed)
{
	int i,j;
	
	for (i = 0;i < search->ConnList.size(); i++)
 	{
 		
        for (j = 0;j < AConnList.size(); j++)
	 	{
            bool closed;
            if (onlyclosed) {
                closed = true;
            }
            else
                closed = closeList.at(j)
;
            if ( (closeList.at(j)==closed) && ( search->ConnList.at(i) == AConnList.at(j) ))
	 		{
	 			if (liste->indexOf(BConnList.at(j)->Parent) == -1)
	 			{
	 				liste->append(BConnList.at(j)->Parent);
	 				findAllObj(BConnList.at(j)->Parent, liste);
 				}
 			}
		}
		for ( j = 0;j < BConnList.size(); j++)
	 	{
            bool closed;
            if (onlyclosed) {
                closed = true;
            }
            else
                closed = closeList.at(j);

            if ((closeList.at(j)==closed) &&( search->ConnList.at(i) == BConnList.at(j) ))
	 		{
	 			if (liste->indexOf(AConnList.at(j)->Parent) == -1)
	 			{
	 				liste->append(AConnList.at(j)->Parent);
	 				findAllObj(AConnList.at(j)->Parent, liste);
 				}
 			}
		}
	}

}


bool CDirectLink::isLinked(CPObject * search)
{
	return ( findObj(search) ? true : false );
}

bool CDirectLink::isLinked(Cconnector * search)
{
     return ((AConnList.indexOf(search)>=0) || (BConnList.indexOf(search)>=0)) ;
}

Cconnector * CDirectLink::Linked(Cconnector * search)
{
    int i = AConnList.indexOf(search);
    if (i>=0) return BConnList.at(i);
    i = BConnList.indexOf(search);
    if (i>=0) return AConnList.at(i);

    return 0;
}

void CDirectLink::Cascade(CPObject *pPC)
{
    connectorsMutex.lock();
    clearlog();
    Output(pPC);
    connectorsMutex.unlock();
}

// Update all conectors of a CPOboject
void CDirectLink::updateConnectors(CPObject* pPC) {
    for (int i = 0;i < pPC->ConnList.size(); i++)
    {
        pPC->ConnList.at(i)->ConnectTo(Linked(pPC->ConnList.at(i)));
    }
}

void CDirectLink::Output(CPObject* pPC)
{
//    qWarning()<<"Output:"<<pPC->getcfgfname();
	for (int i = 0;i < pPC->ConnList.size(); i++)
 	{
 		outConnector( pPC->ConnList.at(i) );
    }
}

void CDirectLink::outConnector(Cconnector* search)
{
    addlinkMutex.lock();

	// Search the connector in List A
	int found = AConnList.indexOf(search);
    Cconnector *foundConnector;
    if ( found >= 0 ) {
        foundConnector = BConnList.at(found);
    }
    else { 	// Search the connector in List B
        found = BConnList.indexOf(search);
        if ( found >= 0 ) {
            foundConnector = AConnList.at(found);
        }
    }
    addlinkMutex.unlock();

    if ( found >= 0 ) {
        foundConnector->ConnectTo(search);
        // start the corresponding pPC
        // if no frequency link to the parent timer
        if ( (foundConnector->Parent->getfrequency() == 0 ) &&
             (! inlogrun(foundConnector->Parent) ))
        {
//				AddLog(LOG_TEMP,tr("Connector Chain with %1").arg(BConnList.at(found)->Parent->getName()));
            foundConnector->Parent->pTIMER = search->Parent->pTIMER;
            updateConnectors(foundConnector->Parent);
            insertlog(foundConnector->Parent);
//            qWarning()<<"run (before):"<<foundConnector->Parent->getcfgfname();
            foundConnector->Parent->run();
//            qWarning()<<"run (after) :"<<foundConnector->Parent->getcfgfname();
            Output( foundConnector->Parent );
        }
    }
}




