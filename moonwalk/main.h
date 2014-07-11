#ifndef MAIN_H
#define MAIN_H

#include "animationwindow.h"
#include "levelwindow.h"

class Supervisor: public QObject {
    Q_OBJECT
private:
    AnimationWindow *aw;
    LevelWindow *lw;
public:
    Supervisor();
    void SetAnimation(AnimationWindow *a);
    void SetLevel(LevelWindow *l);
public slots:
    void Animation();
    void Level();
    void RmAnimation();
    void RmLevel();
};

#endif // MAIN_H
