

#include "common.h"
#include "servertcp.h"
#include "mainwindowpockemul.h"
#include "pobject.h"
#include "cloud/cloudwindow.h"

extern MainWindowPockemul *mainwindow;
extern QList<CPObject *> listpPObject;

ServeurTcp :: ServeurTcp (QObject *parent)
{
    Q_UNUSED(parent)

    Connect();
    currentPC = 0;
}

void ServeurTcp::Connect() {
    if (CloudWindow::getValueFor("telnetEnabled")=="on") {
        int _port = CloudWindow::getValueFor("telnetPort").toInt();
        listen(QHostAddress::LocalHost,_port);
        QObject:: connect(this, SIGNAL(newConnection()),this, SLOT(demande_connexion()));
    }
}

// si un client demande une connexion
void ServeurTcp :: demande_connexion()
 {
    emit vers_IHM_connexion(); // on envoie un signal a l'IHM
    // on cree une nouvelle socket pour ce client
    clientConnection = nextPendingConnection();
    // si on recoit des donnees, le slot lecture() est appele
    QObject:: connect(clientConnection, SIGNAL(readyRead()),this, SLOT(lecture()));
    clientConnection->setTextModeEnabled(true);
    send_greeting(new QTextStream(clientConnection));
}
void ServeurTcp ::lecture()
{
    QString ligne;
    qWarning()<<"LECTURE";
    while(clientConnection->canReadLine())    // tant qu'on peut lire sur la socket
    {
        qWarning()<<"LECTURE 2";
        ligne = clientConnection->readLine().trimmed(); // on lit une ligne

        ligne.remove('\n');
        qWarning()<<ligne;
        emit vers_IHM_texte(ligne);           // on l'envoie a l'IHM
        process_command(ligne);
        QTextStream texte(clientConnection);
        texte <<"PockEmul>" << flush;
    }

}

void ServeurTcp::send_greeting(QTextStream *sock)
{
    *sock << endl << "Welcome to the PockEmul v1.0 TELNET interface." << endl;
//    *sock << "Use decimal, C hex (0x2a) or Asm hex (14h) for input." << endl;
//    *sock << "Type 'help' for a list of supported commands." << endl<< endl;
    *sock << "PockEmul>" << flush;

}

void ServeurTcp::cmd_help(QTextStream *sock,QString subcmd)
{
    if (subcmd=="") {
        *sock << "Help" << endl << "====" << endl;
        *sock << "  bye" << endl;
        *sock << "  close Id" << endl;
        *sock << "  list            List all running model Ids" << endl;
        *sock << "  load pml_file   Load a .pml file" << endl;
        *sock << "  select Id       focus on the designed pocket. Use list to retrieve Ids" << endl;
        *sock << "  start model_name" << endl;
//        *sock << "  key(k) [enter cr f1 esc ctrl+c shift+code+a \"Text\" ...]" << endl;
    }
    else if(subcmd=="start") {
        *sock << "Help start" << endl << "==========" << endl;
        *sock << "  start ModelName" << endl;
        QHashIterator<QString, Models> i(mainwindow->objtable);
        while (i.hasNext()) {
            i.next();
            *sock << i.key() << " , " ;
        }
        *sock << endl;
    }
    else if(subcmd=="close") {
        *sock << "Help close" << endl << "==========" << endl;
        *sock << "  close [all,Id]  close all pocket or select by Id (use list command)." << endl;
    }
    else if(subcmd=="select") {
        *sock << "Help select" << endl << "==========" << endl;
        *sock << "  select [none,Id]  define corresponding Pocket as current pocket for next commands." << endl;
    }
}
void ServeurTcp::process_command(QString ligne)
{

    if (ligne.isEmpty()) return;

    // Separate out the command word from any arguments
    QStringList args =  splitArgs(ligne);//ligne.split(' ',QString::SkipEmptyParts);
    QString cmd = args.first();


    QTextStream texte(clientConnection);      // creation d'un flux pour ecrire dans la socket
//    texte << \"message recu\n" << endl;          // message à envoyer au client

    qWarning()<<"cmd:'"<<cmd<<"'";
    if (cmd == "help")				// Check for help
        cmd_help(&texte,(args.size()>1)?args.at(1):"");
    else if (cmd=="start") {
        qWarning()<<args[1];
        mainwindow->LoadPocket((args.size()>1)?args.at(1):"");
    }
    else if (cmd=="load") {
        qWarning()<<args[1];
        if (args.size()>1)
        mainwindow->opensession(args.at(1));
    }
    else if (cmd == "list") {
        for (int i=0;i<listpPObject.size();i++)
        {
            CPObject *po = listpPObject.at(i);
            texte << i << " : "<< po->getName();
            if (po == currentPC) {
                texte << " (** selected Pocket **)";
            }
            texte<<endl;
        }
    }
    else if (cmd=="select") {
        if ((args.size()==2) && (args.at(1) == "none")) {
            currentPC = 0;
            return;
        }
        if ((args.size()==2) && (args.at(1).toInt() < listpPObject.size())) {
            currentPC = listpPObject.at(args.at(1).toInt());
            currentPC->raise();
            currentPC->setFocus();
            return;
        }
        texte << "Bad argument." << endl;
        cmd_help(&texte,"select");
        return;
    }
    else if (cmd=="close") {
        if ((args.size()==2) && (args.at(1) == "all")) {
            for (int i=0;i<listpPObject.size();i++)
            {
                CPObject *po = listpPObject.at(i);
                po->slotExit();
            }
            return;
        }
        if ((args.size()==2) && (args.at(1).toInt() < listpPObject.size())) {
            mainwindow->saveAll = NO;
            CPObject *pc = listpPObject.at(args.at(1).toInt());
            pc->slotExit();
            return;
        }
        texte << "Bad argument." << endl;
        cmd_help(&texte,"close");
        return;
    }
}

QStringList ServeurTcp::splitArgs(QString cmd) {
    bool inside = (cmd.at(0) == '\"'); //true if the first character is "
    QStringList tmpList = cmd.split(QRegExp("\""), QString::SkipEmptyParts); // Split by " and make sure you don't have an empty string at the beginning
    QStringList csvlist;
    foreach (QString s, tmpList) {
        if (inside) { // If 's' is inside quotes ...
            csvlist.append(s); // ... get the whole string
        } else { // If 's' is outside quotes ...
            csvlist.append(s.split(" ", QString::SkipEmptyParts)); // ... get the splitted string
        }
        inside = !inside;
    }

    return csvlist;
}
