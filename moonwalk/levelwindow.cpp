#include "levelwindow.h"
#include "ui_levelwindow.h"
#include <QMessageBox>
#include <QFileDialog>

#define COMPANY "HeartGames"
#define APPNAME "LevelEditor"
#define VERSION 1

#define W_WIDTH 480
#define W_HEIGHT 320

bool ElementLessThan(const Element* el1, const Element* el2)
{
    return el1->z < el2->z;
}

/********* Element ***********/

QString Element::BasePath = QDir::homePath();

Element::Element(): type(ET_NONE), offsetX(0), offsetY(0), grit(0), x(0), y(0), z(1), scale(1), phys(PY_NONE) {}

Element::~Element()
{
    if(grit) delete grit;
}

void Element::Position(int posX, int posY) {
    offsetX = posX;
    offsetY = posY;
}

void Element::Update()
{
    if(grit) {
        grit->setPos( x - z * offsetX, y - z * offsetY);
        grit->setZValue(z);
        grit->setScale(scale);
    }
}

ElementType Element::getType() {
    return type;
}

/********* ElementImage ***********/

ElementImage::ElementImage(QString filename, QGraphicsScene *scene): filename(filename), curP(0)
{
    type = ET_IMAGE;
    QPixmap pix(filename);
    w = pix.width();
    h = pix.height();
    gr = scene->addPixmap(pix);
    grit = gr;
    poly = scene->addPolygon(QPolygonF(QRectF(0,0,25,25)), QPen(), Qt::yellow);
    poly->setOpacity(0.5f);
    cursor = scene->addRect(-2, -2, 10, 10);
    cursor->setVisible(false);
    Update();
}

ElementImage::ElementImage(QXmlStreamReader *reader, QGraphicsScene *scene, double par) : curP(0)
{
    x = reader->attributes().value("x").toString().toInt();
    y = reader->attributes().value("y").toString().toInt();
    z = par;
    scale = reader->attributes().value("scale").toString().toDouble();
    filename = QDir(BasePath).absoluteFilePath(reader->attributes().value("texture").toString());
    QString ph = reader->attributes().value("physics").toString();
    if(ph == "static") phys = PY_STATIC;
    else if(ph == "dynamic") phys = PY_DYNAMIC;
    type = ET_IMAGE;
    QPixmap pix(filename);
    w = pix.width();
    h = pix.height();
    gr = scene->addPixmap(pix);
    grit = gr;

    QPolygonF polygon;
    while (!reader->atEnd()) {
        reader->readNext();
        if(reader->isStartElement()) {
            if(reader->name() == "point") {
                QPointF p;
                p.setX(reader->attributes().value("x").toString().toDouble());
                p.setY(reader->attributes().value("y").toString().toDouble());
                polygon << p;
            }
        }
        if(reader->isEndElement()) {
            if(reader->name() == "point") {
            } else if(reader->name() == "image") {
                break;
            }
        }
    }
    poly = scene->addPolygon(polygon, QPen(), Qt::gray);
    poly->setOpacity(0.5f);
    cursor = scene->addRect(-2, -2, 10, 10);
    cursor->setVisible(false);
    Update();
}

ElementImage::~ElementImage()
{
    delete cursor;
    delete poly;
}

void ElementImage::Update()
{
    Element::Update();
    bool ph = phys != PY_NONE;
    poly->setVisible(ph);
    cursor->setVisible(ph);
    QPointF p = poly->polygon()[curP];
    cursor->setX(gr->x() + p.x() - 2);
    cursor->setY(gr->y() + p.y() - 2);
    cursor->setZValue(z + 0.1);
    poly->setX(gr->x());
    poly->setY(gr->y());
    poly->setZValue(z + 0.05);
}

void ElementImage::Save(QXmlStreamWriter* writer)
{
    writer->writeStartElement("image");
    writer->writeAttribute("x", QString::number(x));
    writer->writeAttribute("y", QString::number(y));
    writer->writeAttribute("scale", QString::number(scale));
    writer->writeAttribute("texture", QDir(BasePath).relativeFilePath(filename));
    switch(phys) {
    case PY_STATIC:
        writer->writeAttribute("physics", "static");
        break;
    case PY_DYNAMIC:
        writer->writeAttribute("physics", "dynamic");
        break;
    }
    if(phys != PY_NONE) {
        foreach(QPointF p, poly->polygon()) {
            writer->writeStartElement("point");
            writer->writeAttribute("x", QString::number(p.x()));
            writer->writeAttribute("y", QString::number(p.y()));
            writer->writeEndElement(); // point
        }
    }
    writer->writeEndElement(); // image
}

