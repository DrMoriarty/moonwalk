#include "anim.h"

bool Bone::DRAW_SKELETON = true;
bool Bone::DRAW_GRAPHICS = true;

///////////////
// Bone
///////////////

QString Bone::BasePath = QDir::homePath();

Bone::Bone(int ID, Bone *parent)
    :parent(parent)
    ,ID(ID)
    ,x(0)
    ,y(0)
    ,angle(0)
    ,pivotX(0)
    ,pivotY(0)
	,z(0)
    ,name("bone")
    ,selected(false)
{
    if(parent) parent->addChild(this);
}

Bone::Bone(QXmlStreamReader *reader, Bone* parent)
    :parent(parent)
    ,ID(0)
    ,x(0)
    ,y(0)
    ,angle(0)
    ,pivotX(0)
    ,pivotY(0)
	,z(0)
    ,selected(false)
{
    ID = reader->attributes().value("ID").toString().toInt();
    x = reader->attributes().value("x").toString().toFloat();
    y = reader->attributes().value("y").toString().toFloat();
    z = reader->attributes().value("z").toString().toInt();
    pivotX = reader->attributes().value("pivotX").toString().toFloat();
    pivotY = reader->attributes().value("pivotY").toString().toFloat();
    angle = reader->attributes().value("angle").toString().toFloat();
    name = reader->attributes().value("name").toString();
    file = QDir(BasePath).absoluteFilePath(reader->attributes().value("file").toString());
    pixmap.load(file);
    if(parent) parent->addChild(this);
}

Bone::~Bone()
{
//    while(children.size()) {
//        delete children.back();
//        children.pop_back();
//    }
    if(parent) parent->remChild(this);
}

void Bone::addChild(Bone *ch)
{
    children.push_back(ch);
}

void Bone::remChild(Bone *ch)
{
    for(int i=0; i<children.size(); i++) if(children[i] == ch) {
        children.remove(i);
        return;
    }
}

QVector<int> Bone::childrenIDs()
{
    QVector<int> res;
    foreach(Bone *b, children) {
        res.push_back(b->ID);
        res << b->childrenIDs();
    }
    return res;
}

