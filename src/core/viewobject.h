#ifndef VIEWOBJECT_H
#define VIEWOBJECT_H

#include <QWidget>

class Ckeyb;

enum Direction { TOPdir,LEFTdir, RIGHTdir, BOTTOMdir, NONEdir};
enum View {FRONTview,TOPview,LEFTview,RIGHTview,BOTTOMview,BACKview};

class CViewObject: public QWidget {
    Q_OBJECT

    Q_PROPERTY(int angle READ angle WRITE setAngle)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom)

public:

    CViewObject(CViewObject *parent=0);
    virtual ~CViewObject();

    Ckeyb		*pKEYB;

    QString FrontFname,TopFname,LeftFname,RightFname,BottomFname,BackFname;
    QImage *FrontImage,*TopImage,*LeftImage,*RightImage,*BottomImage,*BackImage;
    QImage* FinalImage;
    QPixmap mask;
    View currentView,targetView,animationView1,animationView2;
    Direction currentFlipDir;
    bool flipping;

    void setAngle(int value);
    int angle() const { return m_angle; }
    int m_angle;

    void setZoom(qreal value);
    qreal zoom() const { return m_zoom; }
    qreal m_zoom;

    Direction borderClick(QPoint pt);
    QSize viewRect(View v);
    QSize currentViewRect();

    QSize targetSize;
    QImage *getViewImage(View v);

    float	posx();
    float   posy();
    void    setPosX(float);
    void    setPosY(float);
    void    Move(QPoint);
    QRect   rect();
    QPoint  pos();

    float	PosX,PosY;

    int     getDX();
    int     getDY();
    int     getDZ();
    void    setDX(int v);
    void    setDY(int v);
    void    setDZ(int v);

    int     getDXmm() {return Pc_DX_mm;}
    int     getDYmm() {return Pc_DY_mm;}
    int     getDZmm() {return Pc_DZ_mm;}
    void    setDXmm(int v) {Pc_DX_mm = v;}
    void    setDYmm(int v) {Pc_DY_mm = v;}
    void    setDZmm(int v) {Pc_DZ_mm = v;}
    int		Pc_DX,Pc_DY;
    int		Pc_DX_mm,Pc_DY_mm,Pc_DZ_mm;		// PC size (mm




    virtual bool InitDisplay(void);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void flip(Direction dir);
    virtual bool UpdateFinalImage(void) { return true;}

    static QImage * CreateImage(QSize size, QString fname, bool Hmirror=false, bool Vmirror=false, int angle=0);
    void changeGeometry(int newposx, int newposy, int newwidth, int newheight);
    void changeGeometrySize(int newposx, int newposy, int newwidth, int newheight);
signals:
    void movePObject(CViewObject*,QPoint);
    void sizePObject(CViewObject*,QSize);

public slots:
    void endAnimation();
};

#endif // VIEWOBJECT_H