QRect ElementImage::getRect()
{
    if(z <= 0.f) return QRect(x, y, x, y);
    return QRect(x / z, y / z, (w*scale - W_WIDTH) / z, h*scale / z);
}

QString ElementImage::getName()
{
    return QFileInfo(filename).fileName();
}

void ElementImage::select(int s)
{
    if(s > 0) {
        curP = s - 1;
        cursor->setVisible(true);
        if(!poly->polygon().size()) {
            poly->setPolygon(QPolygonF(QRectF(0,0,25,25)));
        }
        Update();
    } else {
        cursor->setVisible(false);
    }
}

int ElementImage::size()
{
    return poly->polygon().size();
}

void ElementImage::movePoint(int p, float dx, float dy)
{
    QPolygonF polygon = poly->polygon();
    polygon[p].setX( polygon[p].x() + dx );
    polygon[p].setY( polygon[p].y() + dy );
    poly->setPolygon(polygon);
    cursor->setX(cursor->x() + dx);
    cursor->setY(cursor->y() + dy);
}

int ElementImage::addPoint(int id)
{
    QPointF p1, p2, p3;
    QPolygonF polygon = poly->polygon();
    p1 = polygon[id];
    if(id >= polygon.size()) p3 = polygon[0];
    else p3 = polygon[id+1];
    p2 = (p1 + p3) * .5f;
    polygon.insert(id+1, p2);
    curP = id+1;
    poly->setPolygon(polygon);
    Update();
    return curP;
}

int ElementImage::delPoint(int id)
{
    QPolygonF polygon = poly->polygon();
    if(polygon.size() > 1) {
        polygon.erase(polygon.begin() + id);
        if(id > 0) curP = id -1;
        else curP = 0;
        poly->setPolygon(polygon);
        Update();
        return curP;
    }
    return id;
}

/********* ElementPolygon ***********/

ElementPolygon::ElementPolygon(QString texfile, QGraphicsScene *scene): texfile(texfile), curP(0)
{
    type = ET_POLYGON;
    gr = scene->addPolygon(QPolygonF(QRectF(0, 0, 25, 25)), QPen(), QBrush(QPixmap(texfile)));
    grit = gr;
    cursor = scene->addRect(-2, -2, 10, 10);
    cursor->setVisible(false);
    Update();
}

ElementPolygon::ElementPolygon(QXmlStreamReader *reader, QGraphicsScene *scene, double par)
{
    x = reader->attributes().value("x").toString().toInt();
    y = reader->attributes().value("y").toString().toInt();
    z = par;
    scale = reader->attributes().value("scale").toString().toDouble();
    texfile = QDir(BasePath).absoluteFilePath(reader->attributes().value("texture").toString());
    QString ph = reader->attributes().value("physics").toString();
    if(ph == "static") phys = PY_STATIC;
    else if(ph == "dynamic") phys = PY_DYNAMIC;

    QPolygonF poly;
    while (!reader->atEnd()) {
        reader->readNext();
        if(reader->isStartElement()) {
            if(reader->name() == "point") {
                QPointF p;
                p.setX(reader->attributes().value("x").toString().toDouble());
                p.setY(reader->attributes().value("y").toString().toDouble());
                poly << p;
            }
        }
        if(reader->isEndElement()) {
            if(reader->name() == "point") {
            } else if(reader->name() == "polygon") {
                break;
            }
        }
    }

    type = ET_POLYGON;
    gr = scene->addPolygon(poly, QPen(), QBrush(QPixmap(texfile)));
    grit = gr;
    cursor = scene->addRect(-2, -2, 10, 10);
    cursor->setVisible(false);
    Update();
}

ElementPolygon::~ElementPolygon()
{
    delete cursor;
}

void ElementPolygon::Update()
{
    Element::Update();
    QPointF p = gr->polygon()[curP];
    cursor->setX(gr->x() + p.x() - 2);
    cursor->setY(gr->y() + p.y() - 2);
    cursor->setZValue(z + 0.1);
}

