/*
    This file is part of QOrbiter for use with the LinuxMCE project found at http://www.linuxmce.org
    Author: Langston Ball
   Langston Ball  golgoj4@gmail.com
    QOrbiter is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QOrbiter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QOrbiter.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H


#include <QQuickItem>
#include <QImage>
#include <QKeyEvent>
#include <QObject>

#include <qMediaPlayer/qMediaPlayer.h>

#include <QTime>
#include <QTcpServer>

class NavigationMenu;

using namespace DCE;
using namespace Qt;
/*!
 * \brief The MediaManager class provides Video and Audio playback for DCE related video streams.
 * This class represents a network media player embedded into the application. It runs as a plugin
 * as opposed to being directly instantiated by the Application to allow for modularity.
 */
class MediaManager : public QQuickItem
{


    Q_PROPERTY(bool connected READ getConnectionStatus WRITE setConnectionStatus NOTIFY connectedChanged)
    Q_PROPERTY(QString pluginUrl READ getPluginUrl NOTIFY pluginUrlUpdated)
    Q_PROPERTY(QString currentStatus READ getCurrentStatus WRITE setCurrentStatus NOTIFY currentStatusChanged)
    Q_PROPERTY(bool mediaPlaying READ getMediaPlaying WRITE setMediaPlaying NOTIFY mediaPlayingChanged)
    Q_PROPERTY(bool hasError READ getErrorStatus WRITE setErrorStatus NOTIFY hasErrorChanged)
    Q_PROPERTY(QString lastError READ getMediaError WRITE setMediaError NOTIFY lastErrorChanged)
    Q_PROPERTY(int mediaBuffer READ getMediaBuffer WRITE setMediaBuffer NOTIFY mediaBufferChanged)
    Q_PROPERTY(QString fileReference READ getFileReference NOTIFY fileReferenceChanged)
    Q_PROPERTY(qreal volume READ getVolume NOTIFY volumeChanged)
    Q_PROPERTY(qreal displayVolume READ getDisplayVolume() WRITE setDisplayVolume NOTIFY displayVolumeChanged)
    Q_PROPERTY(bool muted READ getMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QString fileUrl READ getMediaUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(int incomingTime READ getIncomingTime WRITE setIncomingTime NOTIFY incomingTimeChanged)
    Q_PROPERTY(bool videoStream READ getVideoStream WRITE setVideoStream NOTIFY videoStreamChanged)
    Q_PROPERTY(QString serverAddress READ getServerAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(int deviceNumber READ getDeviceNumber WRITE setDeviceNumber NOTIFY deviceNumberChanged)

    Q_OBJECT

public:
    explicit MediaManager(QQuickItem *parent = 0);

    //media info
    int currentTime;
    qint64 totalTime;
    int playBackSpeed;
    int streamId;
    QString fileReference;
    int fileno;
    QString filepath;
    QString fileUrl;
    int mediaBuffer;

    bool mediaPlaying;
    bool hasError;
    bool useInvertTrick;
    QString lastError;
    QString lastTick;

    //--------------------------
    qreal volume;
    qreal displayVolume;
    bool muted;

    QString currentStatus;
    QString qs_totalTime;


    QTcpServer *timeCodeServer;
    QString current_position;
    int iCurrent_Position;

    qMediaPlayer *mediaPlayer;

    QList<QTcpSocket*> clientList;
    QTcpSocket*lastClient;
    QTcpSocket*callbackClient;

    QString serverAddress;
    int deviceNumber;

    QString pluginUrl;

    bool connected;

    int videoHeight;
    int videoWidth;

    bool flipColors;
    quint64 tempTime;

    int incomingTime;
    long currentDevice;
    bool videoStream;

signals:
    void useInvertTrickChanged();
    void videoStreamChanged();
    void connectedChanged();
    void currentStatusChanged();
    void totalTimeChanged();

    void colorFlipChanged();

    void newData(QByteArray b);

    void mediaPlayingChanged();
    void hasErrorChanged();
    void lastErrorChanged();
    void mediaBufferChanged();
    void mediaAboutToFinish();
    void prefinishMark();

    void serverAddressChanged();
    void deviceNumberChanged();

    void volumeChanged();
    void mutedChanged();

    void availibleAudioOutputsChanged();

    void pluginUrlUpdated();
    void asyncStop();
    void asyncPositionRequest(int r);
    void callbackChanged();
    void fileReferenceChanged();
    void fileUrlChanged();
    void fileNumberChanged();
    void incomingTimeChanged();
    void incomingTick(quint64);
    void updatePluginSeek(int pos);

    void setPluginVolume(double d);
    void pauseMedia();
    void requestPosition();
    void requestDuration();
    void displayVolumeChanged();
    void pluginVolumeUp();
    void pluginVolumeDown();

public slots:
    void setVideoStream(bool b ){

        if(videoStream != b){
            videoStream = b;
            emit videoStreamChanged();
        }
        qDebug() << "Item has video::"<<videoStream;
    }
    bool getVideoStream(){return videoStream;}

    void setCurrentDevice(long d){currentDevice = d;mountDrive(currentDevice);}

    void setIncomingTime(int i){ incomingTime = i; emit incomingTimeChanged();}
    int getIncomingTime() { return incomingTime;}

    QString getPluginUrl(){ return pluginUrl;}

    void stopPluginMedia(){
        setMediaPlaying(false);
    }

    void qmlPlaybackEnded(bool ended){
        mediaPlayer->EVENT_Playback_Completed(mediaPlayer->getInternalMediaUrl().toStdString(),mediaPlayer->getStreamID(), ended);
    }

    void setMuted(bool m){muted = m; emit mutedChanged();}
    bool getMuted(){ return muted;}

    void triggerVolumeChange(){

    }

    void setPaused(){
        qWarning() << "mediaManager::Setting pause";
        emit pauseMedia();
    }

    void setVolume(qreal vol){
        volume = vol;
        displayVolume=vol;
        qWarning() << "Vol level :: " << vol;
        emit displayVolumeChanged();
        emit volumeChanged();
    }

    qreal getVolume(){
        return volume;
    }

    void setDisplayVolume(qreal lvl){if(displayVolume !=lvl){ displayVolume=lvl; emit displayVolumeChanged();} }
    qreal getDisplayVolume(){return displayVolume;}

    void mediaStarted(){
        setMediaPlaying(true);
    }

    void setServerAddress(QString a) {if(serverAddress !=a) {serverAddress = a;emit serverAddressChanged();}}
    QString getServerAddress(){return serverAddress;}

    void setDeviceNumber(int d) { if(deviceNumber !=d) {deviceNumber = d; emit deviceNumberChanged();}}
    int getDeviceNumber() {return deviceNumber;}

    void setPrefinishMarkHit(qint32 t ) {}

    void setMediaBuffer(int b) {mediaBuffer = b; emit mediaBufferChanged();}
    int getMediaBuffer () {return mediaBuffer;}

    void setMediaError(QString e) {lastError = e; emit lastErrorChanged();}
    QString getMediaError() {return lastError;}

    void setErrorStatus(bool e) {hasError = e; emit hasErrorChanged();}
    bool getErrorStatus() {return hasError;}

    void setMediaPlaying(bool s) {
        if(mediaPlaying == s) return;
        mediaPlaying = s;
        qDebug() << "media playback changed in plugin!" << s;
        emit mediaPlayingChanged();
    }

    bool getMediaPlaying() {return mediaPlaying;}

    void setFileReference(QString f){
        qDebug() << "Updating file url.";
        fileReference = f;
        pluginUrl = fileReference;
        emit pluginUrlUpdated();
        startTimeCodeServer();
        qDebug() << "CPP Url Updated";
        emit fileReferenceChanged();
    }
    QString getFileReference() {return fileReference; }

    void setFileNumber(int n) {fileno = n; fileNumberChanged();}

    void transmit(QString d);
    void setStreamId(int id) {streamId = id; }

    void setMediaUrl(QString url);
    QString getMediaUrl(){ return fileUrl;}

    void setCurrentStatus(QString s) {currentStatus = QTime::currentTime().toString()+"::"+s; emit currentStatusChanged(); qDebug() <<currentStatus; }
    QString getCurrentStatus() {return currentStatus;}

    void setConnectionDetails(int r, QString s);

    void setConnectionStatus(bool stat){connected = stat; emit connectedChanged();}
    bool getConnectionStatus(){return connected;}

    void setWindowSize(int h, int w) {
        Q_UNUSED(h);
        Q_UNUSED(w);
    }

    void newClientConnected();
    void callBackClientConnected();
    void startTimeCodeServer();
    void stopTimeCodeServer();
    void handleError(){
    }
    void forwardCallbackData();

    void setMediaPosition(int msec) {
        qDebug() << "Seeking to " << msec << " msec";
        updatePluginSeek(msec);
    }

    void setZoomLevel(QString zoom);
    void setAspectRatio(QString aspect);

    QImage getScreenShot();

    void setVideoSize(int h, int w) {
        Q_UNUSED(h);
        Q_UNUSED(w);
    }

    void setQmlTotalTime(int inSec){
        setCurrentStatus("Android Time call in Audio Visual Plugin::"+QString::number(inSec));
        int s = inSec ;

        int seconds = s / 1000;
        int displayHours = seconds / (3600);
        int remainder = seconds % 3600;
        int minutes = remainder / 60;
        int forseconds = remainder % 60;
        QString hrs = QString::number(displayHours);
        if(hrs.length()==1)
            hrs.prepend("0");

        QString min = QString::number(minutes);
        if(min.length()==1)
            min.prepend("0");

        QString sec = QString::number(forseconds);
        if(sec.length()==1)
            sec.prepend("0");

        qs_totalTime =hrs + ":" + min + ":" +sec;
        totalTime = s;
        qDebug() << qs_totalTime;
        emit totalTimeChanged();
        processTimeCode(0);
    }

    void setTotalTime(qint64 t) {
        int seconds = t / 1000;
        int displayHours = seconds / (3600);
        int remainder = seconds % 3600;
        int minutes = remainder / 60;
        int forseconds = remainder % 60;
        QString hrs = QString::number(displayHours);
        if(hrs.length()==1)
            hrs.prepend("0");

        QString min = QString::number(minutes);
        if(min.length()==1)
            min.prepend("0");

        QString sec = QString::number(forseconds);
        if(sec.length()==1)
            sec.prepend("0");

        qs_totalTime =hrs + ":" + min + ":" +sec;
        totalTime = t;
        emit totalTimeChanged();
    }


    QString getTotalTime() {return qs_totalTime;}

    void processTimeCode(qint64 f);

    void reInit(){
        setConnectionStatus(false);
        mediaPlayer=NULL;
        initializePlayer();
    }

    void pluginNotifyStart(QString title, QString audioOptions="", QString videoOptions=""){
        startTimeCodeServer();
        mediaPlayer->EVENT_Playback_Started(
                    mediaPlayer->getInternalMediaUrl().toStdString(),
                    mediaPlayer->i_StreamId,
                    title.toStdString(),
                    audioOptions.toStdString(),
                    videoOptions.toStdString());
    }
    void pluginNotifyEnd(bool withError){
        mediaPlayer->EVENT_Playback_Completed(mediaPlayer->getInternalMediaUrl().toStdString(), mediaPlayer->i_StreamId, withError);
    }

    //Event slots
    void playbackInfoUpdated(QString mediaTitle, QString mediaSubTitle, QString name, int screen);

private:
    void initializePlayer();
    void initializeConnections();
    void shutdownDevice();
    bool mountDrive(long device);

private slots:

    void setupDirectories();
};


QML_DECLARE_TYPE(MediaManager)

#endif // MEDIAMANAGER_H
