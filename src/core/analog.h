#ifndef ANALOG_H
#define ANALOG_H
#include <QList>
#include <QHash>

typedef	struct 
{
	qint64	values;
    quint64	state;
    quint8	marker;
} TAnalog_Data;


class CData:public QObject
{
    public:
		void			Clear(void);
		void			Write(TAnalog_Data);
        void			Write(qint64, quint64);

		TAnalog_Data	Read(int);
		qint64			Read_values(int);
        quint64			Read_state(int);
		qint8			Read_marker(int);
		qint64			nb_data;
        int				size(void){ return dataset.size(); }

		int				Save(void);
		int				Load(void);
		
		CData()
		{
			maxSize = 1000000;
			Marker = 0;
		}
        virtual ~CData() {}
        

    	int maxSize;
    	float				timeUnit;
private:
        quint8				Marker;
public:
        QList<TAnalog_Data> dataset;
		
	friend class dialogAnalog;
};

QDataStream &operator<<(QDataStream &out, const CData &dataplot);
QDataStream &operator>>(QDataStream &in, CData &dataplot);





#endif /* ANALOG_H */