void ElementPolygon::Save(QXmlStreamWriter* writer)
{
    writer->writeStartElement("polygon");
    writer->writeAttribute("x", QString::number(x));
    writer->writeAttribute("y", QString::number(y));
    writer->writeAttribute("scale", QString::number(scale));
    writer->writeAttribute("texture", QDir(BasePath).relativeFilePath(texfile));
    switch(phys) {
    case PY_STATIC:
        writer->writeAttribute("physics", "static");
        break;
    case PY_DYNAMIC:
        writer->writeAttribute("physics", "dynamic");
        break;
    }
    foreach(QPointF p, gr->polygon()) {
        writer->writeStartElement("point");
        writer->writeAttribute("x", QString::number(p.x()));
        writer->writeAttribute("y", QString::number(p.y()));
        writer->writeEndElement(); // point
    }
    writer->writeEndElement(); // polygon
}

QRect ElementPolygon::getRect()
{
    if(z <= 0.f) return QRect(x, y, x, y);
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    foreach(QPointF p, gr->polygon()) {
        if(p.x() < x1) x1 = p.x();
        if(p.x() > x2) x2 = p.x();
        if(p.y() < y1) y1 = p.y();
        if(p.y() > y2) y2 = p.y();
    }
    x1 *= scale;
    x2 *= scale;
    y1 *= scale;
    y2 *= scale;
    return QRect((x + x1) / z, (y + y1) / z, (x + x2 - W_WIDTH) / z, (y + y2) / z);
}

QString ElementPolygon::getName()
{
    return QString("Polygon [%1]").arg(QFileInfo(texfile).fileName());
}

int ElementPolygon::size()
{
    return gr->polygon().size();
}

void ElementPolygon::select(int s)
{
    if(s > 0) {
        curP = s - 1;
        cursor->setVisible(true);
        Update();
    } else {
        cursor->setVisible(false);
    }
}

void ElementPolygon::movePoint(int p, float dx, float dy)
{
    QPolygonF poly = gr->polygon();
    poly[p].setX( poly[p].x() + dx );
    poly[p].setY( poly[p].y() + dy );
    gr->setPolygon(poly);
    cursor->setX(cursor->x() + dx);
    cursor->setY(cursor->y() + dy);
}

int ElementPolygon::addPoint(int id)
{
    QPointF p1, p2, p3;
    QPolygonF poly = gr->polygon();
    p1 = poly[id];
    if(id >= poly.size()) p3 = poly[0];
    else p3 = poly[id+1];
    p2 = (p1 + p3) * .5f;
    poly.insert(id+1, p2);
    curP = id+1;
    gr->setPolygon(poly);
    Update();
    return curP;
}

int ElementPolygon::delPoint(int id)
{
    QPolygonF poly = gr->polygon();
    if(poly.size() > 1) {
        poly.erase(poly.begin() + id);
        if(id > 0) curP = id -1;
        else curP = 0;
        gr->setPolygon(poly);
        Update();
        return curP;
    }
    return id;
}

/********** LevelWindow *************/

LevelWindow::LevelWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LevelWindow),
    curType(ET_NONE),
    nextElID(1),
    SelectElement(false),
    windowX(0),
    windowY(0),
    curPolygonPoint(0),
    curElement(0),
    polygonPointMode(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(ui->actionAnimation, SIGNAL(activated()), this, SIGNAL(raiseAnimation()));
    connect(ui->actionLevel, SIGNAL(activated()), this, SIGNAL(raiseLevel()));

    ui->graphicsView->setScene(&scene);
    background = scene.addRect(0, 0, W_WIDTH, W_HEIGHT, QPen(), Qt::lightGray);
    updateWindowPos();
    connect(ui->graphicsView, SIGNAL(dragLeftMouse(float,float)), this, SLOT(leftDrag(float, float)));
    connect(ui->graphicsView, SIGNAL(dragRightMouse(float,float)), this, SLOT(rightDrag(float, float)));
    connect(ui->graphicsView, SIGNAL(dragMiddleMouse(float,float)), this, SLOT(middleDrag(float, float)));
    connect(ui->graphicsView, SIGNAL(pressLeftMouse(float,float)), this, SLOT(leftPress(float, float)));
    connect(ui->graphicsView, SIGNAL(pressRightMouse(float,float)), this, SLOT(rightPress(float, float)));
    connect(ui->graphicsView, SIGNAL(pressMiddleMouse(float,float)), this, SLOT(middlePress(float, float)));

    QSettings set(COMPANY, APPNAME);
    this->restoreGeometry(set.value("window/geometry").toByteArray());
    this->restoreState(set.value("window/state").toByteArray(), VERSION);

}

