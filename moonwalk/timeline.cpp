#include "timeline.h"
#include <QPainter>
#include <QStyle>

TimeLine::TimeLine(QWidget *parent) :
    QSlider(parent),
    curPose(0),
    bone(0)
{
}

void TimeLine::paintEvent(QPaintEvent *pe)
{
    QSlider::paintEvent(pe);
    if(!curPose || !bone) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int steps = maximum() - minimum();
    int offset = 10;//style()->pixelMetric(QStyle::PM_SliderSpaceAvailable);
    float stepW = (float)(width() - 2*offset ) / steps;
    float pos = offset - stepW/2;
    painter.setPen(Qt::NoPen);
    for(int i=0; i<=steps; i++, pos += stepW) {
        switch(curPose->IsKey(i + minimum(), bone)) {
        case 0:
        default:
            continue;
        case 1:
            painter.setBrush(QBrush(Qt::darkGreen));
            break;
        case -1:
            painter.setBrush(QBrush(Qt::gray));
            break;
        }
        painter.drawRect(QRectF(pos, height()-5, stepW, 5));
    }
}

void TimeLine::setAnim(Pose *pose, int boneID)
{
    curPose = pose;
    bone = boneID;
    update();
}
