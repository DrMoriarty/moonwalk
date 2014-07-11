#include "mygraphicsview.h"
#include <QMouseEvent>

MyGraphicsView::MyGraphicsView(QWidget *parent) :
    QGraphicsView(parent), curButton(Qt::NoButton)
{
}

MyGraphicsView::MyGraphicsView(QGraphicsScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent), curButton(Qt::NoButton)
{
}

void MyGraphicsView::mouseMoveEvent ( QMouseEvent * event )
{
    QPointF dm = event->localPos() - mousePos;
    mousePos = event->localPos();
    mod = event->modifiers();
    if(curButton == Qt::LeftButton) emit dragLeftMouse(dm.x(), dm.y());
    if(curButton == Qt::RightButton) emit dragRightMouse(dm.x(), dm.y());
    if(curButton == Qt::MiddleButton) emit dragMiddleMouse(dm.x(), dm.y());
}

void MyGraphicsView::mousePressEvent ( QMouseEvent * event )
{
    curButton = event->button();
    mousePos = event->localPos();
    mod = event->modifiers();
    if(curButton == Qt::LeftButton) emit pressLeftMouse(mousePos.x(), mousePos.y());
    if(curButton == Qt::RightButton) emit pressRightMouse(mousePos.x(), mousePos.y());
    if(curButton == Qt::MiddleButton) emit pressMiddleMouse(mousePos.x(), mousePos.y());
}

void MyGraphicsView::mouseReleaseEvent ( QMouseEvent * event )
{
    curButton = Qt::NoButton;
    mod = event->modifiers();
}
