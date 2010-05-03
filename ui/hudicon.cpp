#include "hudicon.h"
#include "ui.h"
#include <QRect>
#include <QColor>
#include <QFont>
#include "window.h"
#include "hudobject.h"
#include "glwidget.h"

HudIcon::HudIcon(HudObject* parent,  Window::Alignments alignment, qreal x, qreal y, QString text)
{
    this->ui = parent->ui;
    this->x = x;
    this->alignment = alignment;
    this->y = y;
    this->text = text;
    ui->addHudObject(this);
}

void HudIcon::draw(QPainter* painter) {
    qreal xoff = x;
    qreal yoff = y;
    int height = ui->glW->height();
    if (alignment == Window::TopRight) {
        xoff += ui->glW->width() - 0.05*height;
    }

    QRect rect(xoff,yoff,0.05*height,0.05*height);

    QColor color;
    if (ui->selectedHudObject == this && hovers() && ui->isMouseDown())
        color = Window::ColorHighlight;
    else
        color = Window::ColorBorder;

    painter->setPen(color);
    painter->setBrush(Window::ColorBackground);

    painter->drawRect(rect);

    QFont font;
    font.setPixelSize(0.02 * ui->glW->height());
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignCenter, text);

}

bool HudIcon::click() {
    return hovers();

}

void HudIcon::clickRelease() {
    iconClicked();
}

bool HudIcon::hovers() {
    int height = ui->glW->height();
    QRect rect(ui->glW->width()-0.05*height,0,0.05*height,0.05*height);

    return (!(ui->mouseX < rect.x() || ui->mouseX > rect.x() + rect.width() || ui->mouseY < rect.y() || ui->mouseY > rect.y()+ rect.height()));

}
