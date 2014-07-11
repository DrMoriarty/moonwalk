#include "animationwindow.h"
#include "ui_animationwindow.h"
#include <QDomDocument>
#include <QShortcut>
#include <QMessageBox>
#include <QFileDialog>
#include <assert.h>

#define COMPANY "HeartGames"
#define APPNAME "AnimationEditor"
#define VERSION 2

/////////////////
// AnimationWindow
/////////////////

AnimationWindow::AnimationWindow(QWidget *parent) :
    QMainWindow(parent),
    boneID(1),
    poseID(1),
    curBone(0),
    curPose(0),
    defaultPose(true),
    curFrame(0),
    viewScale(1.f),
    DeepCopy(false),
    Ghosts(0),
    GhostStep(0),
    ruler(RULER_NO),
    rulerPos(0.f),
    stepSize(8),
    _showValues_(false),
    ui(new Ui::AnimationWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);

    const QObjectList& tools = ui->widget_tools->children();
    foreach(QObject *obj, tools) {
        QWidget *w = qobject_cast<QWidget*>(obj);
        if(w) ui->mainToolBar->addWidget(w);
    }
    QMenu *wnd = createPopupMenu();
    wnd->setTitle(tr("Window"));
    ui->menuBar->addMenu(wnd);

    QShortcut *sk = new QShortcut(QKeySequence(Qt::Key_1), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepSW()));
    sk = new QShortcut(QKeySequence(Qt::Key_2), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepS()));
    sk = new QShortcut(QKeySequence(Qt::Key_3), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepSE()));
    sk = new QShortcut(QKeySequence(Qt::Key_4), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepW()));
    sk = new QShortcut(QKeySequence(Qt::Key_6), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepE()));
    sk = new QShortcut(QKeySequence(Qt::Key_7), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepNW()));
    sk = new QShortcut(QKeySequence(Qt::Key_8), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepN()));
    sk = new QShortcut(QKeySequence(Qt::Key_9), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepNE()));
    sk = new QShortcut(QKeySequence(Qt::Key_Plus), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepMore()));
    sk = new QShortcut(QKeySequence(Qt::Key_Minus), this, 0, 0, Qt::ApplicationShortcut);
    connect(sk, SIGNAL(activated()), this, SLOT(stepLess()));

    connect(ui->actionAnimation, SIGNAL(activated()), this, SIGNAL(raiseAnimation()));
    connect(ui->actionLevel, SIGNAL(activated()), this, SIGNAL(raiseLevel()));

    connect(&playTimer, SIGNAL(timeout()), this, SLOT(playFrame()));
    fsModel.setRootPath(QDir::homePath());
    ui->treeView_fs->setModel(&fsModel);
    ui->treeView_fs->setRootIndex(fsModel.index(QDir::homePath()));

    QSettings set(COMPANY, APPNAME);
    this->restoreGeometry(set.value("window/geometry").toByteArray());
    this->restoreState(set.value("window/state").toByteArray(), VERSION);
    newPerson();
}

AnimationWindow::~AnimationWindow()
{
    delete ui;
}

