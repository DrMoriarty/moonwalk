#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "anim.h"

namespace Ui {
    class AnimationWindow;
}

class AnimationWindow : public QMainWindow
{
    Q_OBJECT
private:
    int boneID, poseID;

    QTreeWidgetItem* curBone;
    QListWidgetItem* curPose;
    QMap<int, Bone*> bones;
    QMap<int, Pose*> poses;
    QVector<Bone*> sortedBones;
    QString personFileName;
    bool defaultPose;
    int curFrame;
    float viewScale;
    QTimer playTimer;
    QMap<int, Key> copiedKey;
    QFileSystemModel fsModel;
    bool drawBackground;
    QPixmap background;
    QString fileBackground;
    bool DeepCopy;
    int Ghosts, GhostStep;
    enum {RULER_NO, RULER_YES, RULER_SET} ruler;
    float rulerPos;
    int stepSize;
    bool _showValues_;

    void showValues();
    void areaRotate(float dA);
    void alignMousePos(float& dx, float& dy, bool reset = false);
    void sortBones();
    void drawSortedBones(QPainter* painter, Pose* pose, float frame);

public:
    explicit AnimationWindow(QWidget *parent = 0);
    ~AnimationWindow();

signals:
    void raiseAnimation();
    void raiseLevel();

public slots:
    void loadPerson();
    void savePerson();
    void savePersonAs(QString file);
    void savePersonNewFile();
    void newPerson();
    void boneAdd();
    void boneDel();
    void poseAdd();
    void poseDel();
    void selectBone(QTreeWidgetItem*,int);
    void selectPose(QListWidgetItem*);
    void drawArea(QPainter* painter, int width, int height);
    void setX(double);
    void setY(double);
    void setAngle(double);
    void setAnchorX(double);
    void setAnchorY(double);
    void dragLeftMouse(float dx, float dy);
    void dragRightMouse(float dx, float dy);
    void dragMiddleMouse(float dx, float dy);
    void pressRightMouse(float x, float y);
    void pressLeftMouse(float x, float y);
    void setZCoord(int);
    void setFrame(int);
    void setViewScale(int);
    void setAnimationLength(int);
    void renamePose(QListWidgetItem*);
    void playPose(bool);
    void playFrame();
    void delKeyFrame();
    void prevKeyFrame();
    void nextKeyFrame();
    void copyKeyFrame();
    void pasteKeyFrame();
    void moveBoneUp();
    void moveBoneDown();
    void previewImageFile(QModelIndex);
    void reloadGraphics();
    void setImageForBone();
    void setBackground(bool);
    void setVisibleObjects(int);
    void disableKeyFrame(bool);
    void setDeepCopyFlag(bool);
    void moveKeyFrameBw();
    void moveKeyFrameFw();
    void setGhost();
    void SetNextFrame();
    void SetPrevFrame();
    void SetFirstFrame();
    void SetLastFrame();
    void setRuler(bool);
    void Undo();
    void poseCopy();

    void updateInterface();
    void stepN();
    void stepS();
    void stepW();
    void stepE();
    void stepNE();
    void stepNW();
    void stepSE();
    void stepSW();
    void stepMore();
    void stepLess();
private:
    Ui::AnimationWindow *ui;

protected:
    void closeEvent(QCloseEvent *event);
    void wheelEvent ( QWheelEvent * event );
};

#endif // MAINWINDOW_H
