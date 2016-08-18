#include "binarydata.h"


BinaryData::BinaryData(QAbstractListModel*parent) :
    QAbstractListModel(parent)
{
    mKeyMap[Qt::Key_J] = Qt::Key_Down;
    mKeyMap[Qt::Key_K] = Qt::Key_Up;
    mKeyMap[Qt::Key_L] = Qt::Key_Right;
    mKeyMap[Qt::Key_H] = Qt::Key_Left;
}

void BinaryData::load(const QByteArray& data)
{
    beginResetModel();
    mMode = BrowseMode;
    mPrompt = tr("Goto Address: ");
    mData = data;
    mEntered.clear();
    mSearchPattern.clear();
    mLastSearchIdx = -1;
    setAddress("0");
    qDebug() << "loading data " << mData.length();
    endResetModel();
}

QHash<int, QByteArray> BinaryData::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Line] = "lineData";
    return roles;
}

QStringList BinaryData::containingRow(const QModelIndex &index)
{
    if ((index == mLastIdx) && !mLastResult.isEmpty())
        return mLastResult;

    mLastResult.clear();
    mLastResult.reserve(LineSize);
    mLastIdx = index;

    int offset = index.row() * LineSize;
    QString addr = QString("%1").arg(offset, 4, 16, QChar('0')).toUpper();
    mLastResult.append(addr);
    const QByteArray& lineData = mData.mid(offset, LineSize);
    foreach (char c, lineData) {
        QString byte = QString("%1").arg((quint8)c, 2, 16, QChar('0')).toUpper();
        mLastResult.append(byte);
    }
    return mLastResult;
}

QVariant BinaryData::data(const QModelIndex &index, int role) const
{
    (void)role;
    return const_cast<BinaryData*>(this)->containingRow(index);
}

int BinaryData::rowCount(const QModelIndex &parent) const
{
    (void)parent;
    return mData.count() / LineSize;
}


bool BinaryData::hexRegEx(int keyCode) const
{
    return (keyCode != Qt::Key_Backspace) && (keyCode != Qt::Key_Return);
}

bool BinaryData::hexadecimal(int keyCode) const
{
    return (((keyCode >= Qt::Key_0) && (keyCode <= Qt::Key_9)) ||
            ((keyCode >= Qt::Key_A) && (keyCode <= Qt::Key_F)));
}

bool BinaryData::canEdit(int keyCode) const
{
    bool permit = (mOffset != 0);
    return permit && hexadecimal(keyCode);
}

bool BinaryData::isNavigation(int keyCode) const
{
    return (keyCode == Qt::Key_Left) || (keyCode == Qt::Key_Right) || (keyCode == Qt::Key_Down) || (keyCode == Qt::Key_Up);
}

void BinaryData::keyPress(int keyCode, const QString& keyText)
{
    QString before = mEntered;

    switch (keyCode)
    {
    case Qt::Key_Escape:
        mEntered.clear();
        break;
    case Qt::Key_Backspace:
        mEntered.chop(1);
        break;
    }

    switch (mMode) {
    case BrowseMode:
        keyPressBrowseMode(keyCode, keyText);
        break;
    case InsertMode:
        keyPressInsertMode(keyCode, keyText);
        break;
    case SearchMode:
        keyPressSearchMode(keyCode, keyText);
    }

    if (before != mEntered) {
        emit enteredChange();
    }
}


void BinaryData::keyPressBrowseMode(int keyCode, const QString& keyText)
{
    if (mKeyMap.contains(keyCode)) {
        keyCode = mKeyMap[keyCode];
    }

    if (isNavigation(keyCode)) {
        keyboardNavigation(keyCode);
    }

    if (hexadecimal(keyCode)) {
        mEntered += keyText.toUpper();
    }

    switch (keyCode) {
    case Qt::Key_I:
        setMode(InsertMode);
        break;
    case Qt::Key_N:
        mLastSearchIdx = findPattern(mLastSearchIdx + 1);
        break;
    case Qt::Key_G:
    case Qt::Key_Return: {
        QString addr;
        if ((keyText == "G") && mEntered.isEmpty())
            mEntered = QString("%1").arg(mData.length() - LineSize, 0, 16);
        setAddress(mEntered);
        mEntered.clear();
    }
        break;
    case Qt::Key_Slash:
        setMode(SearchMode);
        break;
    }
}