void AnimationWindow::closeEvent(QCloseEvent *event)
{
    QSettings set(COMPANY, APPNAME);
    set.setValue("window/geometry", saveGeometry());
    set.setValue("window/state", saveState(VERSION));
    if(bones.size() > 0) {
        QMessageBox *msg = new QMessageBox(QMessageBox::Question, tr("Close program"), tr("Save animation before quit?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        int res = msg->exec();
        if(res == QMessageBox::Cancel) event->ignore();
        else {
            if(res == QMessageBox::Save) savePerson();
            event->accept();
        }
    } else {
        event->accept();
    }
}

void AnimationWindow::loadPerson()
{
    boneID = 1;
    defaultPose = true;
    QString filename = QFileDialog::getOpenFileName(this, tr("Load animation"), Bone::BasePath, tr("Xml file (*.xml)"));
    if(filename.isEmpty()) return;
    personFileName = filename;
    setWindowTitle(tr("MoonTools/Animation - %1").arg(QFileInfo(personFileName).fileName()));
    newPerson();
    QFile input(filename);
    QFileInfo fi(input);
    Bone::BasePath = fi.absolutePath();
    QDir().cd(Bone::BasePath);
    input.open(QIODevice::ReadOnly);

    QXmlStreamReader stream(&input);
    Bone *root = 0;
    QVector<QTreeWidgetItem*> branches;
    int curPoseBoneID = 0;
    Pose *lastPose = 0;
    while (!stream.atEnd()) {
        stream.readNext();
        if(stream.isStartElement()) {
            if(stream.name() == "bone") {
                Bone *nb = new Bone(&stream, root);
                bones[nb->ID] = nb;
                root = nb;
                QTreeWidgetItem *ni = 0;
                if(branches.size()) ni = new QTreeWidgetItem(branches.back(), QStringList(nb->Name()));
                else {
                    ni = new QTreeWidgetItem(ui->treeWidget, QStringList(nb->Name()));
                }
                ni->setData(0, Qt::UserRole, nb->ID);
                branches.push_back(ni);
                if(boneID <= nb->ID) boneID = nb->ID + 1;
            } else if(stream.name() == "pose") {
                Pose *ps = new Pose(&stream);
                lastPose = ps;
                poses[poseID] = ps;
                QListWidgetItem *li = new QListWidgetItem(ps->name, ui->listWidget);
                li->setData(Qt::UserRole, poseID++);
                li->setFlags(li->flags() | Qt::ItemIsEditable);
            } else if(stream.name() == "keys") {
                curPoseBoneID = stream.attributes().value("bone").toString().toInt();
            } else if(stream.name() == "key") {
                assert(lastPose);
                assert(curPoseBoneID > 0);
                Key k(&stream);
                lastPose->SetKey(curPoseBoneID, k);
            }
        }
        if(stream.isEndElement()) {
            if(stream.name() == "bone") {
                assert(root);
                root = root->Parent();
                assert(branches.size());
                branches.pop_back();
            } else if(stream.name() == "pose") {
                lastPose = 0;
            } else if(stream.name() == "keys") {
                curPoseBoneID = 0;
            } else if(stream.name() == "key") {
            }
        }
    }
    if (stream.hasError()) {
        // TODO
    } else {
        if(ui->treeWidget->topLevelItem(0)) {
            //int rootID = ui->treeWidget->topLevelItem(0)->data(0, Qt::UserRole).toInt();
            ui->treeWidget->topLevelItem(0)->setSelected(true);
            ui->treeWidget->expandAll();
            selectBone(ui->treeWidget->topLevelItem(0), 0);
        }
    }
    input.close();
	sortBones();
}

void AnimationWindow::savePerson()
{
    if(personFileName.isEmpty()) personFileName = QFileDialog::getSaveFileName(this, tr("Save animation"), Bone::BasePath, tr("Xml file (*.xml)"));
    savePersonAs(personFileName);
}

void AnimationWindow::savePersonNewFile()
{
    personFileName = QFileDialog::getSaveFileName(this, tr("Save animation"), Bone::BasePath, tr("Xml file (*.xml)"));
    savePersonAs(personFileName);
}

void AnimationWindow::savePersonAs(QString file)
{
    if(file.isEmpty()) return;
    setWindowTitle(tr("MoonTools/Animation - %1").arg(QFileInfo(file).fileName()));
    int rootID = -1;
    if(ui->treeWidget->topLevelItem(0)) {
        rootID = ui->treeWidget->topLevelItem(0)->data(0, Qt::UserRole).toInt();
    }
    if(rootID <= 0) return;
    QFile output(file);
    Bone::BasePath = QFileInfo(output).absolutePath();
    QDir().cd(Bone::BasePath);
    output.open(QIODevice::WriteOnly);
    QXmlStreamWriter stream(&output);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Person");
    stream.writeAttribute("name", "NamelessPerson");
    bones[rootID]->Save(&stream);
    foreach(Pose* p, poses) {
        p->Save(&stream);
    }
    stream.writeEndElement(); // Person
    stream.writeEndDocument();
    output.close();
}

void AnimationWindow::newPerson()
{
    if(ui->treeWidget->topLevelItem(0)) {
        int rootID = ui->treeWidget->topLevelItem(0)->data(0, Qt::UserRole).toInt();
        delete bones[rootID];
    }
    ui->treeWidget->clear();
    ui->listWidget->clear();
    bones.clear();
    poses.clear();
    curBone = 0;
    curPose = 0;
    boneID = 1;
    poseID = 1;
    ui->drawArea->update();
    defaultPose = true;

    QListWidgetItem *defPose = new QListWidgetItem("Default");
    defPose->setData(Qt::UserRole, 0);
    ui->listWidget->addItem(defPose);
}

void AnimationWindow::boneAdd()
{
    //QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), Bone::BasePath, tr("Images (*.png *.xpm *.jpg)"));
    //if(filename.isNull()) return;
    Bone *par = 0;
    if(curBone) {
        par = bones[curBone->data(0, Qt::UserRole).toInt()];
    }
    Bone *nb = new Bone(boneID, par);
    QTreeWidgetItem *ni = 0;
    if(curBone) ni = new QTreeWidgetItem(curBone, QStringList(nb->Name()));
    else {
        ni = new QTreeWidgetItem(ui->treeWidget, QStringList(nb->Name()));
    }
    ni->setData(0, Qt::UserRole, boneID);
    bones[boneID++] = nb;
    if(curBone) curBone->setSelected(false);
    ni->setSelected(true);
    ui->treeWidget->expandAll();
    selectBone(ni, 0);
	sortBones();
}

void AnimationWindow::boneDel()
{
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    delete curBone;
    QVector<int> ch = bones[curID]->childrenIDs();
    foreach(int i, ch) {
        delete bones[i];
        bones.erase(bones.find(i));
    }
    delete bones[curID];
    bones.erase(bones.find(curID));
    curBone = 0;
    ui->drawArea->update();
    if(ui->treeWidget->selectedItems().size())
        selectBone(ui->treeWidget->selectedItems()[0], 0);
    else
        updateInterface();
    ui->drawArea->update();
}

void AnimationWindow::poseAdd()
{
    QString pName = QString("Pose%1").arg(poseID);
    poses[poseID] = new Pose(pName);
    QListWidgetItem* it = new QListWidgetItem(pName, ui->listWidget);
    it->setData(Qt::UserRole, poseID++);
    it->setFlags(Qt::ItemIsEditable | it->flags());
    updateInterface();
}

void AnimationWindow::poseDel()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    delete curPose;
    delete poses[pid];
    poses.erase(poses.find(pid));
    if(ui->listWidget->selectedItems().size()) {
        selectPose(ui->listWidget->selectedItems()[0]);
    }
}

void AnimationWindow::showValues()
{
    _showValues_ = true;
    if(curBone) {
        int curID = curBone->data(0, Qt::UserRole).toInt();
        Bone &b = *bones[curID];
        if(defaultPose) {
            ui->label_xValue->setText(QString::number(b.x));
            ui->label_yValue->setText(QString::number(b.y));
            ui->doubleSpinBox_angle->setValue(b.angle);
            //ui->label_angleValue->setText(QString::number(b.angle));
            ui->label_xPivot->setText(QString::number(b.pivotX));
            ui->label_yPivot->setText(QString::number(b.pivotY));
        } else if(curPose) {
            int pid = curPose->data(Qt::UserRole).toInt();
            if(pid > 0) {
                Key k = poses[pid]->CalcKey(curFrame, b.ID);
                ui->label_xValue->setText(QString::number(k.x));
                ui->label_yValue->setText(QString::number(k.y));
                ui->doubleSpinBox_angle->setValue(k.angle);
                //ui->label_angleValue->setText(QString::number(k.angle));
            }
        }
    }
    _showValues_ = false;
}

void AnimationWindow::selectBone(QTreeWidgetItem* it, int n)
{
    if(curBone) {
        int oldID = curBone->data(0, Qt::UserRole).toInt();
        bones[oldID]->selected = false;
    }
    curBone = it;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    bones[curID]->selected = true;
    ui->label_boneName->setText(bones[curID]->Name());
	ui->spinBox_zCoord->setValue(bones[curID]->z);
    ui->timeline->setAnim(defaultPose || !curPose ? 0 : poses[curPose->data(Qt::UserRole).toInt()], curID);
    showValues();
    updateInterface();
    ui->drawArea->update();
}

void AnimationWindow::selectPose(QListWidgetItem* it)
{
    curPose = it;
    int poseID = curPose->data(Qt::UserRole).toInt();
    if(poseID <= 0) {
        defaultPose = true;
        ui->timeline->setValue(0);
        ui->timeline->setAnim(0, 0);
    } else {
        defaultPose = false;
        ui->timeline->setAnim(poses[poseID], curBone ? curBone->data(0, Qt::UserRole).toInt() : 0);
    }
    showValues();
    updateInterface();
    ui->drawArea->update();
}

void AnimationWindow::drawSortedBones(QPainter* painter, Pose* pose, float frame)
{
	if(Bone::DRAW_GRAPHICS) {
		foreach(Bone* b, sortedBones) {
			b->DrawGraphics(painter, pose, frame);
		}
	}
	if(Bone::DRAW_SKELETON) {
		foreach(Bone *b, sortedBones) {
			b->DrawSkeleton(painter, pose, frame);
		}
	}
}


void AnimationWindow::drawArea(QPainter *painter, int width, int height)
{
    Pose *pose = 0;
    if(curPose) {
        int pid = curPose->data(Qt::UserRole).toInt();
        if(pid > 0) pose = poses[pid];
    }
    painter->drawLine(width/2, 0, width/2, height);
    painter->drawLine(0, height/2, width, height/2);
    painter->translate(width * .5f, height * .5f);
    painter->scale(viewScale, viewScale);
    if(drawBackground) {
        painter->setOpacity(0.5f);
        painter->drawPixmap(-background.width()/2, -background.height()/2, background);
        painter->setOpacity(1.f);
    }
    if(ruler == RULER_YES) {
        painter->save();
        painter->setPen(Qt::blue);
        painter->drawLine(-width/2, rulerPos, width/2, rulerPos);
        painter->restore();
    }
    if(!bones.size()) return;
    //int rootID = ui->treeWidget->topLevelItem(0)->data(0, Qt::UserRole).toInt();
    if(Ghosts && GhostStep) {
        float dAlpha = 1.f / (Ghosts + 1);
        float alpha = dAlpha;
        for(int frame = curFrame - Ghosts * GhostStep; frame <= curFrame; frame += GhostStep, alpha += dAlpha) {
            if(frame >= 0) {
                painter->setOpacity(alpha);
                //bones[rootID]->Draw(painter, pose, frame);
				drawSortedBones(painter, pose, frame);
            }
        }
    } else {
        //bones[rootID]->Draw(painter, pose, curFrame);
		drawSortedBones(painter, pose, curFrame);
    }
}

void AnimationWindow::setX(double x)
{
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    if(defaultPose) {
        bones[curID]->x = x;
    } else {
//        int pid = curPose->data(Qt::UserRole).toInt();
//        if(pid > 0) {
//            poses[pid]->ChangeKey(curFrame, bones[curID]->name, x, 0.f, 0.f);
//        }
    }
    ui->drawArea->update();
}

void AnimationWindow::setY(double y)
{
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    if(defaultPose) {
        bones[curID]->y = y;
    } else {
//        int pid = curPose->data(Qt::UserRole).toInt();
//        if(pid > 0) {
//            poses[pid]->ChangeKey(curFrame, bones[curID]->name, 0.f, y, 0.f);
//        }
    }
    ui->drawArea->update();
}

void AnimationWindow::setAngle(double angle)
{
    if(!curBone || _showValues_) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    if(defaultPose) {
        bones[curID]->angle = angle;
    } else {
        int pid = curPose->data(Qt::UserRole).toInt();
        Key k = poses[pid]->CalcKey(curFrame, curID);
        float delta = angle - k.angle;
        poses[pid]->ChangeKey(curFrame, curID, 0.f, 0.f, delta);
    }
    ui->drawArea->update();
}

void AnimationWindow::setAnchorX(double x)
{
    if(!curBone || !defaultPose) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    bones[curID]->pivotX = x;
    ui->drawArea->update();
}

void AnimationWindow::setAnchorY(double y)
{
    if(!curBone || !defaultPose) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    bones[curID]->pivotY = y;
    ui->drawArea->update();
}

void AnimationWindow::dragLeftMouse(float dx, float dy)
{
    playPose(false);
    if(ui->drawArea->mod & Qt::ALT) {
        areaRotate(dy / viewScale);
        return;
    }
    alignMousePos(dx, dy);
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    Bone &b = *bones[curID];
    int pid = 0;
    if(!defaultPose && curPose) pid = curPose->data(Qt::UserRole).toInt();
    float dX = 0.f, dY = 0.f, dA = 0.f;
    {
        float ang = 0.f;
        Bone * par = b.Parent();
        while(par) {
            ang += par->angle;
            if(pid) {
                Key k = poses[pid]->CalcKey(curFrame, par->ID);
                ang += k.angle;
            }
            par = par->Parent();
        }
        QTransform tr;
        tr = tr.rotate(-ang);
        QPointF p(dx, dy);
        p = tr.map(p);
        dX = p.x() / viewScale;
        dY = p.y() / viewScale;
    }
    if(defaultPose) {
        b.angle += dA;
        if(b.angle < -360) b.angle = -360;
        if(b.angle > 360) b.angle = 360;
        b.x += dX;
        b.y += dY;
    } else if(curPose) {
        if(pid > 0) {
            bool oldKey = poses[pid]->IsKey(curFrame, b.ID);
            poses[pid]->ChangeKey(curFrame, b.ID, dX, dY, dA);
            if(!oldKey) {
                updateInterface();
                ui->timeline->update();
            }
        }
    }
    showValues();
    ui->drawArea->update();
}

void AnimationWindow::pressRightMouse(float x, float y)
{
    alignMousePos(x, y, true);
    if(!curBone || !defaultPose) {
        // select nearest bone
        int pid = 0;
        if(!defaultPose && curPose) pid = curPose->data(Qt::UserRole).toInt();
        Bone *nearest = 0;
        float distance = -1.f;
        QPointF mp(x-ui->drawArea->width()/2, y-ui->drawArea->height()/2);
        foreach(Bone *b, bones) {
            QPointF p = b->Map(QPoint(0,0), 0, poses[pid], curFrame);
            float dis = (p-mp).manhattanLength();
            if(distance < 0.f || distance > dis) {
                distance = dis;
                nearest = b;
            }
        }
        if(nearest && distance >= 0.f) {
            if(curBone) curBone->setSelected(false);
            QTreeWidgetItem *it = ui->treeWidget->findItems(nearest->Name(), Qt::MatchFixedString | Qt::MatchRecursive)[0];
            it->setSelected(true);
            selectBone(it, 0);
        }
        return;
    }
}

void AnimationWindow::pressLeftMouse(float x, float y)
{
    alignMousePos(x, y, true);
    if(ruler == RULER_SET) {
        ruler = RULER_YES;
        rulerPos = y - ui->drawArea->height()/2;
        ui->drawArea->update();
    }
}

void AnimationWindow::dragRightMouse(float dx, float dy)
{
    playPose(false);
    if(!defaultPose) return;
    alignMousePos(dx, dy);
    int curID = curBone->data(0, Qt::UserRole).toInt();
    float ang = 0.f;
    Bone *par = bones[curID];
    while(par) {
        ang += par->angle;
        par = par->Parent();
    }
    QTransform tr;
    tr = tr.rotate(-ang);
    QPointF p(dx, dy);
    p = tr.map(p);
    bones[curID]->pivotX += p.x() / viewScale;
    bones[curID]->pivotY += p.y() / viewScale;
    showValues();
    ui->drawArea->update();
}

void AnimationWindow::dragMiddleMouse(float dx, float dy)
{
    areaRotate(dy / viewScale);
}

void AnimationWindow::areaRotate(float dA)
{
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    Bone &b = *bones[curID];
    int pid = 0;
    if(!defaultPose && curPose) pid = curPose->data(Qt::UserRole).toInt();
    if(defaultPose) {
        b.angle += dA;
        if(b.angle < -360) b.angle = -360;
        if(b.angle > 360) b.angle = 360;
    } else if(curPose) {
        if(pid > 0) {
            bool oldKey = poses[pid]->IsKey(curFrame, b.ID);
            poses[pid]->ChangeKey(curFrame, b.ID, 0.f, 0.f, dA);
            if(!oldKey) {
                updateInterface();
                ui->timeline->update();
            }
        }
    }
    showValues();
    ui->drawArea->update();
}

void AnimationWindow::setZCoord(int z)
{
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    bones[curID]->z = z;
	sortBones();
}


void AnimationWindow::updateInterface()
{
    bool bs = curBone != NULL;
    bool be = bs && defaultPose;
    ui->spinBox_zCoord->setEnabled(be);

    ui->pushButton_boneAdd->setEnabled(defaultPose);
    ui->pushButton_boneDel->setEnabled(defaultPose);

    ui->actionPersonSave->setEnabled(bones.size());
    ui->actionPersonSave_as->setEnabled(bones.size());

    ui->widget_timeline->setEnabled(!defaultPose);
    if(curPose) {
        int pid = curPose->data(Qt::UserRole).toInt();
        if(pid > 0) {
            ui->timeline->setMaximum(poses[pid]->length);
            ui->spinBox_animationLength->setValue(poses[pid]->length);
            if(curBone) {
                int bid = curBone->data(0, Qt::UserRole).toInt();
                ui->frame_keyFrame->setEnabled(poses[pid]->IsKey(curFrame, bid));
                ui->pushButton_disableKF->setChecked(poses[pid]->IsKey(curFrame, bid) < 0);
            }
        }
    } else {
        ui->frame_keyFrame->setEnabled(false);
    }
    ui->spinBox_animationLength->setEnabled(!defaultPose);
}

void AnimationWindow::setFrame(int fr)
{
    curFrame = fr;
    showValues();
    updateInterface();
    ui->drawArea->update();
}

void AnimationWindow::setViewScale(int s)
{
    viewScale = 0.01f * s;
    ui->drawArea->update();
}

void AnimationWindow::setAnimationLength(int l)
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    poses[pid]->length = l;
    ui->timeline->setMaximum(l);
    ui->spinBox_currentFrame->setMaximum(l);
}

