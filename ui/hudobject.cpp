#include "hudobject.h"


HudObject::HudObject()
{

}

void HudObject::draw(QPainter* painter) {
    Q_UNUSED(painter);
}

bool HudObject::click(){
    return false;
}

void HudObject::clickRelease(){
}

void HudObject::hide(){
}

void HudObject::show(){
}

void HudObject::move(int x, int y){
    Q_UNUSED(x)
    Q_UNUSED(y)
}

bool HudObject::hovers(){
    return false;
}

void HudObject::resize(){
}