LevelWindow::~LevelWindow()
{
    delete ui;
}

void LevelWindow::closeEvent(QCloseEvent *event)
{
    QSettings set(COMPANY, APPNAME);
    set.setValue("window/geometry", saveGeometry());
    set.setValue("window/state", saveState(VERSION));
    if(elements.size() > 0) {
        QMessageBox *msg = new QMessageBox(QMessageBox::Question, tr("Close program"), tr("Save level before quit?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        int res = msg->exec();
        if(res == QMessageBox::Cancel) event->ignore();
        else {
            if(res == QMessageBox::Save) saveLevel();
            event->accept();
        }
    } else {
        event->accept();
    }
}

void LevelWindow::setElementType(int et)
{
    curType = (ElementType)et;
    //statusBar()->showMessage(QString("eltype = %1").arg(et));
}

void LevelWindow::addElement()
{
    if(curType == ET_NONE) return;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), Element::BasePath, tr("Images (*.png *.xpm *.jpg)"));
    if(filename.isNull()) return;
    switch(curType) {
    case ET_NONE:
    default:
        return;
    case ET_IMAGE:
        elements[nextElID] = new ElementImage(filename, &scene);
        break;
    case ET_POLYGON:
        elements[nextElID] = new ElementPolygon(filename, &scene);
        break;
    }
    QString name = QString("%1 - %2").arg(nextElID).arg(elements[nextElID]->getName());
    QListWidgetItem* it = new QListWidgetItem(name, ui->listWidget_elements);
    it->setData(Qt::UserRole, nextElID);
    it->setSelected(true);
    nextElID ++;
    ui->graphicsView->updateGeometry();
    updateWindowSize();
}

void LevelWindow::delElement()
{
    if(!ui->listWidget_elements->selectedItems().size()) return;
    QListWidgetItem * it = ui->listWidget_elements->selectedItems()[0];
    int eid = it->data(Qt::UserRole).toInt();
    delete it;
    delete elements[eid];
    elements.erase(elements.find(eid));
}

void LevelWindow::selectElement()
{
    int eid = CurrentElementID();
    //this->statusBar()->showMessage(QString("eid = %1").arg(eid));
    if(eid > 0 && elements.find(eid) != elements.end()) {
        if(curElement) curElement->select(0);
        curElement = elements[eid];
        curElement->select(1);
        SelectElement = true;
        if(curElement->getType() == ET_POLYGON || curElement->phys != PY_NONE) {
            //ElementPolygon *poly = (ElementPolygon*)curElement;
            ui->frame_polygon->setEnabled(true);
            ui->spinBox_polygonPoint->setValue(0);
            ui->spinBox_polygonPoint->setMaximum(curElement->size()-1);
            ui->pushButton_polygonEdit->setChecked(false);
            polygonPointMode = false;
        } else {
            ui->frame_polygon->setEnabled(false);
        }
        ui->spinBox_x->setValue(elements[eid]->x);
        ui->spinBox_y->setValue(elements[eid]->y);
        ui->doubleSpinBox_z->setValue(elements[eid]->z);
        ui->doubleSpinBox_scale->setValue(elements[eid]->scale);
        ui->comboBox_physElement->setCurrentIndex(elements[eid]->phys);
        ui->spinBox_x->setEnabled(true);
        ui->spinBox_y->setEnabled(true);
        ui->doubleSpinBox_z->setEnabled(true);
        ui->doubleSpinBox_scale->setEnabled(true);
        ui->comboBox_physElement->setEnabled(true);
        SelectElement = false;
    } else {
        ui->spinBox_x->setEnabled(false);
        ui->spinBox_y->setEnabled(false);
        ui->doubleSpinBox_z->setEnabled(false);
        ui->doubleSpinBox_scale->setEnabled(false);
        ui->comboBox_physElement->setEnabled(false);
    }
}

void LevelWindow::setX(int x)
{
    if(SelectElement) return;
    int eid = CurrentElementID();
    if(eid > 0) {
        elements[eid]->x = x;
        elements[eid]->Update();
    }
}