void AnimationWindow::renamePose(QListWidgetItem* it)
{
    //if(!curPose) return;
    //if(!ui->listWidget->selectedItems().size()) return;
    int pid = it->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    poses[pid]->name = it->text();
}

void AnimationWindow::playPose(bool play)
{
    if(ui->actionPlay_Stop->isChecked() != play) ui->actionPlay_Stop->setChecked(play);
    if(ui->pushButton_play->isChecked() != play) ui->pushButton_play->setChecked(play);
    if(play) {
        playTimer.start(40);
    } else {
        playTimer.stop();
    }
}

void AnimationWindow::playFrame()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    curFrame ++;
    if(curFrame > poses[pid]->length) curFrame = 0;
    ui->timeline->setValue(curFrame);
}

void AnimationWindow::delKeyFrame()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    QVector<int> children;
    children.push_back(curID);
    if(DeepCopy) for(int i=0; i<children.size(); i++) {
        children << bones[children[i]]->childrenIDs();
    }
    foreach(int ind, children) {
        Bone &b = *bones[ind];
        if(poses[pid]->IsKey(curFrame, b.ID)) {
            poses[pid]->DelKey(curFrame, b.ID);
        }
    }
    updateInterface();
    ui->timeline->update();
    ui->drawArea->update();
}

