#ifndef ANIM_H
#define ANIM_H

#include <QtGui>

class Key {
public:
    float x, y, angle;
    int frame;
    bool active;
    Key(): frame(-1), x(0), y(0), angle(0), active(true) {};
    Key(int frame, float x, float y, float angle): frame(frame), x(x), y(y), angle(angle), active(true) {};
    Key(QXmlStreamReader* reader);
    void Save(QXmlStreamWriter* writer);
};

bool operator<(const Key& k1, const Key& k2);

class Pose {
protected:
    QMap<int, QVector<Key> > keys;
public:
    int length;
    QString name;
    Pose(QString name, int length = 25);
    Pose(QXmlStreamReader* reader);
    void SetKey(int boneID, Key k);
    void ChangeKey(int time, int boneID, float dx, float dy, float dangle);
    void DelKey(int time, int boneID);
    Key CalcKey(float time, int boneID);
    void Save(QXmlStreamWriter* writer);
    int IsKey(int time, int boneID);
    int PrevKey(int time, int boneID);
    int NextKey(int time, int boneID);
    int ShiftKey(int time, int boneID, int offset);
    void Enable(int time, int boneID, bool en);
    void FillFrom(const Pose& p);
};

class Bone {
protected:
    Bone* parent;
    QVector<Bone*> children;
    QPixmap pixmap;
protected:
    void addChild(Bone* ch);
    void remChild(Bone* ch);
    void DrawSelf(QPainter *painter, Pose* pose, float frame);
public:
    static bool DRAW_SKELETON;
    static bool DRAW_GRAPHICS;

    static QString BasePath;
    int ID;
    float x, y, angle, pivotX, pivotY;
    int z;
    QString name, file;
    bool selected;

    Bone(int ID, Bone* parent);
    Bone(QXmlStreamReader* reader, Bone* parent);
    virtual ~Bone();
    QVector<int> childrenIDs();
    void Draw(QPainter* painter, Pose* pose = 0, float frame = 0);

	void DrawGraphics(QPainter* painter, Pose* pose = 0, float frame = 0.f);
	void DrawSkeleton(QPainter* painter, Pose* pose = 0, float frame = 0.f);
	
    void Save(QXmlStreamWriter* writer);
    Bone* Parent();
    QString Name();
    void MoveUp(Bone* ch);
    void MoveDown(Bone* ch);
    void ReloadGraphics();
    void SetGraphics(QString filePath);
    QPointF Map(QPointF p = QPointF(0,0), float* angle = 0, Pose* pose = 0, float frame = 0);
};

bool BoneLessThan(const Bone *b1, const Bone *b2);

#endif // ANIM_H