void LevelWindow::setY(int y)
{
    if(SelectElement) return;
    int eid = CurrentElementID();
    if(eid > 0) {
        elements[eid]->y = y;
        elements[eid]->Update();
    }
}

void LevelWindow::setZ(double z)
{
    if(SelectElement) return;
    int eid = CurrentElementID();
    if(eid > 0) {
        elements[eid]->z = z;
        elements[eid]->Update();
    }
}

void LevelWindow::setScale(double scale)
{
    if(SelectElement) return;
    int eid = CurrentElementID();
    if(eid > 0) {
        elements[eid]->scale = scale;
        elements[eid]->Update();
    }
}

void LevelWindow::setPhysics(int ph)
{
    if(SelectElement) return;
    int eid = CurrentElementID();
    if(eid > 0) {
        elements[eid]->phys = (ElementPhysic)ph;
        elements[eid]->Update();
        selectElement();
    }
}

void LevelWindow::newLevel()
{
    clear();
}

void LevelWindow::openLevel()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load level"), Element::BasePath, tr("Xml file (*.xml)"));
    if(filename.isEmpty()) return;
    levelFile = filename;
    levelName = filename;
    clear();
    QFile input(filename);
    QFileInfo fi(input);
    Element::BasePath = fi.absolutePath();
    QDir().cd(Element::BasePath);
    input.open(QIODevice::ReadOnly);

    QXmlStreamReader stream(&input);
    double parallax = 0;
    while (!stream.atEnd()) {
        stream.readNext();
        if(stream.isStartElement()) {
            if(stream.name() == "level") {
                levelName = stream.attributes().value("name").toString();
            } else if(stream.name() == "element") {
                parallax = stream.attributes().value("parallax").toString().toDouble();
            } else if(stream.name() == "image") {
                elements[nextElID] = new ElementImage(&stream, &scene, parallax);
                QString name = QString("%1 - %2").arg(nextElID).arg(elements[nextElID]->getName());
                QListWidgetItem* it = new QListWidgetItem(name, ui->listWidget_elements);
                it->setData(Qt::UserRole, nextElID);
                nextElID ++;
            } else if(stream.name() == "polygon") {
                elements[nextElID] = new ElementPolygon(&stream, &scene, parallax);
                QString name = QString("%1 - %2").arg(nextElID).arg(elements[nextElID]->getName());
                QListWidgetItem* it = new QListWidgetItem(name, ui->listWidget_elements);
                it->setData(Qt::UserRole, nextElID);
                nextElID ++;
            } else if(stream.name() == "particle") {
                // TODO
            }
        }
        if(stream.isEndElement()) {
            if(stream.name() == "level") {
            } else if(stream.name() == "element") {
            } else if(stream.name() == "image") {
            } else if(stream.name() == "polygon") {
            } else if(stream.name() == "particle") {
            }
        }
    }
    if (stream.hasError()) {
        // TODO
    } else {
        setWindowTitle(tr("MoonTools/Level - %1").arg(levelName));
    }
    input.close();
    updateFrame();
}

void LevelWindow::saveLevel()
{
    if(levelFile.isEmpty()) levelFile = QFileDialog::getSaveFileName(this, tr("Save animation"), Element::BasePath, tr("Xml file (*.xml)"));
    saveLevelTo(levelFile);
}

void LevelWindow::saveLevelAs()
{
    levelFile = QFileDialog::getSaveFileName(this, tr("Save animation"), Element::BasePath, tr("Xml file (*.xml)"));
    saveLevelTo(levelFile);
}

void LevelWindow::saveLevelTo(QString file)
{
    if(file.isEmpty()) return;
    setWindowTitle(tr("MoonTools/Level - %1").arg(levelName));
    QFile output(file);
    Element::BasePath = QFileInfo(output).absolutePath();
    QDir().cd(Element::BasePath);
    output.open(QIODevice::WriteOnly);
    QXmlStreamWriter stream(&output);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("level");
    stream.writeAttribute("name", levelName);
    QVector<Element*> els;
    foreach(Element *el, elements) {
        els << el;
    }
    qSort(els.begin(), els.end(), ElementLessThan);
    float par = -1.f;
    foreach(Element *el, els) {
        if(el->z != par) {
            if(par >= 0.f) stream.writeEndElement(); // element
            par = el->z;
            stream.writeStartElement("element");
            stream.writeAttribute("parallax", QString::number(par));
        }
        el->Save(&stream);
    }
    if(par >= 0.f) stream.writeEndElement(); // element
    stream.writeEndElement(); // level
    stream.writeEndDocument();
    output.close();
}

