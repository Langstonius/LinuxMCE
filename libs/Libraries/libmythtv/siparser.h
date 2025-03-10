/*
 *  Copyright 2004 - Taylor Jacob (rtjacob at earthlink.net)
 */

#ifndef SIPARSER_H
#define SIPARSER_H

// C includes
#include <cstdio>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <unistd.h>
#include <sys/types.h>

// C++ includes
#include <iostream>
using namespace std;

// Qt includes
#include <qstringlist.h>
#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qmap.h>
#include <qtextcodec.h>
#include <qmutex.h>

// MythTV includes
#include "sitypes.h"

class VirtualChannelTable;

/** \TODO Fix this size */
#define NumHandlers     7

/**
 *  Custom descriptors allow or disallow HUFFMAN_TEXT - For North American 
 *  DVB providers who use Huffman compressed guide in the 9x descriptors.
 */
#define CUSTOM_DESC_HUFFMAN_TEXT               1

/**
 *  Custom descriptors allow or disallow CHANNEL_NUMBERS - For the UK where
 *  channel numbers are sent in one of the SDT tables (at least per scan.c docs)
 */
#define CUSTOM_DESC_CHANNEL_NUMBERS         2

/**
 * The guide source pid.
 */
#define GUIDE_STANDARD                0

/**
 *  GUIDE_DATA_PID is for nonstandard PID being used for EIT style guide
 *  this is seen in North America (this only applies to DVB)
 */
#define GUIDE_DATA_PID                1

/**
 *  Post processing of the guide.  Some carriers put all of the event text
 *  into the description (subtitle, acotors, etc).  You can post processes these
 *  types of carriers EIT data using some simple RegExps to get more powerful
 *  guide data.  BellExpressVu in Canada is one example.
 */
#define GUIDE_POST_PROCESS_EXTENDED        1

class SIParser : public QObject
{
    Q_OBJECT
  public:
    SIParser(const char *name = "SIParser");
    ~SIParser();

    int Start(void);

    // Generic functions that will begin collection of tables based on the
    // SIStandard.
    bool FindTransports(void);
    bool FindServices(void);
    bool FillPMap(SISTANDARD _SIStandard);
    bool FillPMap(const QString&);

    bool AddPMT(uint16_t ServiceID);

    bool ReinitSIParser(const QString &si_std, uint service_id);

    // Stops all collection of data and clears all values (on a channel change for example)
    void Reset(void);

    virtual void AddPid(uint16_t pid,uint8_t mask,uint8_t filter,bool CheckCRC,int bufferFactor);
    virtual void DelPid(int pid);
    virtual void DelAllPids(void);

    // Functions that may become signals for communication with the outside world
    void ServicesComplete(void);
    void GuideComplete(void);
//    void EventsReady(void);

    // Functions to get objects into other classes for manipulation
    bool GetTransportObject(NITObject &NIT);
    bool GetServiceObject(QMap_SDTObject &SDT);

    void ParseTable(uint8_t* buffer, int size, uint16_t pid);
    void CheckTrackers(void);

    // Functions that allow you to initialize the SIParser
    void AddToServices(const VirtualChannelTable &vct);

  public slots:
    virtual void deleteLater(void);

  signals:
    void FindTransportsComplete(void);
    void FindServicesComplete(void);
    void FindEventsComplete(void);
    void EventsReady(QMap_Events* Events);
    void AllEventsPulled(void);
    void TableLoaded(void);
    void UpdatePMT(const PMTObject *pmt);

  protected:
    void PrintDescriptorStatistics(void) const;

  private:
    uint GetLanguagePriority(const QString &language);

    // Fixes for various DVB Network Spec Deviations
    void LoadDVBSpecDeviations(uint16_t NetworkID);

    void LoadPrivateTypes(uint16_t networkID);


    // MPEG Transport Parsers (ATSC and DVB)
    tablehead_t       ParseTableHead(uint8_t* buffer, int size);

    void ParsePAT(uint pid, tablehead_t* head, uint8_t* buffer, uint size);
    void ParseCAT(uint pid, tablehead_t* head, uint8_t* buffer, uint size);
    void ParsePMT(uint pid, tablehead_t* head, uint8_t* buffer, uint size);

    void ProcessUnusedDescriptor(uint pid, const uint8_t *buffer, uint size);

    // Common Helper Functions
    QString DecodeText(const uint8_t *s, uint length);

    // DVB Helper Parsers
    QDateTime ConvertDVBDate(const uint8_t* dvb_buf);

    // DVB Table Parsers
    void ParseNIT(uint pid, tablehead_t* head, uint8_t* buffer, uint size);
    void ParseSDT(uint pid, tablehead_t* head, uint8_t* buffer, uint size);
    void ParseDVBEIT(uint pid, tablehead_t* h, uint8_t* buffer, uint size);

    // Common Descriptor Parsers
    CAPMTObject ParseDescCA(uint8_t* buffer, int size);

