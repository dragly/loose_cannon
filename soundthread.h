#ifndef SOUNDTHREAD_H
#define SOUNDTHREAD_H

#include <QThread>

#include "soundbank.h"

class GLWidget;

class SoundThread : public QThread
{
    Q_OBJECT
public:
    SoundThread(GLWidget *glW);

    int loadSample(QString fileName);

public slots:
    void play(int sample);

private:
    void run();
    SoundBank *soundBank;
    GLWidget *glW;

};

#endif // SOUNDTHREAD_H