void BinaryData::keyPressInsertMode(int keyCode, const QString& keyText)
{
    if (mKeyMap.contains(keyCode)) {
        keyCode = mKeyMap[keyCode];
    }

    if (hexadecimal(keyCode)) {
        mEntered += keyText.toUpper();

        if ((mOffset != 0) && (editCell())) {
            mEntered.clear();
            keyCode = Qt::Key_Right;
        }
    }

    if (isNavigation(keyCode)) {
        keyboardNavigation(keyCode);
    }

    switch (keyCode) {
    case Qt::Key_I:
    case Qt::Key_Escape:
        setMode(BrowseMode);
        break;
    }
}

void BinaryData::keyPressSearchMode(int keyCode, const QString& keyText)
{
    if (hexRegEx(keyCode))
        mEntered += keyText;

    switch (keyCode) {
    case Qt::Key_Escape:
        setMode(BrowseMode);
        break;
    case Qt::Key_Return:
        dataToString();
        mSearchPattern = mEntered.toUpper();
        mLastSearchIdx = findPattern(0);
        break;
    }
}

void BinaryData::keyboardNavigation(int keyCode)
{
    switch (keyCode) {
    case Qt::Key_Up:
        if (mAddress >= LineSize)
            mAddress -= LineSize;
        break;
    case Qt::Key_Down:
        if (mAddress + LineSize < mData.length())
            mAddress += LineSize;
        break;
    case Qt::Key_Left:
        if (mOffset > 1)
            mOffset--;
        else {
            if (mAddress >= LineSize) {
                mAddress -= LineSize;
                mOffset = LineSize;
            }
        }
        emit offsetChange();
        break;
    case Qt::Key_Right:
        if (mOffset < LineSize) {
            mOffset++;
        } else {
            mOffset = 1;
            if (mAddress + LineSize < mData.length())
                mAddress += LineSize;
        }
        emit offsetChange();
        break;
    }

    mEntered.clear();
    emit addressChange(mAddress);
}

bool BinaryData::editCell()
{
    int addr = mAddress + mOffset - 1;
    mData[addr] = mEntered.toInt(0, 16);

    QModelIndex row = index(addr / LineSize);
    mLastResult.clear();
    emit dataChanged(row, row);
    bool editComplete = (mEntered.length() == 2);
    if (editComplete)
        mEntered.clear();

    return editComplete;
}


void BinaryData::setAddress(const QString& addr)
{
    int address = addr.toInt(0, 16);
    mAddress = address / LineSize * LineSize;
    mOffset = address % LineSize + 1;
    addressChange(mAddress);
    offsetChange();
}

void BinaryData::setMode(Mode mode)
{
    switch (mode) {
    case SearchMode: mPrompt = tr("Find Pattern: "); break;
    case BrowseMode: mPrompt = tr("Goto Address: "); break;
    default:
        mPrompt.clear();
        break;
    }
    emit promptChange();

    mEntered.clear();
    mMode = mode;
    emit modeChange();
}


int BinaryData::findPattern(int start)
{
    int addr = -1;
    start *= 2;

    ///// search only on whole bytes. Don''t match least significat 4 bits of one byte and 4 MSB  of the other as a byte
    do {
        QRegularExpression regex(mSearchPattern);
        QRegularExpressionMatch match = regex.match(mStrData, start);
        if (match.hasMatch()) {
            addr = match.capturedStart();
        }
    } while ((addr != -1) && ((addr % 2) == 1) && (++start < mStrData.length()));

    addr>>=1;

    if (addr != -1) {
        QString strAddr = QString("%1").arg(addr, 4, 16, QChar('0')).toUpper();
        setAddress(strAddr);
    }

    setMode(BrowseMode);
    return addr;
}

void BinaryData::dataToString()
{
    mStrData.clear();
    mStrData.reserve(mData.length() * 2); // each byte is represented by 2 characters
    for (int i = 0; i < mData.length(); ++i) {
        mStrData  += QString("%1").arg((quint8)mData[i] & 0xFF, 2, 16, QChar('0')).toUpper();
    }
}

int BinaryData::search(const QString& pattern, bool findFirst)
{
    if (findFirst) {
        dataToString();
        mSearchPattern = pattern.toUpper();
        mLastSearchIdx = -1;
    }
    mLastSearchIdx = findPattern(mLastSearchIdx + 1);
    return mLastSearchIdx;
}