void AnimationWindow::nextKeyFrame()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    curFrame = poses[pid]->NextKey(curFrame, curID);
    ui->timeline->setValue(curFrame);
    updateInterface();
}

void AnimationWindow::prevKeyFrame()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    if(!curBone) return;
    int curID = curBone->data(0, Qt::UserRole).toInt();
    curFrame = poses[pid]->PrevKey(curFrame, curID);
    ui->timeline->setValue(curFrame);
    updateInterface();
}

void AnimationWindow::copyKeyFrame()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    if(!curBone) return;
    copiedKey.clear();
    int curID = curBone->data(0, Qt::UserRole).toInt();
    QVector<int> children;
    children.push_back(curID);
    if(DeepCopy) for(int i=0; i<children.size(); i++) {
        children << bones[children[i]]->childrenIDs();
    }
    foreach(int ind, children) {
        copiedKey[ind] = poses[pid]->CalcKey(curFrame, ind);
    }
}

void AnimationWindow::pasteKeyFrame()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    if(!curBone) return;
    foreach(int ind, copiedKey.keys()) {
        copiedKey[ind].frame = curFrame;
        poses[pid]->SetKey(ind, copiedKey[ind]);
    }
    ui->timeline->update();
    ui->drawArea->update();
}

void AnimationWindow::moveBoneUp()
{
    if(!curBone || !curBone->parent()) return;
    QTreeWidgetItem *tipar = curBone->parent();
    int newindex = tipar->indexOfChild(curBone)-1;
    if(newindex < 0) return;

    int curID = curBone->data(0, Qt::UserRole).toInt();
    Bone *par = bones[curID]->Parent();
    if(par) par->MoveUp(bones[curID]);

    ui->treeWidget->clearSelection();
    QTreeWidgetItem *ni;
    ni = curBone->clone();
    tipar->insertChild(newindex, ni);
    ni->setData(0, Qt::UserRole, curID);
    curBone->setSelected(false);
    delete curBone;
    curBone = ni;
    ui->treeWidget->expandAll();
    ui->drawArea->update();
}

