#include <QCoreApplication>
#include "main.h"

Supervisor::Supervisor() : aw(0), lw(0)
{
}

void Supervisor::SetAnimation(AnimationWindow* a)
{
    if(aw) delete aw;
    aw = a;
}

void Supervisor::SetLevel(LevelWindow* l)
{
    if(lw) delete lw;
    lw = l;
}

void Supervisor::Animation()
{
    if(!aw) {
        aw = new AnimationWindow();
        aw->show();
        connect(aw, SIGNAL(destroyed()), this, SLOT(RmAnimation()));
        connect(aw, SIGNAL(raiseAnimation()), this, SLOT(Animation()));
        connect(aw, SIGNAL(raiseLevel()), this, SLOT(Level()));
    } else {
        aw->raise();
    }
}

void Supervisor::Level()
{
    if(!lw) {
        lw = new LevelWindow();
        lw->show();
        connect(lw, SIGNAL(destroyed()), this, SLOT(RmLevel()));
        connect(lw, SIGNAL(raiseAnimation()), this, SLOT(Animation()));
        connect(lw, SIGNAL(raiseLevel()), this, SLOT(Level()));
    } else {
        lw->raise();
    }
}

void Supervisor::RmAnimation()
{
    if(aw) aw->disconnect();
    aw = 0;
}

void Supervisor::RmLevel()
{
    if(lw) lw->disconnect();
    lw = 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //AnimationWindow w;
    //w.show();
    Supervisor s;
    //s.Animation();
    s.Level();
    return a.exec();
}
