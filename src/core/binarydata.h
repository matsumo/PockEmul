#ifndef BINARYDATA_H
#define BINARYDATA_H

#include <QtCore>
#include <QStandardItemModel>

class BinaryData : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(Param)
    Q_ENUMS(Mode)
public:
    enum Roles {
        Line = Qt::UserRole + 1
    };

    enum Mode {
        BrowseMode,
        InsertMode,
        SearchMode
    };

    enum Param {
        LineSize = 16
    };

    explicit BinaryData(QAbstractListModel *parent = 0);
    Q_PROPERTY (QString address READ address WRITE setAddress NOTIFY addressChange)
    Q_PROPERTY (int offset READ offset WRITE setOffset NOTIFY offsetChange)
    Q_PROPERTY (Mode mode READ mode WRITE setMode NOTIFY modeChange)
    Q_PROPERTY (QString entered READ entered NOTIFY enteredChange)
    Q_PROPERTY (QString prompt READ prompt NOTIFY promptChange)

    Q_INVOKABLE int search(const QString& pattern, bool findFirst);
    Q_INVOKABLE void keyPress(int keyCode, const QString& keyText);
    Q_INVOKABLE int findPattern(int start);
    void load(const QByteArray& data);

    QString address() const {return QString("%1").arg(mAddress, 4, 16, QChar('0')).toUpper();}
    void setAddress(const QString& addr);

    int offset() const {return mOffset;}
    void setOffset(int off) {mOffset = off; offsetChange();}

    Mode mode() const {return mMode;}
    void setMode(Mode mode);

    QVariant data(const QModelIndex &index, int role) const;
    QStringList containingRow(const QModelIndex &index, int role);

    QString entered() const {return mEntered;}
    QString prompt() const {return mPrompt;}

    int rowCount(const QModelIndex &parent) const;
protected:
    virtual QHash<int, QByteArray> roleNames() const;

private:
    int mAddress;
    int mOffset;
    QString mEntered;
    QStringList mLastResponse;
    QModelIndex mLastRequest;
    QMap<int, int> mKeyMap;
    Mode mMode;

    QByteArray mData;
    QString mStrData;
    QModelIndex mLastIdx[10];
    QStringList mLastResult[10];

    int mLastSearchIdx;
    QString mSearchPattern;

    QString mPrompt;

    bool canEdit(int keyCode) const;
    bool hexadecimal(int keyCode) const;
    bool hexRegEx(int keyCode) const;
    bool isNavigation(int keyCode) const;

    void insertSampleData();
    bool editCell();
    void keyboardNavigation(int keyCode);
    void dataToString();

    void keyPressBrowseMode(int keyCode, const QString& keyText);
    void keyPressInsertMode(int keyCode, const QString& keyText);
    void keyPressSearchMode(int keyCode, const QString& keyText);

signals:
    void addressChange(int address);
    void offsetChange();
    void modeChange();
    void enteredChange();
    void promptChange();
};

#endif // BINARYDATA_H