void AnimationWindow::moveBoneDown()
{
    if(!curBone || !curBone->parent()) return;
    QTreeWidgetItem *tipar = curBone->parent();
    int newindex = tipar->indexOfChild(curBone)+2;
    if(newindex > tipar->childCount()) return;

    int curID = curBone->data(0, Qt::UserRole).toInt();
    Bone *par = bones[curID]->Parent();
    if(par) par->MoveDown(bones[curID]);

    ui->treeWidget->clearSelection();
    QTreeWidgetItem *ni;
    ni = curBone->clone();
    tipar->insertChild(newindex, ni);
    ni->setData(0, Qt::UserRole, curID);
    curBone->setSelected(false);
    delete curBone;
    curBone = ni;
    ui->treeWidget->expandAll();
    ui->drawArea->update();
}

void AnimationWindow::previewImageFile(QModelIndex ind)
{
   if(!fsModel.isDir(ind)) {
       ui->label_previewFile->setPixmap(QPixmap(fsModel.filePath(ind)));
   }
}

void AnimationWindow::reloadGraphics()
{
    foreach(Bone* b, bones) {
        b->ReloadGraphics();
    }
    ui->drawArea->update();
}

void AnimationWindow::setImageForBone()
{
    QModelIndex ind = ui->treeView_fs->currentIndex();
    if(ind.isValid() && !fsModel.isDir(ind) && curBone) {
        int curID = curBone->data(0, Qt::UserRole).toInt();
        bones[curID]->SetGraphics(fsModel.filePath(ind));
        curBone->setText(0, bones[curID]->Name());
    }
    ui->drawArea->update();
}

