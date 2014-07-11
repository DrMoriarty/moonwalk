#ifndef DRAWAREA_H
#define DRAWAREA_H

#include <QWidget>

class DrawArea : public QWidget
{
    Q_OBJECT
private:
    QPointF mousePos;
    Qt::MouseButton curButton;

public:
    int mod;
    explicit DrawArea(QWidget *parent = 0);

signals:
    void customPaint(QPainter* painter, int width, int height);
    void dragLeftMouse(float, float);
    void dragRightMouse(float, float);
    void dragMiddleMouse(float, float);
    void pressLeftMouse(float, float);
    void pressRightMouse(float, float);
    void pressMiddleMouse(float, float);

public slots:

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void	mouseMoveEvent ( QMouseEvent * event );
    virtual void	mousePressEvent ( QMouseEvent * event );
    virtual void	mouseReleaseEvent ( QMouseEvent * event );
//    virtual void	keyPressEvent ( QKeyEvent * event );
//    virtual void	keyReleaseEvent ( QKeyEvent * event );
};



#endif // DRAWAREA_H
