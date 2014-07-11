#include "drawarea.h"
#include <QPainter>
#include <QMouseEvent>

DrawArea::DrawArea(QWidget *parent) :
    QWidget(parent), curButton(Qt::NoButton)
{
}

void DrawArea::paintEvent(QPaintEvent *pe)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    emit customPaint(&painter, width(), height());
}

void DrawArea::mouseMoveEvent ( QMouseEvent * event )
{
    QPointF dm = event->posF() - mousePos;
    mousePos = event->posF();
    mod = event->modifiers();
    if(curButton == Qt::LeftButton) emit dragLeftMouse(dm.x(), dm.y());
    if(curButton == Qt::RightButton) emit dragRightMouse(dm.x(), dm.y());
    if(curButton == Qt::MiddleButton) emit dragMiddleMouse(dm.x(), dm.y());
}

void DrawArea::mousePressEvent ( QMouseEvent * event )
{
    curButton = event->button();
    mousePos = event->posF();
    mod = event->modifiers();
    if(curButton == Qt::LeftButton) emit pressLeftMouse(mousePos.x(), mousePos.y());
    if(curButton == Qt::RightButton) emit pressRightMouse(mousePos.x(), mousePos.y());
    if(curButton == Qt::MiddleButton) emit pressMiddleMouse(mousePos.x(), mousePos.y());
}

void DrawArea::mouseReleaseEvent ( QMouseEvent * event )
{
    curButton = Qt::NoButton;
    mod = event->modifiers();
}

//void DrawArea::keyPressEvent ( QKeyEvent * event )
//{
//    mod = event->modifiers();
//}

//void DrawArea::keyReleaseEvent ( QKeyEvent * event )
//{
//    mod = event->modifiers();
//}
