#ifndef _SLOT_H_
#define _SLOT_H_

#include <QString>

class CSlot 
{
public:

    enum SlotType
    {
        RAM,
        ROM,
        CUSTOM_ROM,
        NOT_USED
    };

    CSlot(	int size, int adr, QString	resid, QString filename, SlotType type, QString label)
	{
		Size = size;
		Adr	= adr;
		ResID = resid;
		FileName = filename;
		Type = type;
        Label = label;
        Empty = resid.isEmpty() && filename.isEmpty();
    }
	
	int		getSize()		  { return Size; }
    void	setSize(int size) { Size = size; }
	
	int		getAdr()		{ return Adr; }
    void	setAdr(int adr)	{ Adr = adr; }
	
	QString getResID() { return ResID; }
    void	setResID(QString resid)	{ ResID = resid; }
	
	QString getFileName() { return FileName; }
    void	setFileName(QString filename)	{ FileName = filename; }
	
	QString getLabel() { return Label; }
    void	setLabel(QString label)	{ Label = label; }

    SlotType		getType() { return Type; }
    void	setType(SlotType type)	{ Type = type; }

    bool    isEmpty() { return Empty; }
    void	setEmpty(bool empty)	{ Empty = empty; }
	
private:
	int		Size;
	int		Adr;
	QString	ResID;
	QString	FileName;
    SlotType		Type;
	QString	Label;
    bool    Empty;
	
};
#endif
