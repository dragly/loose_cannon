#ifndef HUDICON_H
#define HUDICON_H
#include "hudobject.h"
#include "window.h"
class Ui;

class HudIcon : public HudObject
{
    Q_OBJECT
public:
    HudIcon(HudObject* parent,  Window::Alignments alignment, qreal x, qreal y, QString text);


    //overloaded
    void draw(QPainter* painter);
    bool click();
    void clickRelease();
    bool hovers();

signals:
    void iconClicked();
private:
    Window::Alignments alignment;
    qreal x,y;
    QString text;
};

#endif // HUDICON_H