    // DVB Descriptor Parsers
    void ParseDescNetworkName  (uint8_t* buf, int sz, NetworkObject   &n);
    void ParseDescLinkage      (uint8_t* buf, int sz, NetworkObject   &n);
    void ParseDescService      (uint8_t* buf, int sz, SDTObject       &s);
    void ParseDescFrequencyList(uint8_t* buf, int sz, TransportObject &t);
    void ParseDescUKChannelList(uint8_t* buf, int sz, QMap_uint16_t &num);
    TransportObject ParseDescTerrestrial     (uint8_t* buf, int sz);
    TransportObject ParseDescSatellite       (uint8_t* buf, int sz);
    TransportObject ParseDescCable           (uint8_t* buf, int sz);
    QString ParseDescLanguage                (const uint8_t*, uint sz);
    void ParseDescTeletext                   (uint8_t* buf, int sz);
    void ParseDescSubtitling                 (uint8_t* buf, int sz);

    // ATSC EIT Table Descriptor processors
    void ProcessDescHuffmanEventInfo         (uint8_t *buf, uint sz, Event &e);
    QString ProcessDescHuffmanText           (uint8_t *buf, uint sz);
    QString ProcessDescHuffmanTextLarge      (uint8_t *buf, uint sz);

    // DVB EIT Table Descriptor processors
    uint ProcessDVBEventDescriptors(
        uint                         pid,
        const unsigned char          *data,
        uint                         &bestPrioritySE,
        const unsigned char*         &bestDescriptorSE, 
        uint                         &bestPriorityEE,
        vector<const unsigned char*> &bestDescriptorsEE,
        Event                        &event);

    void ProcessContentDescriptor       (const uint8_t*, uint sz, Event &e);
    void ProcessShortEventDescriptor    (const uint8_t*, uint sz, Event &e);
    void ProcessExtendedEventDescriptor (const uint8_t*, uint sz, Event &e);
    void ProcessComponentDescriptor     (const uint8_t*, uint sz, Event &e);

    // ATSC Helper Parsers
    QDateTime ConvertATSCDate(uint32_t offset);
    QString ParseMSS(uint8_t* buffer, int size);

    // ATSC Table Parsers
    void ParseMGT       (tablehead_t* head, uint8_t* buffer, int size);
    void ParseRRT       (tablehead_t* head, uint8_t* buffer, int size);
    void ParseATSCEIT   (tablehead_t* head, uint8_t* buffer, int size, uint16_t pid);
    void ParseETT       (tablehead_t* head, uint8_t* buffer, int size, uint16_t pid);
    void ParseSTT       (tablehead_t* head, uint8_t* buffer, int size);
    void ParseDCCT      (tablehead_t* head, uint8_t* buffer, int size);
    void ParseDCCSCT    (tablehead_t* head, uint8_t* buffer, int size);

    // ATSC Descriptor Parsers
    QString ParseATSCExtendedChannelName(uint8_t* buffer, int size);
    void ParseDescATSCContentAdvisory(uint8_t* buffer, int size);

    /* Huffman Text Decompression Routines */
    int HuffmanGetRootNode(uint8_t Input, uint8_t Table[]);
    bool HuffmanGetBit(uint8_t test[], uint16_t bit);
    QString HuffmanToQString(uint8_t test[], uint16_t size,uint8_t Table[]);

    int Huffman2GetBit(int bit_index, unsigned char *byteptr);
    uint16_t Huffman2GetBits(int bit_index, int bit_count,
                             unsigned char *byteptr);
    int Huffman2ToQString(unsigned char *compressed, int length, int table,
                          QString &Decompressed);

    void InitializeCategories(void);

    // EIT Fix Up Functions
    void EITFixUp      (Event &event);
    void EITFixUpStyle1(Event &event);
    void EITFixUpStyle2(Event &event);
    void EITFixUpStyle3(Event &event);
    void EITFixUpStyle4(Event &event);

  private:
    bool PAT_ready;
    bool PMT_ready;

    // Timeout Variables
    QDateTime TransportSearchEndTime;
    QDateTime ServiceSearchEndTime;
    QDateTime EventSearchEndTime;

    // Common Variables
    int                 SIStandard;
    uint                CurrentTransport;
    uint                RequestedServiceID;
    uint                RequestedTransportID;
    
    // Preferred languages and their priority
    QMap<QString,uint>  LanguagePriority;

    // DVB Variables
    uint                NITPID;

    // Storage Objects (DVB)
    QValueList<CAPMTObject> CATList;
    NITObject           NITList;

    // Storage Objects (ATSC)
    QMap<uint,uint>     sourceid_to_channel;

    // Storage Objects (ATSC & DVB)
    QMap2D_Events       EventMapObject;
    QMap_Events         EventList;

    // Mutex Locks
    // TODO: Lock Events, and Services, Transports, etc
    QMutex              pmap_lock;

    int                 ThreadRunning;
    bool                exitParserThread;
    TableSourcePIDObject TableSourcePIDs;
    bool                ParserInReset;
    bool                standardChange;

    // New tracking objects
    TableHandler       *Table[NumHandlers+1];
    privateTypes        PrivateTypes;
    bool                PrivateTypesLoaded;

    // DVB category descriptions
    QMap<uint,QString>  m_mapCategories;

    // statistics
    QMap<uint,uint>     descCount;
    mutable QMutex      descLock;
};

#endif // SIPARSER_H