void AnimationWindow::setBackground(bool b)
{
    drawBackground = b;
    if(b) {
        QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QFileInfo(fileBackground).absolutePath(), tr("Images (*.png *.xpm *.jpg)"));
        if(filename.isNull()) {
            drawBackground = !background.isNull();
            return;
        } else fileBackground = filename;
        background.load(filename);
    }
    ui->drawArea->update();
}

void AnimationWindow::setVisibleObjects(int m)
{
    switch(m) {
    case 0: // both
        Bone::DRAW_SKELETON = true;
        Bone::DRAW_GRAPHICS = true;
        break;
    case 1: // skeleton
        Bone::DRAW_SKELETON = true;
        Bone::DRAW_GRAPHICS = false;
        break;
    case 2: // graphics
        Bone::DRAW_SKELETON = false;
        Bone::DRAW_GRAPHICS = true;
        break;
    }
    ui->drawArea->update();
}

void AnimationWindow::disableKeyFrame(bool disable)
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    if(!curBone) return;
    int bid = curBone->data(0, Qt::UserRole).toInt();
    QVector<int> ch;
    ch.push_back(bid);
    if(DeepCopy) for(int i=0; i<ch.size(); i++) {
        ch << bones[ch[i]]->childrenIDs();
    }
    foreach(int id, ch) {
        poses[pid]->Enable(curFrame, id, !disable);
    }

    ui->timeline->update();
    ui->drawArea->update();
}

