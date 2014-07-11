#ifndef TIMELINE_H
#define TIMELINE_H

#include <QSlider>
#include "anim.h"

class TimeLine : public QSlider
{
    Q_OBJECT
public:
    explicit TimeLine(QWidget *parent = 0);
    void setAnim(Pose* pose, int boneID);

signals:

public slots:

protected:
    Pose *curPose;
    int bone;

    virtual void paintEvent(QPaintEvent *);
};

#endif // TIMELINE_H
