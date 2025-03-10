// -*- Mode: c++ -*-
#ifndef PREVIEW_GENERATOR_H_
#define PREVIEW_GENERATOR_H_

#include <pthread.h>

#include <qstring.h>
#include <qmutex.h>
#include <qsocket.h>

#include "programinfo.h"
#include "util.h"

class PreviewGenerator : public QObject
{
    Q_OBJECT
  public:
    PreviewGenerator(const ProgramInfo *pginfo, bool local_only = true);
    virtual ~PreviewGenerator();

    void Start(void);
    void Run(void);

    static bool SavePreview(QString filename,
                            const unsigned char *data,
                            uint width, uint height, float aspect);

    static char *GetScreenGrab(const ProgramInfo *pginfo,
                               const QString &filename, int secondsin,
                               int &bufferlen,
                               int &video_width, int &video_height,
                               float &video_aspect);

    void disconnectSafe(void);

  signals:
    void previewThreadDone(const QString&, bool&);
    void previewReady(const ProgramInfo*);

  protected slots:
    void EventSocketConnected();
    void EventSocketClosed();
    void EventSocketRead();

  private:
    bool RemotePreviewSetup(void);
    void RemotePreviewRun(void);
    void RemotePreviewTeardown(void);
    void LocalPreviewRun(void);
    bool IsLocal(void) const;
    static void *PreviewRun(void*);

    QMutex             previewLock;
    pthread_t          previewThread;
    ProgramInfo        programInfo;

    bool               localOnly;
    bool               isConnected;
    bool               createSockets;
    QSocket           *eventSock;
    QSocketDevice     *serverSock;
};

#endif // PREVIEW_GENERATOR_H_
