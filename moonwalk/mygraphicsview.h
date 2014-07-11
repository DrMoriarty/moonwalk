#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
private:
    QPointF mousePos;
    Qt::MouseButton curButton;

public:
    int mod;
    MyGraphicsView(QWidget *parent = 0);
    MyGraphicsView(QGraphicsScene* scene, QWidget* parent = 0);

signals:
    void dragLeftMouse(float, float);
    void dragRightMouse(float, float);
    void dragMiddleMouse(float, float);
    void pressLeftMouse(float, float);
    void pressRightMouse(float, float);
    void pressMiddleMouse(float, float);

public slots:

protected:
    virtual void	mouseMoveEvent ( QMouseEvent * event );
    virtual void	mousePressEvent ( QMouseEvent * event );
    virtual void	mouseReleaseEvent ( QMouseEvent * event );
};

#endif // MYGRAPHICSVIEW_H
