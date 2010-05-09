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

    // to avoid recreating outputs and buffers for each sound played, we rather use an upper limit for the amount of sounds available
    for(int i = 0; i<16; i++) { // create channels and buffers
        QAudioOutput *audioOutput = new QAudioOutput(device,settings,this);
        freeChannels.append(audioOutput);
        audioOutput->setNotifyInterval(100);
        connect(audioOutput,SIGNAL(notify()),SLOT(status()));
        connect(audioOutput,SIGNAL(stateChanged(QAudio::State)),SLOT(state(QAudio::State)));
        freeBuffers.append(new QBuffer());
    }
    qDebug() << "SoundBank::SoundBank() loaded";
}

int SoundBank::loadSample(const QString &fileName) {

    qDebug() << "SoundBank::loadSample loading" << fileName;

    QFile inputFile;
    inputFile.setFileName(fileName);
    inputFile.open(QIODevice::ReadOnly);

    qDebug() << "SoundBank::loadSample size" << inputFile.size();
    QByteArray *data = new QByteArray(inputFile.readAll());
    inputFile.close();
    audioSources.append(data);
    return audioSources.count() - 1;
}

void SoundBank::run() {
    exec();
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
            audioOutput = closedChannels.at(0);
        }
////        audioOutput->suspend();
    }
    buffer->setBuffer(audioSources.at(sample)); // set new data pointer to buffer

    buffer->open(QBuffer::ReadOnly); // open the buffer for reading
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
//        qWarning() << " state=" << state << " == " << QAudio::IdleState;
//        qWarning() << " error=" << audioOutput->error();
        if(state == QAudio::IdleState) {
            buffer->close();
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
