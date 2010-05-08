#include "soundbank.h"

SoundBank::SoundBank()
{
    settings.setFrequency(48000);
    settings.setChannels(2);
    settings.setSampleSize(16);
    settings.setCodec("audio/pcm");
    settings.setByteOrder(QAudioFormat::LittleEndian);
    settings.setSampleType(QAudioFormat::SignedInt);
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    qDebug() << "deviceName" << info.deviceName();
    if (!info.isFormatSupported(settings)) {
        qWarning()<<"default format not supported try to use nearest";
        settings = info.nearestFormat(settings);
    }

    if(settings.sampleSize() != 16) {
        qWarning()<<"audio device doesn't support 16 bit samples, example cannot run";
        return;
    }
    device = info;

    // create channels
    for(int i = 0; i<8; i++) {
        QAudioOutput *audioOutput = new QAudioOutput(device,settings,this);
        freeChannels.append(audioOutput);
        audioOutput->setNotifyInterval(100);
        connect(audioOutput,SIGNAL(notify()),SLOT(status()));
        connect(audioOutput,SIGNAL(stateChanged(QAudio::State)),SLOT(state(QAudio::State)));
        freeBuffers.append(new QBuffer());
    }
}

int SoundBank::loadSample(const QString &fileName) {

    qDebug() << "loading" << fileName;

    QFile inputFile;
    inputFile.setFileName(fileName);
    inputFile.open(QIODevice::ReadOnly);

    QBuffer *buffer = new QBuffer();
    buffer->open(QBuffer::ReadWrite);
    QByteArray data = inputFile.readAll();
    buffer->write(data);
    qDebug() << "loadSample size" << buffer->size();
    inputFile.close();
    buffer->close();
    audioSources.append(buffer);
    return audioSources.count() - 1;
}

void SoundBank::play(int sample) {
    QBuffer *buffer;
    QAudioOutput *audioOutput;
    if(freeChannels.count() > 0 && freeBuffers.count() > 0) {
        buffer = freeBuffers.at(0);
        freeBuffers.removeAt(0);
        closedBuffers.append(buffer);
        audioOutput = freeChannels.at(0);
        freeChannels.removeAt(0);
        closedChannels.append(audioOutput);
    } else {
        qDebug() << "No free channels, opening!";
        if(closedBuffers.count() > 0 && closedChannels.count() > 0) {
            buffer = closedBuffers.at(0);
            buffer->close();
            buffer->setData(QByteArray()); // delete data from memory
            audioOutput = closedChannels.at(0);
        }
////        audioOutput->suspend();
    }
    // copy our buffer
    QBuffer *inBuffer = audioSources.at(sample);
    buffer->open(QBuffer::ReadWrite);
    inBuffer->open(QBuffer::ReadOnly);
    buffer->write(inBuffer->readAll());
    inBuffer->close();
    buffer->close();

    // end copy, open buffer for reading
    buffer->open(QBuffer::ReadOnly);
    hashOutputToBuffer.insert(audioOutput, buffer);
    counter.insert(audioOutput,0);
    audioOutput->start(buffer);
}


void SoundBank::status()
{
    QAudioOutput *audioOutput = qobject_cast<QAudioOutput*>(sender());
    if(audioOutput != NULL) {
//        qWarning() << "byteFree = " << audioOutput->bytesFree() << " bytes, elapsedUSecs = " << audioOutput->elapsedUSecs() << ", processedUSecs = " << audioOutput->processedUSecs();
    }
}
void SoundBank::state(QAudio::State state)
{
    QAudioOutput *audioOutput = qobject_cast<QAudioOutput*>(sender());
    if(audioOutput != NULL) {
        QBuffer *buffer = hashOutputToBuffer.value(audioOutput);
        qWarning() << " state=" << state << " == " << QAudio::IdleState;
        qWarning() << " error=" << audioOutput->error();
        if(state == QAudio::IdleState) {
            buffer->close();
            buffer->setData(QByteArray()); // delete data from memory
            freeBuffers.append(buffer);
            closedBuffers.removeOne(buffer);
//            delete hashOutputToBuffer.value(audioOutput);
            freeChannels.append(audioOutput);
            closedChannels.removeOne(audioOutput);
            // closing and stopping causes extreme lags - they should be deleted, but not here!
//            audioOutput->stop();
//            delete audioOutput;
        }
    }
}