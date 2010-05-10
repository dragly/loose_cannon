#ifndef SOUNDBANK_H
#define SOUNDBANK_H

#include <QtMultimedia>

class SoundBank : public QObject
{
    Q_OBJECT
public:
    SoundBank();
    QAudioDeviceInfo  device;
    QAudioFormat      settings;
    QList<QAudioOutput*> audioOuputs;
    QHash<QString, QByteArray*> audioSamples;
    QHash<QAudioOutput*, QBuffer*> hashOutputToBuffer;
    void loadSample(const QString &fileName);
    QHash<QAudioOutput*,int> counter;
    int numberOfChannels;
    QList<QAudioOutput*> freeChannels;
    QList<QBuffer*> freeBuffers;
    QList<QAudioOutput*> closedChannels;
    QList<QBuffer*> closedBuffers;

public slots:
    void play(QString sample);

private slots:
    void state(QAudio::State s);
    void status();
};

#endif // SOUNDBANK_H