void Bone::DrawSelf(QPainter *painter, Pose* pose, float frame)
{
    if(!pixmap.isNull() && DRAW_GRAPHICS) {
        painter->drawPixmap(pivotX, pivotY, pixmap.width(), pixmap.height(), pixmap);
        if(selected) {
            painter->setCompositionMode(QPainter::CompositionMode_SoftLight);
            painter->drawPixmap(pivotX, pivotY, pixmap.width(), pixmap.height(), pixmap);
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }
    if(pixmap.isNull() || DRAW_SKELETON) {
        if(selected) {
            QPen p = painter->pen();
            p.setWidth(2.f);
            painter->setPen(p);
        }
        painter->drawEllipse(-10, -10, 20, 20);
        painter->drawLine(0, 0, 15, 0);
        painter->drawLine(0, 0, 0, 15);
        if(selected) {
            QPen p = painter->pen();
            p.setWidth(1.f);
            painter->setPen(p);
        }
        foreach(Bone *b, children) {
            Key k;
            if(pose) k = pose->CalcKey(frame, b->ID);
            painter->drawLine(0, 0, b->x + k.x, b->y + k.y);
        }
    }
}

void Bone::Draw(QPainter *painter, Pose* pose, float frame)
{
    painter->save();
    if(pose) {
        Key k = pose->CalcKey(frame, ID);
        painter->translate(x+k.x, y+k.y);
        painter->rotate(angle + k.angle);
    } else {
        painter->translate(x, y);
        painter->rotate(angle);
    }
    DrawSelf(painter, pose, frame);
    //painter->drawRect(anchorX-5, anchorY-5, 10, 10);
    foreach(Bone *b, children) {
        b->Draw(painter, pose, frame);
    }
    painter->restore();
}

void Bone::DrawGraphics(QPainter *painter, Pose* pose, float frame)
{
	float ang = 0.f;
	QPointF origin = Map(QPointF(0,0), &ang, pose, frame);
    painter->save();
    /*if(pose) {
        Key k = pose->CalcKey(frame, ID);
        painter->translate(origin.x()+k.x, origin.y()+k.y);
        painter->rotate(angle + k.angle + ang);
		} else {*/
        painter->translate(origin.x(), origin.y());
        painter->rotate(ang);
		//}
	painter->drawPixmap(pivotX, pivotY, pixmap.width(), pixmap.height(), pixmap);
	if(selected) {
		painter->setCompositionMode(QPainter::CompositionMode_SoftLight);
		painter->drawPixmap(pivotX, pivotY, pixmap.width(), pixmap.height(), pixmap);
		painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
	}
    painter->restore();
}

void Bone::DrawSkeleton(QPainter *painter, Pose* pose, float frame)
{
	float ang = 0.f;
	QPointF origin = Map(QPointF(0,0), &ang, pose, frame);
    painter->save();
    /*if(pose) {
        Key k = pose->CalcKey(frame, ID);
        painter->translate(origin.x()+k.x, origin.y()+k.y);
        painter->rotate(k.angle + ang);
		} else {*/
        painter->translate(origin.x(), origin.y());
        painter->rotate(ang);
		//}
	if(selected) {
		QPen p = painter->pen();
		p.setWidth(2.f);
		painter->setPen(p);
	}
	painter->drawEllipse(-10, -10, 20, 20);
	painter->drawLine(0, 0, 15, 0);
	painter->drawLine(0, 0, 0, 15);
	if(selected) {
		QPen p = painter->pen();
		p.setWidth(1.f);
		painter->setPen(p);
	}
	foreach(Bone *b, children) {
		Key k;
		if(pose) k = pose->CalcKey(frame, b->ID);
		painter->drawLine(0, 0, b->x + k.x, b->y + k.y);
	}
    painter->restore();
}

void Bone::Save(QXmlStreamWriter* writer)
{
    writer->writeStartElement("bone");
    writer->writeAttribute("ID", QString::number(ID));
    writer->writeAttribute("x", QString::number(x));
    writer->writeAttribute("y", QString::number(y));
    writer->writeAttribute("z", QString::number(z));
    writer->writeAttribute("angle", QString::number(angle));
    writer->writeAttribute("pivotX", QString::number(pivotX));
    writer->writeAttribute("pivotY", QString::number(pivotY));
    writer->writeAttribute("name", name);
    writer->writeAttribute("file", QDir(BasePath).relativeFilePath(file));
    foreach(Bone *ch, children) {
        ch->Save(writer);
    }
    writer->writeEndElement(); // bone
}

Bone* Bone::Parent()
{
    return parent;
}

QString Bone::Name()
{
    return QString("%1:%2").arg(ID).arg(name);
}

void Bone::MoveUp(Bone *ch)
{
    int index = children.indexOf(ch);
    if(index > 0) {
        qSwap(children[index-1], children[index]);
    }
}

void Bone::MoveDown(Bone *ch)
{
    int index = children.indexOf(ch);
    if(index >= 0 && index < children.size()-1) {
        qSwap(children[index], children[index+1]);
    }
}

void Bone::ReloadGraphics()
{
    if(!pixmap.load(file)) {
        pixmap = QPixmap();
    }
}

void Bone::SetGraphics(QString filePath)
{
    file = filePath;
    name = QFileInfo(filePath).fileName();
    ReloadGraphics();
}

QPointF Bone::Map(QPointF p, float* ang, Pose* pose, float frame)
{
    QPointF p2(x, y);
    float a = angle;
    if(pose && frame >= 0) {
        Key k = pose->CalcKey(frame, ID);
        p2.rx() += k.x;
        p2.ry() += k.y;
        a += k.angle;
    }
    QTransform tr;
    tr = tr.rotate(a);
    p2 += tr.map(p);
    if(ang) *ang += a;
    if(parent) return parent->Map(p2, ang, pose, frame);
    else return p2;
}

/////////////////
// Key
/////////////////

Key::Key(QXmlStreamReader *reader)
{
    active = true;
    x = reader->attributes().value("x").toString().toFloat();
    y = reader->attributes().value("y").toString().toFloat();
    angle = reader->attributes().value("angle").toString().toFloat();
    frame = reader->attributes().value("frame").toString().toInt();
}

void Key::Save(QXmlStreamWriter *writer)
{
    if(!active) return;
    writer->writeStartElement("key");
    writer->writeAttribute("frame", QString::number(frame));
    writer->writeAttribute("x", QString::number(x));
    writer->writeAttribute("y", QString::number(y));
    writer->writeAttribute("angle", QString::number(angle));
    writer->writeEndElement(); // key
}

bool operator<(const Key& k1, const Key& k2)
{
    return k1.frame < k2.frame;
}

/////////////////
// Pose
/////////////////

Pose::Pose(QString name, int length)
  :length(length)
  ,name(name)
{
}

Pose::Pose(QXmlStreamReader *reader)
{
    length = reader->attributes().value("length").toString().toInt();
    name = reader->attributes().value("name").toString();
}

void Pose::DelKey(int time, int boneID)
{
    QVector<Key> &vk = keys[boneID];
    for(int i=0; i<vk.size(); i++) {
        if(vk[i].frame == time) {
            vk.erase(vk.begin()+i);
            return;
        }
    }
}

int Pose::IsKey(int time, int boneID)
{
    QVector<Key> &vk = keys[boneID];
    for(int i=0; i<vk.size(); i++) {
        if(vk[i].frame == time) {
            return vk[i].active ? 1 : -1;
        }
    }
    return false;
}

int Pose::PrevKey(int time, int boneID)
{
    int prevkey = 0;
    QVector<Key> &vk = keys[boneID];
    for(int i=0; i<vk.size(); i++) {
        if(vk[i].frame < time && vk[i].frame > prevkey) {
            prevkey = vk[i].frame;
        }
    }
    return prevkey;
}

int Pose::NextKey(int time, int boneID)
{
    int nextkey = length;
    QVector<Key> &vk = keys[boneID];
    for(int i=0; i<vk.size(); i++) {
        if(vk[i].frame > time && vk[i].frame < nextkey) {
            nextkey = vk[i].frame;
        }
    }
    return nextkey;
}

void Pose::SetKey(int boneID, Key k)
{
    QVector<Key> &vk = keys[boneID];
    for(int i=0; i<vk.size(); i++) {
        if(vk[i].frame == k.frame) {
            vk[i] = k;
            return;
        }
    }
    vk.push_back(k);
    qSort(vk.begin(), vk.end());
}

void Pose::ChangeKey(int time, int boneID, float dx, float dy, float dangle)
{
    Key k = CalcKey(time, boneID);
    k.x += dx;
    k.y += dy;
    k.angle += dangle;
    k.frame = time;
    SetKey(boneID, k);
}

Key Pose::CalcKey(float time, int boneID)
{
    QVector<Key> &vk = keys[boneID];
    Key prev, next;
    foreach(Key k, vk) {
        if(k.active && k.frame <= time && (prev.frame < 0 || prev.frame < k.frame)) prev = k;
        if(k.active && k.frame >= time && (next.frame < 0 || next.frame > k.frame)) next = k;
    }
    if(prev.frame < 0 && next.frame < 0) return Key(); // not found
    if(prev.frame == time || (prev.frame >= 0 && next.frame < 0)) return prev; // only previous key frame found
    if(prev.frame < 0) prev.frame = 0;
    float T = (float)(time - prev.frame) / (float)(next.frame - prev.frame);
    Key res;
    res.x = (next.x - prev.x)*T + prev.x;
    res.y = (next.y - prev.y)*T + prev.y;
    res.angle = (next.angle - prev.angle)*T + prev.angle;
    return res;
}

void Pose::Save(QXmlStreamWriter *writer)
{
    writer->writeStartElement("pose");
    writer->writeAttribute("length", QString::number(length));
    writer->writeAttribute("name", name);
    foreach(int boneID, keys.keys()) {
        writer->writeStartElement("keys");
        writer->writeAttribute("bone", QString::number(boneID));
        foreach(Key k, keys[boneID]) {
            k.Save(writer);
        }
        writer->writeEndElement(); // keys
    }
    writer->writeEndElement(); // pose
}

int Pose::ShiftKey(int time, int boneID, int offset)
{
    if(offset == 0) return time;
    QVector<Key> &vk = keys[boneID];
    int newTime = time + offset;
    if(newTime > length) return -1;
    if(newTime < 0) return -1;
    while(IsKey(newTime, boneID)) {
        if(offset > 0) newTime ++;
        else newTime --;
        if(newTime > length) return -1;
        if(newTime < 0) return -1;
    }
    for(int i=0; i<vk.size(); i++) {
        if(vk[i].frame == time) {
            vk[i].frame = newTime;
            qSort(vk.begin(), vk.end());
            return newTime;
        }
    }
    return -1;
}


void Pose::Enable(int time, int boneID, bool en)
{
    QVector<Key> &vk = keys[boneID];
    for(int i=0; i<vk.size(); i++) {
        if(vk[i].frame == time) {
            vk[i].active = en;
            return;
        }
    }
}

void Pose::FillFrom(const Pose &p)
{
    keys = p.keys;
    length = p.length;
}

bool BoneLessThan(const Bone *b1, const Bone *b2)
{
	return b1->z < b2->z;
}
