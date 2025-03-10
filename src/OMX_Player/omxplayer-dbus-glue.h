
/*
 *	This file was automatically generated by dbusxx-xml2cpp; DO NOT EDIT!
 */

#ifndef __dbusxx__omxplayer_dbus_glue_h__PROXY_MARSHAL_H
#define __dbusxx__omxplayer_dbus_glue_h__PROXY_MARSHAL_H

#include <dbus-c++/dbus.h>
#include <cassert>

namespace org {
namespace freedesktop {
namespace DBus {

class Properties_proxy
: public ::DBus::InterfaceProxy
{
public:

    Properties_proxy()
    : ::DBus::InterfaceProxy("org.freedesktop.DBus.Properties")
    {
    }

public:

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    bool CanQuit()
    {
        ::DBus::CallMessage call;
        call.member("CanQuit");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool Fullscreen()
    {
        ::DBus::CallMessage call;
        call.member("Fullscreen");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool CanSetFullscreen()
    {
        ::DBus::CallMessage call;
        call.member("CanSetFullscreen");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool CanRaise()
    {
        ::DBus::CallMessage call;
        call.member("CanRaise");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool HasTrackList()
    {
        ::DBus::CallMessage call;
        call.member("HasTrackList");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    std::string Identity()
    {
        ::DBus::CallMessage call;
        call.member("Identity");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    std::vector< std::string > SupportedUriSchemes()
    {
        ::DBus::CallMessage call;
        call.member("SupportedUriSchemes");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::vector< std::string > argout;
        ri >> argout;
        return argout;
    }

    std::vector< std::string > SupportedMimeTypes()
    {
        ::DBus::CallMessage call;
        call.member("SupportedMimeTypes");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::vector< std::string > argout;
        ri >> argout;
        return argout;
    }

    bool CanGoNext()
    {
        ::DBus::CallMessage call;
        call.member("CanGoNext");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool CanGoPrevious()
    {
        ::DBus::CallMessage call;
        call.member("CanGoPrevious");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool CanSeek()
    {
        ::DBus::CallMessage call;
        call.member("CanSeek");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool CanControl()
    {
        ::DBus::CallMessage call;
        call.member("CanControl");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool CanPlay()
    {
        ::DBus::CallMessage call;
        call.member("CanPlay");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool CanPause()
    {
        ::DBus::CallMessage call;
        call.member("CanPause");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    std::string PlaybackStatus()
    {
        ::DBus::CallMessage call;
        call.member("PlaybackStatus");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    std::string GetSource()
    {
        ::DBus::CallMessage call;
        call.member("GetSource");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    double Volume()
    {
        ::DBus::CallMessage call;
        call.member("Volume");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        double argout;
        ri >> argout;
        return argout;
    }

    double Volume(const double& volume)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << volume;
        call.member("Volume");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        double argout;
        ri >> argout;
        return argout;
    }

    void Mute()
    {
        ::DBus::CallMessage call;
        call.member("Mute");
        ::DBus::Message ret = invoke_method (call);
    }

    void Unmute()
    {
        ::DBus::CallMessage call;
        call.member("Unmute");
        ::DBus::Message ret = invoke_method (call);
    }

    int64_t Position()
    {
        ::DBus::CallMessage call;
        call.member("Position");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int64_t argout;
        ri >> argout;
        return argout;
    }

    double Aspect()
    {
        ::DBus::CallMessage call;
        call.member("Aspect");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        double argout;
        ri >> argout;
        return argout;
    }

    int64_t VideoStreamCount()
    {
        ::DBus::CallMessage call;
        call.member("VideoStreamCount");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int64_t argout;
        ri >> argout;
        return argout;
    }

    int64_t ResWidth()
    {
        ::DBus::CallMessage call;
        call.member("ResWidth");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int64_t argout;
        ri >> argout;
        return argout;
    }

    int64_t ResHeight()
    {
        ::DBus::CallMessage call;
        call.member("ResHeight");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int64_t argout;
        ri >> argout;
        return argout;
    }

    int64_t Duration()
    {
        ::DBus::CallMessage call;
        call.member("Duration");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int64_t argout;
        ri >> argout;
        return argout;
    }

    double MinimumRate()
    {
        ::DBus::CallMessage call;
        call.member("MinimumRate");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        double argout;
        ri >> argout;
        return argout;
    }

    double MaximumRate()
    {
        ::DBus::CallMessage call;
        call.member("MaximumRate");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        double argout;
        ri >> argout;
        return argout;
    }


public:

    /* signal handlers for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
};

} } } 
namespace org {
namespace mpris {
namespace MediaPlayer2 {

class Player_proxy
: public ::DBus::InterfaceProxy
{
public:

    Player_proxy()
    : ::DBus::InterfaceProxy("org.mpris.MediaPlayer2.Player")
    {
    }

public:

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    void Next()
    {
        ::DBus::CallMessage call;
        call.member("Next");
        ::DBus::Message ret = invoke_method (call);
    }

    void Previous()
    {
        ::DBus::CallMessage call;
        call.member("Previous");
        ::DBus::Message ret = invoke_method (call);
    }

    void Pause()
    {
        ::DBus::CallMessage call;
        call.member("Pause");
        ::DBus::Message ret = invoke_method (call);
    }

    void Stop()
    {
        ::DBus::CallMessage call;
        call.member("Stop");
        ::DBus::Message ret = invoke_method (call);
    }

    int64_t Seek(const int64_t& microseconds)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << microseconds;
        call.member("Seek");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int64_t argout;
        ri >> argout;
        return argout;
    }

    int64_t SetPosition(const std::string& path, const int64_t& position)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << path;
        wi << position;
        call.member("SetPosition");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int64_t argout;
        ri >> argout;
        return argout;
    }

    int64_t SetAlpha(const std::string& path, const int64_t& alpha)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << path;
        wi << alpha;
        call.member("SetAlpha");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int64_t argout;
        ri >> argout;
        return argout;
    }

    std::string SetAspectMode(const std::string& path, const std::string& aspectMode)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << path;
        wi << aspectMode;
        call.member("SetAspectMode");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    std::vector< std::string > ListSubtitles()
    {
        ::DBus::CallMessage call;
        call.member("ListSubtitles");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::vector< std::string > argout;
        ri >> argout;
        return argout;
    }

    std::string VideoPos(const ::DBus::Path& path, const std::string& win)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << path;
        wi << win;
        call.member("VideoPos");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    std::string SetVideoCropPos(const ::DBus::Path& path, const std::string& win)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << path;
        wi << win;
        call.member("SetVideoCropPos");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    void HideVideo()
    {
        ::DBus::CallMessage call;
        call.member("HideVideo");
        ::DBus::Message ret = invoke_method (call);
    }

    void UnHideVideo()
    {
        ::DBus::CallMessage call;
        call.member("UnHideVideo");
        ::DBus::Message ret = invoke_method (call);
    }

    std::vector< std::string > ListAudio()
    {
        ::DBus::CallMessage call;
        call.member("ListAudio");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::vector< std::string > argout;
        ri >> argout;
        return argout;
    }

    std::vector< std::string > ListVideo()
    {
        ::DBus::CallMessage call;
        call.member("ListVideo");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::vector< std::string > argout;
        ri >> argout;
        return argout;
    }

    bool SelectSubtitle(const int32_t& index)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << index;
        call.member("SelectSubtitle");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    bool SelectAudio(const int32_t& index)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << index;
        call.member("SelectAudio");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        bool argout;
        ri >> argout;
        return argout;
    }

    void ShowSubtitles()
    {
        ::DBus::CallMessage call;
        call.member("ShowSubtitles");
        ::DBus::Message ret = invoke_method (call);
    }

    void HideSubtitles()
    {
        ::DBus::CallMessage call;
        call.member("HideSubtitles");
        ::DBus::Message ret = invoke_method (call);
    }

    void Action(const int32_t& key_val)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << key_val;
        call.member("Action");
        ::DBus::Message ret = invoke_method (call);
    }


public:

    /* signal handlers for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
};

} } } 
namespace org {
namespace mpris {
namespace MediaPlayer2 {

class Root_proxy
: public ::DBus::InterfaceProxy
{
public:

    Root_proxy()
    : ::DBus::InterfaceProxy("org.mpris.MediaPlayer2.Root")
    {
    }

public:

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    void Quit()
    {
        ::DBus::CallMessage call;
        call.member("Quit");
        ::DBus::Message ret = invoke_method (call);
    }


public:

    /* signal handlers for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
};

} } } 
#endif //__dbusxx__omxplayer_dbus_glue_h__PROXY_MARSHAL_H
