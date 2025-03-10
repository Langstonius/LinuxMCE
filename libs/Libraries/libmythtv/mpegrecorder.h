#ifndef MPEGRECORDER_H_
#define MPEGRECORDER_H_

#include "recorderbase.h"

struct AVFormatContext;
struct AVPacket;

class MpegRecorder : public RecorderBase
{
  public:
    MpegRecorder(TVRec*);
   ~MpegRecorder();
    void TeardownAll(void);

    void SetOption(const QString &opt, int value);
    void SetOption(const QString &name, const QString &value);
    void SetVideoFilters(QString &filters);

    void SetOptionsFromProfile(RecordingProfile *profile,
                               const QString &videodev, 
                               const QString &audiodev,
                               const QString &vbidev);

    void Initialize(void);
    void StartRecording(void);
    void StopRecording(void);
    void Reset(void);

    void Pause(bool clear = true);

    bool IsRecording(void);
    bool IsErrored(void) { return errored; }

    long long GetFramesWritten(void);

    bool Open(void);
    int GetVideoFd(void);

    long long GetKeyframePosition(long long desired);

    void SetNextRecording(const ProgramInfo*, RingBuffer*);

  public slots:
    void deleteLater(void);

  private:
    bool SetupRecording();
    void FinishRecording();
    void HandleKeyframe(void);
    void SavePositionMap(bool force);

    void ProcessData(unsigned char *buffer, int len);

    bool OpenMpegFileAsInput(void);
    bool OpenV4L2DeviceAsInput(void);

    void ResetForNewFile(void);

    bool errored;
    bool deviceIsMpegFile;
    int bufferSize;

    bool recording;
    bool encoding;

    bool paused;
    bool mainpaused;
    bool cleartimeonpause;

    long long framesWritten;

    int width, height;
    int bitrate, maxbitrate, streamtype, aspectratio;
    int audtype, audsamplerate, audbitratel1, audbitratel2;
    int audvolume;

    int chanfd;
    int readfd;

    int keyframedist;
    bool gopset;

    QMutex                     positionMapLock;
    QMap<long long, long long> positionMap;
    QMap<long long, long long> positionMapDelta;

    static const int audRateL1[];
    static const int audRateL2[];
    static const char* streamType[];
    static const char* aspectRatio[];

    unsigned int leftovers;
    long long lastpackheaderpos;
    long long lastseqstart;
    long long numgops;

    unsigned char *buildbuffer;
    unsigned int buildbuffersize;
};
#endif
