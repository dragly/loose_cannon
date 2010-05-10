#include "soundthread.h"

#include "glwidget.h"

SoundThread::SoundThread(GLWidget *glW, QStringList samples)
{
    this->glW = glW;
    this->samples = samples;
}

void SoundThread::run() {
    qDebug() << "run thread is" << QThread::currentThreadId();
    soundBank = new SoundBank();
    foreach(QString sample, samples) {
        soundBank->loadSample(sample);
    }
    connect(glW, SIGNAL(playSound(QString)), soundBank, SLOT(play(QString)));
    exec();
}