void AnimationWindow::setDeepCopyFlag(bool d)
{
    DeepCopy = d;
}

void AnimationWindow::moveKeyFrameBw()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    if(!curBone) return;
    int bid = curBone->data(0, Qt::UserRole).toInt();
    int newTime = poses[pid]->ShiftKey(curFrame, bid, -1);
    if(newTime >= 0) {
        if(DeepCopy) {
            QVector<int> ch = bones[bid]->childrenIDs();
            for(int i=0; i<ch.size(); i++) {
                ch << bones[ch[i]]->childrenIDs();
            }
            foreach(int id, ch) {
                poses[pid]->ShiftKey(curFrame, id, -1);
            }
        }
        ui->timeline->setValue(newTime);
        ui->timeline->update();
    }
}

void AnimationWindow::moveKeyFrameFw()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;
    if(!curBone) return;
    int bid = curBone->data(0, Qt::UserRole).toInt();
    int newTime = poses[pid]->ShiftKey(curFrame, bid, 1);
    if(newTime >= 0) {
        if(DeepCopy) {
            QVector<int> ch = bones[bid]->childrenIDs();
            for(int i=0; i<ch.size(); i++) {
                ch << bones[ch[i]]->childrenIDs();
            }
            foreach(int id, ch) {
                poses[pid]->ShiftKey(curFrame, id, 1);
            }
        }
        ui->timeline->setValue(newTime);
        ui->timeline->update();
    }
}


void AnimationWindow::setGhost()
{
    if(ui->checkBox_ghost->isChecked()) {
        Ghosts = ui->spinBox_ghostNumber->value();
        GhostStep = ui->spinBox_ghostStep->value();
    } else {
        Ghosts = 0;
        GhostStep = 0;
    }
    ui->drawArea->update();
}

void AnimationWindow::wheelEvent(QWheelEvent *event)
{
    areaRotate(0.125f * event->delta());
}

void AnimationWindow::SetNextFrame()
{
    if(defaultPose) return;
    if(curFrame >= ui->timeline->maximum()) ui->timeline->setValue(ui->timeline->minimum());
    else ui->timeline->setValue(curFrame + 1);
}

