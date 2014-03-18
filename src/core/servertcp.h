#ifndef SERVERTCP_H
#define SERVERTCP_H

#include <QTcpServer>
#include <QTcpSocket>

class CPObject;

class ServeurTcp : public QTcpServer
{
Q_OBJECT
public :
    ServeurTcp(QObject *parent=0);

    void process_command(QString ligne);
    void send_greeting(QTextStream *sock);
    void cmd_help(QTextStream *sock, QString subcmd);
    QStringList splitArgs(QString cmd);
private slots :
    void demande_connexion() ;
    void lecture();
signals :
    void vers_IHM_connexion();
    void vers_IHM_texte(QString);
private :
    QTcpSocket *clientConnection;
    CPObject *currentPC;
};

#endif // SERVERTCP_H
