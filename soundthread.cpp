#include "soundthread.h"

#include "glwidget.h"

SoundThread::SoundThread(GLWidget *glW)
{
    this->glW = glW;
}

void SoundThread::run() {
    qDebug() << "run thread is" << QThread::currentThreadId();
    soundBank = new SoundBank();
    soundBank->loadSample("sounds/bomb.wav");
    connect(glW, SIGNAL(play(int)), soundBank, SLOT(play(int)));
    exec();
}

int SoundThread::loadSample(QString fileName) {
    return soundBank->loadSample(fileName);
}

void SoundThread::play(int sample) {
    soundBank->play(sample);
}
