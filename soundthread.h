#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <QThread>

#include "soundbank.h"

class GLWidget;

class SoundThread : public QThread
{
    Q_OBJECT
public:
    SoundThread(GLWidget *glW, QStringList samples);

    int loadSample(QString fileName);

private:
    void run();
    SoundBank *soundBank;
    GLWidget *glW;
    QMutex mutex;
    QStringList samples;

};

#endif // SOUNDTHREAD_H
