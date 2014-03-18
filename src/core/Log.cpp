#include "common.h"
#include "Log.h"
#include "pcxxxx.h"
#include "init.h"
#include "dialoglog.h"
 

void AddLogbis(int Level,QString str)
{
	if (mainwindow->dialoglog)
	{
		if (Level & mainwindow->dialoglog->LogLevel)
		{
			mainwindow->SendSignal_AddLogItem(str);
		}
	}
}