void LevelWindow::scrollWindowX(int x)
{
    if(windowX != x) {
        windowX = x;
        updateWindowPos();
    }
}

void LevelWindow::scrollWindowY(int y)
{
    if(windowY != y) {
        windowY = y;
        updateWindowPos();
    }
}

void LevelWindow::updateFrame()
{
    updateWindowSize();
    updateWindowPos();
}

int LevelWindow::CurrentElementID()
{
    if(!ui->listWidget_elements->selectedItems().size()) return -1;
    return ui->listWidget_elements->selectedItems()[0]->data(Qt::UserRole).toInt();
}

void LevelWindow::updateWindowSize()
{
    int x1=0, x2=0, y1=0, y2=0;
    foreach(Element *el, elements) {
        QRect s = el->getRect();
        if(s.left() < x1) x1 = s.left();
        if(s.right() > x2) x2 = s.right();
        if(s.bottom() < y1) y1 = s.bottom();
        if(s.top() > y2) y2 = s.top();
        y1 -= 50;
        y2 += 50;
        ui->verticalScrollBar->setMinimum(y1);
        ui->verticalScrollBar->setMaximum(y2);
        ui->horizontalScrollBar->setMinimum(x1);
        ui->horizontalScrollBar->setMaximum(x2);
        ui->spinBox_frameY->setMinimum(y1);
        ui->spinBox_frameY->setMaximum(y2);
        ui->spinBox_frameX->setMinimum(x1);
        ui->spinBox_frameX->setMaximum(x2);
    }
}

void LevelWindow::updateWindowPos()
{
    foreach(Element *el, elements) {
        el->Position(windowX, windowY);
        el->Update();
    }
    ui->graphicsView->setSceneRect(0, 0, W_WIDTH, W_HEIGHT);
    background->setPos(0, 0);
}

void LevelWindow::clear()
{
    ui->listWidget_elements->clear();
    foreach(Element* el, elements) {
        delete el;
    }
    elements.clear();
    nextElID = 1;
}

void LevelWindow::leftDrag(float dx, float dy)
{
    if(SelectElement) return;
    int eid = CurrentElementID();
    if(eid > 0) {
        if(polygonPointMode) {
            ElementPolygon *poly = (ElementPolygon*)elements[eid];
            poly->movePoint(curPolygonPoint, dx, dy);
        } else {
            elements[eid]->y += dy;
            elements[eid]->x += dx;
            elements[eid]->Update();
            ui->spinBox_x->setValue(elements[eid]->x);
            ui->spinBox_y->setValue(elements[eid]->y);
        }
    }
}

void LevelWindow::rightDrag(float dx, float dy)
{
}

void LevelWindow::middleDrag(float dx, float dy)
{
}

void LevelWindow::leftPress(float x, float y)
{
}

void LevelWindow::rightPress(float x, float y)
{
}

void LevelWindow::middlePress(float x, float y)
{
}

void LevelWindow::setPolygonPoint(int p)
{
    if(SelectElement) return;
    int eid = CurrentElementID();
    if(eid > 0) {
        curPolygonPoint = p;
        elements[eid]->select(curPolygonPoint + 1);
    }
}

void LevelWindow::editPolygon(bool ep)
{
    polygonPointMode = ep;
}

void LevelWindow::addPolygonPoint()
{
    int eid = CurrentElementID();
    if(eid > 0 && elements[eid]->getType() == ET_POLYGON) {
        ElementPolygon *poly = (ElementPolygon*)elements[eid];
        curPolygonPoint = poly->addPoint(curPolygonPoint);
        ui->spinBox_polygonPoint->setValue(curPolygonPoint);
        ui->spinBox_polygonPoint->setMaximum(poly->size()-1);
    }
}

void LevelWindow::delPolygonPoint()
{
    int eid = CurrentElementID();
    if(eid > 0 && elements[eid]->getType() == ET_POLYGON) {
        ElementPolygon *poly = (ElementPolygon*)elements[eid];
        curPolygonPoint = poly->delPoint(curPolygonPoint);
        ui->spinBox_polygonPoint->setValue(curPolygonPoint);
        ui->spinBox_polygonPoint->setMaximum(poly->size()-1);
    }
}
