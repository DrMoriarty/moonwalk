#ifndef LEVELWINDOW_H
#define LEVELWINDOW_H

#include <QtGui>
#include <QMainWindow>
#include <QGraphicsItem>
#include <QGraphicsScene>

typedef enum {ET_NONE = 0, ET_IMAGE, ET_POLYGON} ElementType;
typedef enum {PY_NONE = 0, PY_STATIC, PY_DYNAMIC} ElementPhysic;

class Element {
protected:
    ElementType type;
    int offsetX, offsetY;
    QGraphicsItem *grit;
public:
    int x, y;
    double z, scale;
    ElementPhysic phys;
    static QString BasePath;

    Element();
    virtual ~Element();
    void Position(int posX, int posY);
    ElementType getType();
    virtual void Update();
    virtual void Save(QXmlStreamWriter* writer) = 0;
    virtual QRect getRect() = 0;
    virtual QString getName() = 0;
    virtual void select(int) = 0;
    virtual int size() = 0;
    virtual void movePoint(int p, float dx, float dy) = 0;
    virtual int addPoint(int id) = 0;
    virtual int delPoint(int id) = 0;
};

class ElementImage : public Element {
protected:
    QGraphicsPixmapItem *gr;
    QGraphicsPolygonItem *poly;
    QGraphicsRectItem *cursor;
    QString filename;
    int curP;
    int w, h;
public:
    ElementImage(QString filename, QGraphicsScene *scene);
    ElementImage(QXmlStreamReader *reader, QGraphicsScene *scene, double par);
    virtual ~ElementImage();
    virtual void Update();
    virtual void Save(QXmlStreamWriter* writer);
    virtual QRect getRect();
    virtual QString getName();
    virtual void select(int);
    virtual int size();
    virtual void movePoint(int p, float dx, float dy);
    virtual int addPoint(int id);
    virtual int delPoint(int id);
};

class ElementPolygon : public Element {
protected:
    QGraphicsPolygonItem *gr;
    QGraphicsRectItem *cursor;
    QString texfile;
    int curP;
public:
    ElementPolygon(QString texfile, QGraphicsScene *scene);
    ElementPolygon(QXmlStreamReader *reader, QGraphicsScene *scene, double par);
    virtual ~ElementPolygon();
    virtual void Update();
    virtual void Save(QXmlStreamWriter* writer);
    virtual QRect getRect();
    virtual QString getName();
    virtual void select(int);
    virtual int size();
    virtual void movePoint(int p, float dx, float dy);
    virtual int addPoint(int id);
    virtual int delPoint(int id);
};

namespace Ui {
    class LevelWindow;
}

class LevelWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LevelWindow(QWidget *parent = 0);
    ~LevelWindow();

signals:
    void raiseAnimation();
    void raiseLevel();

public slots:
    void setElementType(int);
    void addElement();
    void delElement();
    void selectElement();
    void setX(int);
    void setY(int);
    void setZ(double);
    void setScale(double);
    void setPhysics(int);
    void scrollWindowX(int);
    void scrollWindowY(int);
    void setPolygonPoint(int);
    void editPolygon(bool);
    void addPolygonPoint();
    void delPolygonPoint();

    void newLevel();
    void openLevel();
    void saveLevel();
    void saveLevelAs();

    void updateFrame();

private slots:
    void leftDrag(float, float);
    void rightDrag(float, float);
    void middleDrag(float, float);
    void leftPress(float, float);
    void rightPress(float, float);
    void middlePress(float, float);

private:
    Ui::LevelWindow *ui;
    int nextElID;
    QGraphicsScene scene;
    ElementType curType;
    QString levelName, levelFile;
    QMap<int, Element*> elements;
    bool SelectElement;
    int windowX, windowY;
    QGraphicsRectItem *background;
    int curPolygonPoint;
    Element *curElement;
    bool polygonPointMode;

    int CurrentElementID();
    void updateWindowSize();
    void updateWindowPos();
    void clear();
    void saveLevelTo(QString file);

protected:
    void closeEvent(QCloseEvent *event);

};

#endif // LEVELWINDOW_H