void AnimationWindow::SetPrevFrame()
{
    if(defaultPose) return;
    if(curFrame <= ui->timeline->minimum()) ui->timeline->setValue(ui->timeline->maximum());
    else ui->timeline->setValue(curFrame - 1);
}

void AnimationWindow::SetLastFrame()
{
    if(defaultPose) return;
    if(curFrame >= ui->timeline->maximum()) return;
    ui->timeline->setValue(ui->timeline->maximum());
}

void AnimationWindow::SetFirstFrame()
{
    if(defaultPose) return;
    if(curFrame <= ui->timeline->minimum()) return;
    ui->timeline->setValue(ui->timeline->minimum());
}

void AnimationWindow::setRuler(bool r)
{
    if(r) ruler = RULER_SET;
    else {
        ruler = RULER_NO;
        ui->drawArea->update();
    }
}

void AnimationWindow::alignMousePos(float &dx, float &dy, bool reset)
{
    static QPointF mouseStart, mouseMove, mousePrev;
    if(reset) {
        mouseMove.setX(0.f);
        mouseMove.setY(0.f);
        mouseStart.rx() = dx;
        mouseStart.ry() = dy;
        mousePrev.rx() = dx;
        mousePrev.ry() = dy;
        return;
    }
    mouseMove.rx() += dx;
    mouseMove.ry() += dy;
    if(ui->drawArea->mod & Qt::SHIFT) {
		QPointF newp;
		float ax = fabs(mouseMove.x());
		float ay = fabs(mouseMove.y());
        if(ax > ay*3.f) {
            newp = QPointF(mouseMove.x() + mouseStart.x(), mouseStart.y());
        } else if(ay > ax*3.f) {
            newp = QPointF(mouseStart.x(), mouseMove.y() + mouseStart.y());
        } else {
			float dist = (ax + ay) * .5f;
			float distX = mouseMove.x() > 0.f ? dist : -dist;
			float distY = mouseMove.y() > 0.f ? dist : -dist;
            newp = QPointF(distX + mouseStart.x(), distY + mouseStart.y());
		}
		dx = newp.x() - mousePrev.x();
		dy = newp.y() - mousePrev.y();
		mousePrev = newp;
    } else {
		mousePrev = mouseMove + mouseStart;
	}
}

void AnimationWindow::sortBones()
{
	sortedBones.clear();
	foreach(Bone* b, bones) {
		sortedBones << b;
	}
	qSort(sortedBones.begin(), sortedBones.end(), BoneLessThan);
	ui->drawArea->update();
}

void AnimationWindow::Undo()
{
    sortBones();
}

void AnimationWindow::stepN()
{
    dragLeftMouse(0.f, -stepSize);
}

void AnimationWindow::stepS()
{
    dragLeftMouse(0.f, stepSize);
}

void AnimationWindow::stepW()
{
    dragLeftMouse(-stepSize, 0.f);
}

void AnimationWindow::stepE()
{
    dragLeftMouse(stepSize, 0.f);
}

void AnimationWindow::stepNE()
{
    dragLeftMouse(stepSize, -stepSize);
}

void AnimationWindow::stepNW()
{
    dragLeftMouse(-stepSize, -stepSize);
}

void AnimationWindow::stepSE()
{
    dragLeftMouse(stepSize, stepSize);
}

void AnimationWindow::stepSW()
{
    dragLeftMouse(-stepSize, stepSize);
}

void AnimationWindow::stepMore()
{
    stepSize *= 2;
}

void AnimationWindow::stepLess()
{
    if (stepSize > 1) stepSize /= 2;
}

void AnimationWindow::poseCopy()
{
    if(!curPose) return;
    int pid = curPose->data(Qt::UserRole).toInt();
    if(pid <= 0) return;

    QString pName = poses[pid]->name;
    if(!pName.size()) pName = QString("Pose%1").arg(poseID);
    else {
        bool found;
        do {
            found = false;
            foreach(Pose *p, poses) if(p->name == pName) {
                QChar ch = pName[pName.size()-1];
                if(ch < 'z') pName[pName.size()-1] = ch.toLatin1() + 1;
                else pName += "1";
                found = true;
            }
        } while (found);
    }
    poses[poseID] = new Pose(pName);
    QListWidgetItem* it = new QListWidgetItem(pName, ui->listWidget);
    it->setData(Qt::UserRole, poseID);
    it->setFlags(Qt::ItemIsEditable | it->flags());

    poses[poseID]->FillFrom(*poses[pid]);

    poseID ++;
    updateInterface();
}
