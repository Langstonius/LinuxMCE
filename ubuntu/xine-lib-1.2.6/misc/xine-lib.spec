%define shortname  libxine
%define name       libxine1
%define version    1.2.6
%define release    0

%define major      1
%define libname    %{shortname}%{major}

# Build separat packages:
# 1 create a sparate package
# 0 include files in main package
%if %{?BUILD_AA:0}%{!?BUILD_AA:1}
%define BUILD_AA        0
%endif
%if %{?BUILD_ALSA:0}%{!?BUILD_ALSA:1}
%define BUILD_ALSA      0
%endif
%if %{?BUILD_ARTS:0}%{!?BUILD_ARTS:1}
%define BUILD_ARTS      0
%endif
%if %{?BUILD_DEVEL:0}%{!?BUILD_DEVEL:1}
%define BUILD_DEVEL     1
%endif
%if %{?BUILD_DVB:0}%{!?BUILD_DVB:1}
%define BUILD_DVB       0
%endif
%if %{?BUILD_DVD:0}%{!?BUILD_DVD:1}
%define BUILD_DVD       0
%endif
%if %{?BUILD_DXR3:0}%{!?BUILD_DXR3:1}
%define BUILD_DXR3      0
%endif
%if %{?BUILD_ESD:0}%{!?BUILD_ESD:1}
%define BUILD_ESD       0
%endif
%if %{?BUILD_FLAC:0}%{!?BUILD_FLAC:1}
%define BUILD_FLAC      0
%endif
%if %{?BUILD_GNOME_VFS:0}%{!?BUILD_GNOME_VFS:1}
%define BUILD_GNOME_VFS 0
%endif
%if %{?BUILD_OGG:0}%{!?BUILD_OGG:1}
%define BUILD_OGG       0
%endif
%if %{?BUILD_OPENGL:0}%{!?BUILD_OPENGL:1}
%define BUILD_OPENGL    0
%endif
%if %{?BUILD_SDL:0}%{!?BUILD_SDL:1}
%define BUILD_SDL       0
%endif
%if %{?BUILD_DIRECTFB:0}%{!?BUILD_DIRECTFB:1}
%define BUILD_DIRECTFB  0
%endif
%if %{?BUILD_SYNCFB:0}%{!?BUILD_SYNCFB:1}
%define BUILD_SYNCFB    0
%endif
%if %{?BUILD_W32DLL:0}%{!?BUILD_W32DLL:1}
%define BUILD_W32DLL    0
%endif
%if %{?BUILD_XVMC:0}%{!?BUILD_XVMC:1}
%define BUILD_XVMC      0
%endif
%if %{?BUILD_STK:0}%{!?BUILD_STK:1}
%define BUILD_STK       0
%endif
%if %{?BUILD_JACK:0}%{!?BUILD_JACK:1}
%define BUILD_JACK      0
%endif
%if %{?BUILD_PULSE:0}%{!?BUILD_PULSE:1}
%define BUILD_PULSE     0
%endif

Name:           %{name}
Summary:        A portable video/audio library for unix-like systems.
Summary(cs):    Přenositelná video a audio knihovna pro unixovské systémy
Summary(de):    Eine portabele Audio-/Video-Bibliothek für unixartige Systeme.
Summary(fi):    Joustava video- ja ääniohjelmointikirjasto Unix-tyylisille käyttöjärjestelmille.
Version:        %{version}
Release:        %{release}
License:        GPL
Group:          Development/Libraries
URL:            http://www.xine-project.org
Source:         http://prdownloads.sourceforge.net/xine/xine-lib-1.2.6.tar.bz2
Packager:       Manfred Tremmel <Manfred.Tremmel@iiv.de>
Obsoletes:      xine
Obsoletes:      xine-lib
Obsoletes:      xine-lib-oss
Obsoletes:      xine-lib-xv
Obsoletes:      libxine0
Obsoletes:      %{shortname}
Obsoletes:      xine-vcdx
Provides:       xine
Provides:       xine-lib
Provides:       %{shortname} = %{version}-%{release}
Provides:       xine-vcdx
BuildRoot:      %{_tmppath}/%{name}-buildroot

%description
libxine is the beating heart of xine (a free gpl-licensed video player for
unix-like systems) which among others provides support for decoding (and
playing back) of many today available audio/video codecs, like mpeg-4 (DivX),
mpeg-2 (DVD, SVCD), mpeg-1 (VCD), Quicktime and RealMedia just to name a few.
This library contains (and uses) numerous processor specific optimizations to
provide a smooth playback and to minimize the overall demand of CPU power.

Don't hesitate to use libxine in your own projects as long as your usage
complies to the GPL. More information about GPL-license can be found at
http://www.gnu.org/licenses/gpl.html

%description -l cs
libxine je tepající srdce xine (svobodného videopřehrávače pod licencí GPL pro
unixovské systémy). Mimo jiné poskytuje podporu pro dekódování (a přehrávání)
mnoha dnes dostupnými audio a video kodeky jako jsou např. mpeg-4 (DivX),
mpeg-2 (DVD, SVCD), mpeg-1 (VCD), Quicktime a RealMedia. Tato knihovna používá
četné procesorově závislé optimalizace k dosažení plynulého přehrávání a
zmenšení celkového nároku na výkon procesoru.

Dokud to bude ve shodě s GPL, neváhejte použít libxine ve vašich vlastních
projektech. Více informací o GPL licenci můžete nalézt na
http://www.gnu.org/licenses/gpl.html.

%description -l de
libxine ist das Herzstück von xine (einem freien, GPL lizensiertem Audio- und
Video-Abspielprogramm für unixartige Systeme). libxine stellt die Funktionen
zur Dekodierung und Wiedergabe vieler aktueller Audio- und Videocodecs zur
Verfügung, wie z.B. MPEG-4 (DivX), MPEG-2 (DVD, SVCD) und MPEG-1 (VCD),
Quicktime und RealMedia um nur einige zu nennen.
Die Bibliothek enthält und benutzt eine Vielzahl von prozessorspezifischen
Optimierungen, um eine flüssige Wiedergabe mit minimaler Prozessorbelastung
gewährleisten zu könnnen.

Zögern Sie nicht libxine in Ihren eigenen Projekten zu nutzen. Beachten Sie
hierzu jedoch die in der GPL Lizenz vereinbarten Bestimmungen. Weitere
Informationen zur GPL-Lizenz finden Sie unter http://www.gnu.org/licenses/gpl.html

%description -l fi
libxine on xinen sydän (vapaa GPL-linsensoitu videosoitinohjelma Unix-tyylisille
käyttöjärjestelmille), joka muun muassa tarjoaa mahdollisuudet pakatun videon
ja äänen purkamiseen (sekä näyttämiseen) suurimmalla osalla nykyaikaista äänen-
ja kuvanpakkausformaateista.
Tällaisia ovat mpeg-4 (DivX;-)), mpeg-2 (DVD, SVCD), mpeg-1 (VCD) muutamia
mainitaksemme. xinen purkukirjasto käyttää monia matalan tason prosessoripohjaisia
optimisaatiomenetelmiä tarjotakseen sulavan kuvantoistokokemuksen. Tämä on tarpeen
myös minimoimaan tarvittavaa prosessoritehoa.

Olet tervetullut käyttämään libxine:a omissa projekteissasi kunhan ne ovat GPL-
lisenssin kanssa yhteensopivia. lisätietoja GPL-lisenssistä löytyy osoitteesta:
http://www.gnu.org/licenses/gpl.html

%if %BUILD_DEVEL
%package -n %{shortname}-devel
Summary:        Header files and documentation to develope programs with libxine.
Summary(cs):    Hlavičkové soubory a dokumentace pro vývoj programů používající libxine
Summary(de):    Headerdateien und Dokumentationen, um Programme mit libxine entwickeln zu können.
Summary(fi):    Header-tiedostot ja dokumentaatio, joita tarvitset kehittäessäsi ohjelmia libxine:n kanssa.
Group:          Development/Libraries
Obsoletes:      xine-lib-devel
Obsoletes:      xine-devel
Obsoletes:      libxine0-devel
Obsoletes:      %{name}-devel
Provides:       %{name}-devel = %{version}-%{release}
Provides:       xine-devel
Requires:       %{libname} = %{version}-%{release}

%description -n %{shortname}-devel
This package contains header files and documentation required to develope
programs with libxine.

libxine is the beating heart of xine (a free gpl-licensed video player for
unix-like systems) which among others provides support for decoding (and
playing back) of many today available audio/video codecs, like mpeg-4 (DivX),
mpeg-2 (DVD, SVCD), mpeg-1 (VCD), Quicktime and RealMedia just to name a few.
This library contains (and uses) numerous processor specific optimizations to
provide a smooth playback and to minimize the overall demand of CPU power.

Don't hesitate to use libxine in your own projects as long as your usage
complies to the GPL. More information about GPL-license can be found at
http://www.gnu.org/licenses/gpl.html

%description -n %{shortname}-devel -l cs
Tento balíček obsahuje hlavičkové soubory a dokumentaci potřebnou pro vývoj
programů, které používají libxine.

libxine je tepající srdce xine (svobodného videopřehrávače pod licencí GPL pro
unixovské systémy). Mimo jiné poskytuje podporu pro dekódování (a přehrávání)
mnoha dnes dostupnými audio a video kodeky jako jsou např. mpeg-4 (DivX),
mpeg-2 (DVD, SVCD), mpeg-1 (VCD), Quicktime a RealMedia. Tato knihovna používá
četné procesorově závislé optimalizace k dosažení plynulého přehrávání a
zmenšení celkového nároku na výkon procesoru.

Dokud to bude ve shodě s GPL, neváhejte použít libxine ve vašich vlastních
projektech. Více informací o GPL licenci můžete nalézt na
http://www.gnu.org/licenses/gpl.html.

%description -n %{shortname}-devel -l de
Dieses Paket enthält die Headerdateien und Dokumentationen, um Programme
mit libxine entwickeln zu können.

libxine ist das Herzstück von xine (einem freien, GPL lizensiertem Audio- und
Video-Abspielprogramm für unixartige Systeme). libxine stellt die Funktionen
zur Dekodierung und Wiedergabe vieler aktueller Audio- und Videocodecs zur
Verfügung, wie z.B. MPEG-4 (DivX), MPEG-2 (DVD, SVCD) und MPEG-1 (VCD),
Quicktime und RealMedia um nur einige zu nennen.
Die Bibliothek enthält und benutzt eine Vielzahl von prozessorspezifischen
Optimierungen, um eine flüssige Wiedergabe mit minimaler Prozessorbelastung
gewährleisten zu könnnen.

Zögern Sie nicht libxine in Ihren eigenen Projekten zu nutzen. Beachten Sie
hierzu jedoch die in der GPL Lizenz vereinbarten Bestimmungen. Weitere
Informationen zur GPL-Lizenz finden Sie unter http://www.gnu.org/licenses/gpl.html

%description -n %{shortname}-devel -l fi
libxine on xinen sydän (vapaa GPL-linsensoitu videosoitinohjelma Unix-tyylisille
käyttöjärjestelmille), joka muun muassa tarjoaa mahdollisuudet pakatun videon
ja äänen purkamiseen (sekä näyttämiseen) suurimmalla osalla nykyaikaista äänen-
ja kuvanpakkausformaateista.
Tällaisia ovat mpeg-4 (DivX;-)), mpeg-2 (DVD, SVCD), mpeg-1 (VCD) muutamia
mainitaksemme. xinen purkukirjasto käyttää monia matalan tason prosessoripohjaisia
optimisaatiomenetelmiä tarjotakseen sulavan kuvantoistokokemuksen. Tämä on tarpeen
myös minimoimaan tarvittavaa prosessoritehoa.

Olet tervetullut käyttämään libxine:a omissa projekteissasi kunhan ne ovat GPL-
lisenssin kanssa yhteensopivia. lisätietoja GPL-lisenssistä löytyy osoitteesta:
http://www.gnu.org/licenses/gpl.html
%endif

%if %BUILD_ALSA
%package alsa
Summary:        libxine sound output plugin for alsa >= 0.9
Summary(cs):    Zvukový výstupní modul libxine pro ALSA >= 0.9
Summary(de):    libxine Soundausgabeplguin für Alsa >= 0.9
Summary(fi):    libxine-Ddnilisdke uudelle Linux:n ddniarkkitehtuurille (ALSA >= 0.9)
Group:          Development/Libraries
Obsoletes:      xine-lib-alsa09
Obsoletes:      libxine0-alsa09
Obsoletes:      %{libname}-alsa09
Provides:       %{libname}-alsa09
Requires:       %{libname} = %{version}-%{release}

%description alsa
libxine sound output plugin for alsa >= 0.9

%description alsa -l cs
Zvukový výstupní modul libxine pro ALSA >= 0.9.

%description alsa -l de
libxine Soundausgabe Plugin für alsa >= 0.9

%description alsa -l fi
libxine-ddnilisdke uudelle Linux:n ddniarkkitehtuurille (ALSA >= 0.9)
%endif

%if %BUILD_ARTS
%package arts
Summary:        libxine sound output plugin for arts (KDE-soundserver)
Summary(cs):    Zvukový výstupní modul libxine pro ARTS (zvukový server KDE)
Summary(de):    libxine Soundausgabeplugin für arts (KDE-Soundserver)
Summary(fi):    libxine-Ddnilisdke Arts ddnipalvelimelle (KDE:n ddnipalvelin)
Group:          Development/Libraries
Obsoletes:      xine-lib-arts
Obsoletes:      libxine0-arts
Requires:       %{libname} = %{version}-%{release}

%description arts
libxine sound output plugin for arts (KDE-soundserver)

%description arts -l cs
Zvukový výstupní modul libxine pro ARTS (zvukový server KDE).

%description arts -l de
libxine Soundausgabeplugin für arts (KDE-Soundserver)

%description arts -l fi
libxine-Ddnilisdke Arts ddnipalvelimelle (KDE:n ddnipalvelin)
%endif

%if %BUILD_ESD
%package esd
Summary:        libxine sound output plugin for enlightmend sound daemon
Summary(cs):    Zvukový výstupní modul libxine pro Enlightmend Sound Daemon
Summary(de):    libxine Soundausgabeplugin für den Enlightmend Sound Daemon
Summary(fi):    libxine-ddnilisdke Enlightmend ddnipalvelimelle
Group:          Development/Libraries
Obsoletes:      xine-lib-esd
Obsoletes:      libxine0-esd
Requires:       %{libname} = %{version}-%{release}

%description esd
libxine sound output plugin for enlightmend sound daemon (Gnome and Enlightment)

%description esd -l cs
Zvukový výstupní modul libxine pro Enlightmend Sound Daemon.

%description esd -l de
libxine Soundausgabeplugin für den Enlightmend Sound Daemon (Gnome und Enlightment)

%description esd -l fi
libxine-ddnilisdke Enlightmend ddnipalvelimelle
%endif

%if %BUILD_SDL
%package sdl
Summary:        libxine video output plugin for SDL-library (Simple DirectMedia Layer)
Summary(cs):    Videovýstupní modul libxine pro knihovnu SDL (Simple DirectMedia Layer)
Summary(de):    libxine Videoausgabeplguin für SDL-Bibliothek (Simple DirectMedia Layer)
Summary(fi):    libxine-Videolisdke SDL grafiikkakirjastolle (Simple DirectMedia Layer)
Group:          Development/Libraries
Obsoletes:      xine-lib-sdl
Obsoletes:      libxine0-sdl
Requires:       %{libname} = %{version}-%{release}

%description sdl
libxine video output plugin for SDL-library (Simple DirectMedia Layer)

%description sdl -l cs
Videovýstupní modul libxine pro knihovnu SDL (Simple DirectMedia Layer).

%description sdl -l de
libxine Videoausgabeplguin für SDL-Bibliothek (Simple DirectMedia Layer)

%description sdl -l fi
libxine-Videolisdke SDL-grafiikkakirjastolle (Simple DirectMedia Layer)
%endif

%if %BUILD_AA
%package aa
Summary:        libxine video output plugin for aa-library (ASCII Art)
Summary(cs):    Videovýstupní modul libxine pro knihovnu aa (ASCII Art)
Summary(de):    libxine Videoausgabeplugin für aa-Bibliothek (ASCII Art)
Summary(fi):    libxine-Videolisdke aa-grafiikkakirjastolle (ASCII Art)
Group:          Development/Libraries
Obsoletes:      xine-lib-aa
Obsoletes:      libxine0-aa
Obsoletes:      xine-extra
Provides:       xine-extra
Requires:       %{libname} = %{version}-%{release}

%description aa
libxine video output plugin for aa-library (ASCII Art)

%description aa -l cs
Videovýstupní modul libxine pro knihovnu aa (ASCII Art).

%description aa -l de
libxine Videoausgabeplugin für aa-Bibliothek (ASCII Art)

%description aa -l fi
libxine-Videolisdke aa-grafiikkakirjastolle (ASCII Art)
%endif

%if %BUILD_OPENGL
%package opengl
Summary:        libxine video output plugin using OpenGL (3D graphic cards)
Summary(cs):    Videovýstupní modul libxine používající OpenGL (3D grafické karty)
Summary(de):    libxine Videoausgabeplugin per OpenGL (3D Grafikkarte)
Group:          Development/Libraries
Obsoletes:      xine-lib-opengl
Obsoletes:      libxine0-opengl
Requires:       %{libname} = %{version}-%{release}

%description opengl
libxine video output plugin using OpenGL (3D graphic cards)

%description opengl -l cs
Videovýstupní modul libxine, který používá OpenGL (3D grafické karty).

%description opengl -l de
libxine Videoausgabeplugin per OpenGL (3D Grafikkarte)
%endif

%if %BUILD_SYNCFB
%package syncfb
Summary:        libxine video output plugin using synchroniced framebuffer (Matrox cards)
Summary(cs):    Videovýstupní modul libxine používající framebuffer (karty Matrox)
Summary(de):    libxine Videoausgabeplugin per synchronisiertem Framebuffer (Matrox Karten)
Summary(fi):    libxine-Videolisdke Matrox-ndyttvkorttien synkronisoitua ndyttvmuistia varten.
Group:          Development/Libraries
Obsoletes:      libxine0-syncfb
Requires:       %{libname} = %{version}-%{release}

%description syncfb
libxine video output plugin using synchroniced framebuffer (Matrox cards)

%description syncfb -l cs
Videovýstupní modul libxine, který používá synchronizovaný framebuffer (karty
Matrox).

%description syncfb -l de
libxine Videoausgabeplugin per synchronisiertem Framebuffer (Matrox Karten)

%description syncfb -l fi
libxine-Videolisdke Matrox-ndyttvkorttien synkronisoitua ndyttvmuistia varten.
%endif

%if %BUILD_DVD
%package dvd
Summary:        libxine input plugin for playing video-dvd's with dvd-navigation
Summary(cs):    Vstupní modul libxine na přehrávání VideoDVD s DVD navigací
Summary(de):    libxine Inputplugin zum abspielen von Video-DVDs mit DVD-Navigation
Summary(fi):    libxine-Lukulisdke, jolla kdyttdjd voi soittaa DVD-levyjd
Group:          Development/Libraries
Obsoletes:      xine-dvdnav
Requires:       %{libname} = %{version}-%{release}

%description dvd
libxine input plugin for playing video-dvd's with dvd-navigation

%description dvd -l cs
Vstupní modul libxine na přehrávání VideoDVD s DVD navigací.

%description dvd -l de
libxine Inputplugin zum abspielen von Video-DVDs mit DVD-Navigation

%description dvd -l fi
libxine-Lukulisdke, jolla kdyttdjd voi soittaa DVD-levyjd
%endif

%if %BUILD_DVB
%package dvb
Summary:        libxine input plugin for DigitalTV devices
Summary(cs):    Vstupní modul libxine pro karty digitální TV
Summary(de):    libxine Einabeplugin für digitale TV-Karten
Summary(fi):    libxine-Lukulisdke, jolla kdyttdjd voi katsella DigitalTV-korttien ohjelmia
Group:          Development/Libraries
Requires:       %{libname} = %{version}-%{release}

%description dvb
libxine input plugin for Digital TV (Digital Video Broadcast - DVB) devices
e.g. Hauppauge WinTV Nova supported by DVB drivers from Convergence

%description dvb -l cs
Vstupní modul libxine pro karty digitální televize (Digital Video Broadcast -
DVB) jako je např. Hauppauge WinTV Nova podporovaná ovladači DVB od
Convergence.

%description dvb -l de
libxine Eingabeplugin für digitale TV-Karten (Digital Video Broadcast - DVB)
z.B. wird die Hauppauge WinTV Nova von Convergence unterstützt.

%description dvb -l fi
libxine-Lukulisdke, jolla kdyttdjd voi katsella DigitalTV-korttien ohjelmia
Esimerkiksi Haupaugen WinTV kortit ovat tuettuja DVB ajureilla.
%endif

%if %BUILD_GNOME_VFS
%package gnome-vfs
Summary:        libxine input plugin for totem (a gnome frontend)
Summary(cs):    Vstupní modul libxine pro totem (frontend GNOME)
Summary(de):    libxine Einabeplugin für totem (ein Gnome Frontend)
Summary(fi):    libxine-Lukulisdke Totem ohjelmaa varten, joka on libxine:n Gnome2 kdyttvliittymd
Group:          Development/Libraries
Requires:       %{libname} = %{version}-%{release}

%description gnome-vfs
libxine input plugin for totem (a gnome frontend)

%description gnome-vfs -l cs
Vstupní modul libxine pro totem (frontend GNOME).

%description gnome-vfs -l de
libxine Einabeplugin für totem (ein Gnome Frontend)

%description gnome-vfs -l fi
libxine-Lukulisdke Totem ohjelmaa varten, joka on libxine Gnome2 kdyttvliittymd
%endif

%if %BUILD_FLAC
%package flac
Summary:        libxine sound input plugin for flac-files (Free Lossless Audio Codec)
Summary(cs):    Zvukový modul libxine pro dekódování souborů FLAC (Free Lossless Audio Codec)
Summary(de):    libxine Soundeinabeplugin für flac-Dateien (Free Lossless Audio Codec)
Summary(fi):    libxine-Ddnilisdke flac-tiedostojen toistamiseen (Free Lossless Audio Codec)
Group:          Development/Libraries
Requires:       %{libname} = %{version}-%{release}

%description flac
libxine sound input plugin for flac-files (Free Lossless Audio Codec)

%description flac -l cs
Zvukový modul libxine pro dekódování souborů FLAC (Free Lossless Audio Codec).

%description flac -l de
libxine Soundeinabeplugin für flac-Dateien (Free Lossless Audio Codec == Freier, verlustfreier Audio-Codec)

%description flac -l fi
libxine-Ddnilisdke flac-tiedostojen toistamiseen (Free Lossless Audio Codec)
%endif

%if %BUILD_OGG
%package ogg
Summary:        libxine sound/video input plugin for ogg/ogm-files
Summary(cs):    Zvukový modul libxine pro dekódování souborů OGG a OGM
Summary(de):    libxine Sound-/Videoeinabeplugin für ogg/ogm-Dateien
Summary(fi):    libxine-Ddni/Videolisdke Ogg/Ogm tiedostojen toistamiseen
Group:          Development/Libraries
Requires:       %{libname} = %{version}-%{release}
Obsoletes:      xine-lib-oggvorbis
Obsoletes:      %{libname}-oggvorbis
Obsoletes:      %{libname}-oggtheora
Provides:       xine-lib-oggvorbis
Provides:       %{libname}-oggvorbis
Provides:       %{libname}-oggtheora

%description ogg
libxine sound/video input plugin for ogg/ogm-files

%description ogg -l cs
Zvukový modul libxine pro dekódování souborů OGG a OGM.

%description ogg -l de
libxine Sound-/Videoeinabeplugin für ogg/ogm-Dateien

%description ogg -l fi
libxine-Ddni/Videolisdke Ogg/Ogm tiedostojen toistamiseen
%endif

%if %BUILD_DIRECTFB
%package directfb
Summary:        libxine video output plugin using libdirectfb
Summary(cs):    Videovýstupní modul libxine používající libdirectfb
Summary(de):    libxine Videoausgabeplugin per libdirectfb
Group:          Development/Libraries
Requires:       %{libname} = %{version}-%{release}

%description directfb
libxine video output plugin using libdirectfb

%description directfb -l cs
Videovýstupní modul libxine, který používá libdirectfb.

%description directfb -l de
libxine Videoausgabeplugin per libdirectfb
%endif

%if %BUILD_STK
%package stk
Summary:        libxine video output plugin using libstk
Summary(cs):    Videovýstupní modul libxine používající libstk
Summary(de):    libxine Videoausgabeplugin per libstk
Group:          Development/Libraries
Requires:       %{libname} = %{version}-%{release}

%description stk
libxine video output plugin using libstk (Set-top Toolkit)

%description stk -l cs
Videovýstupní modul libxine, který používá libstk.

%description stk -l de
libxine Videoausgabeplugin per libstk (Set-top Toolkit)
%endif

%if %BUILD_DXR3
%package dxr3
Summary:        libxine video output plugin using mpeg2 decoding cards with dxr3 decoder-chip
Summary(cs):    Videovýstupní modul libxine používající karty s čipem DXR3
Summary(de):    libxine Videoausgabeplugin, nutzt MPEG2-decoder-Karten mit dxr3 Decoder-Chip
Summary(fi):    libxine-Videolisdke MPEG2-videopurkukorteille, joissa on DXR3 purkusiru
Group:          Development/Libraries
Obsoletes:      xine-lib-dxr3
Obsoletes:      libxine0-dxr3
Requires:       %{libname} = %{version}-%{release}

%description dxr3
libxine video output plugin using mpeg2 decoding cards with dxr3 decoder-chip

%description dxr3 -l cs
Videovýstupní modul libxine používající karty, které dekódují MPEG2 pomocí čipu
DXR3.

%description dxr3 -l de
libxine Videoausgabeplugin, nutzt MPEG2-decoder-Karten mit dxr3 Decoder-Chip

%description dxr3 -l fi
libxine-Videolisdke MPEG2-videopurkukorteille, joissa on DXR3 purkusiru
%endif

%if %BUILD_XVMC
%package xvmc
Summary:        libxine video output plugin using XVideo-extension with motion compensation
Summary(cs):    Videovýstupní modul libxine používající rozšíření XVideo MC
Summary(de):    libxine Videoausgabeplugin per XVideo-Erweiterung mit Motion Compensation
Group:          Development/Libraries
Obsoletes:      libxine0-xvmc
Autoreqprov:    Off
Requires:       %{libname} = %{version}-%{release}

%description xvmc
libxine video output plugin using XVideo-extension with motion compensation

%description xvmc -l cs
Videovýstupní modul libxine, který používá rozšíření XVideo s kompenzací pohybu.

%description xvmc -l de
libxine Videoausgabeplugin per XVideo-Erweiterung mit Motion Compensation
%endif

%if %BUILD_W32DLL
%ifarch i386 i486 i586 i686 i786 i868 i986 k6 k7 athlon
%package w32dll
Summary:        libxine decoding plugin using win32 dlls for native not supported formats
Summary(cs):    Dekódovací modul libxine, který používá DLL knihovny WIN32
Summary(de):    libxine Dekodierplugin, nutzt Win32 dlls für natvie nicht unterstützte Formate
Summary(fi):    libxine-Purkulisdke, joka mahdollistaa Win32 DLL:n kdytvn.
Group:          Development/Libraries
Obsoletes:      xine-lib-w32dll
Obsoletes:      libxine0-w32dll
Requires:       %{libname} = %{version}-%{release}

%description w32dll
libxine decoding plugin using win32 dlls for native not supported formats

%description w32dll -l cs
Dekódovací modul libxine, který používá DLL knihovny WIN32 pro formáty
nepodporované přímo.

%description w32dll -l de
libxine Dekodierplugin, nutzt Win32 dlls für native nicht unterstützte Formate

%description w32dll -l fi
libxine-Purkulisdke, joka mahdollistaa Win32 DLL:n kdytvn.
%endif
%endif

%if %BUILD_JACK
%package jack
Summary:        libxine sound output plugin for the jack soundserver
Summary(de):    libxine Soundausgabeplugin für den jack-Soundserver
Group:          Development/Libraries
Obsoletes:      xine-lib-jack
Obsoletes:      libxine0-jack
Requires:       %{libname} = %{version}-%{release}

%description jack
libxine sound output plugin for the jack soundserver

%description jack -l cs
Zvukový výstupní modul libxine pro zvukový server jack.

%description jack -l de
libxine Soundausgabeplugin für den jack-Soundserver
%endif

%if %BUILD_PULSE
%package pulse
Summary:        libxine sound output plugin for the pulseaudio soundserver
Summary(de):    libxine Soundausgabeplugin für den pulseaudio-Soundserver
Group:          Development/Libraries
Obsoletes:      xine-lib-pulse
Obsoletes:      libxine0-pulse
Requires:       %{libname} = %{version}-%{release}

%description pulse
libxine sound output plugin for the pulseaudio soundserver

%description pulse -l cs
Zvukový výstupní modul libxine pro zvukový server pulseaudio.

%description pulse -l de
libxine Soundausgabeplugin für den pulseaudio-Soundserver
%endif


%prep
%setup -q -n xine-lib-1.2.6

%build
export CFLAGS="${RPM_OPT_FLAGS}"
export XINE_DOCPATH="%{_docdir}/%{name}"
export PKG_CONFIG="%{_bindir}/pkg-config"

if [ ! -f configure ]; then
   NO_CONFIGURE=1 ./autogen.sh
fi

#
# currently we do not use %%configure as it seems to cause trouble with
# certain automake produced configure scripts - depending on automake version.
# Use BUILD_ARGS envvar to pass extra parameters to configure (like --enable-dha-mod/etc...)
#
./configure --build=%{_target_platform} --prefix=%{_prefix} \
            --exec-prefix=%{_exec_prefix} --bindir=%{_bindir} \
            --sbindir=%{_sbindir} --sysconfdir=%{_sysconfdir} \
            --datadir=%{_datadir} --includedir=%{_includedir} \
            --libdir=%{_libdir} --libexecdir=%{_libexecdir} \
            --localstatedir=%{_localstatedir} \
            --sharedstatedir=%{_sharedstatedir} --mandir=%{_mandir} \
            --infodir=%{_infodir} --enable-directfb --enable-modplug \
%if %BUILD_STK
            --with-libstk \
%endif
            --without-internal-vcdlibs

# Error in libfaad when compiling with mmx or sse enabled, remove it
%{__mv} contrib/libfaad/Makefile contrib/libfaad/Makefile_save
%{__cat} contrib/libfaad/Makefile_save | %{__sed} -e "s/-mmmx/-mno-mmx/g" -e "s/-msse/-mno-sse/g" > contrib/libfaad/Makefile

%{__make} %{?jobs:-j%{jobs}}

%install
[ "${RPM_BUILD_ROOT}" != "/" ] && %{__rm} -rf ${RPM_BUILD_ROOT}
make DESTDIR=%{?buildroot:%{buildroot}} LIBRARY_PATH=%{?buildroot:%{buildroot}}%{_libdir} install

cd ${RPM_BUILD_ROOT}

echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_zw
%if %BUILD_DEVEL
echo "%doc README TODO AUTHORS COPYING ChangeLog" >> ${RPM_BUILD_DIR}/filelist_%{name}_zw
%else
echo "%doc README TODO AUTHORS COPYING ChangeLog doc/hackersguide/*.html doc/hackersguide/*.png doc/hackersguide/README" >> ${RPM_BUILD_DIR}/filelist_%{name}_zw
%endif
find . -type f | %{__sed} 's,^\.%{_datadir}/doc,\%doc %{_datadir}/doc,' | %{__sed} 's,^\.,,' >> ${RPM_BUILD_DIR}/filelist_%{name}_zw
find . -type l | %{__sed} 's,^\.%{_datadir}/doc,\%doc %{_datadir}/doc,' | %{__sed} 's,^\.,,' >> ${RPM_BUILD_DIR}/filelist_%{name}_zw
find . -type d | %{__grep} xine | %{__sed} 's,^\.,\%dir ,' >> ${RPM_BUILD_DIR}/filelist_%{name}_zw
%{__grep} -v "/man/" ${RPM_BUILD_DIR}/filelist_%{name}_zw | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__grep} "/man/" ${RPM_BUILD_DIR}/filelist_%{name}_zw | %{__sed} -e 's/$/\*/g' | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_zw

%if %BUILD_DEVEL
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_devel
echo "%doc doc/hackersguide/*.html doc/hackersguide/*.png doc/hackersguide/README" >> ${RPM_BUILD_DIR}/filelist_%{name}_devel
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "/include/|dhahelper\.o|libxine*\.(so|la)$|\.m4$" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_devel
%{__grep} -v -E "/include/|dhahelper\.o|libxine*\.(so|la)$|\.m4$" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_ALSA
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_alsa
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_ao_out_alsa\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_alsa
%{__grep} -v "xineplug_ao_out_alsa\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_ESD
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_esd
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_ao_out_esd\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_esd
%{__grep} -v "xineplug_ao_out_esd\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_DXR3
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_dxr3
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "dxr3" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_dxr3
%{__grep} -v "dxr3" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_SDL
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_sdl
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_vo_out_sdl\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_sdl
%{__grep} -v "xineplug_vo_out_sdl\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_AA
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_aa
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_vo_out_aa\."  ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_aa
%{__grep} -v "xineplug_vo_out_aa\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_OPENGL
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_opengl
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "xineplug_vo_out_opengl\.|README.opengl" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_opengl
%{__grep} -v -E "xineplug_vo_out_opengl\.|README.opengl" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_SYNCFB
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_syncfb
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "xineplug_vo_out_syncfb\.|README\.syncfb" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_syncfb
%{__grep} -v -E "xineplug_vo_out_syncfb\.|README\.syncfb" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_DIRECTFB
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_directfb
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "xineplug_vo_out_(xd|d)irectfb\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_directfb
%{__grep} -v -E "xineplug_vo_out_(xd|d)irectfb\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_STK
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_stk
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "xineplug_vo_out_stk\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_stk
%{__grep} -v -E "xineplug_vo_out_stk\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_XVMC
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_xvmc
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_vo_out_x[x|v]mc\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_xvmc
%{__grep} -v "xineplug_vo_out_x[x|v]mc\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_W32DLL
%ifarch i386 i486 i586 i686 i786 i868 i986 k6 k7 athlon
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_w32dll
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "xineplug_decode_qt\.|xineplug_decode_w32dll\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_w32dll
%{__grep} -v -E "xineplug_decode_qt\.|xineplug_decode_w32dll\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif
%endif

%if %BUILD_DVB
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_dvb
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "xineplug_inp_dvb\.|README\.dvb" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_dvb
%{__grep} -v -E "xineplug_inp_dvb\.|README\.dvb" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_DVD
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_dvd
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "xineplug_inp_dvd\.|README\.network_dvd" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_dvd
%{__grep} -v -E "xineplug_inp_dvd\.|README\.network_dvd" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_GNOME_VFS
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_gnome_vfs
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_inp_gnome_vfs\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_gnome_vfs
%{__grep} -v "xineplug_inp_gnome_vfs\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_FLAC
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_flac
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_flac\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_flac
%{__grep} -v "xineplug_flac\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif

%if %BUILD_OGG
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_ogg
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} -E "xineplug_decode_vorbis\.|xineplug_dmx_ogg\.|xineplug_decode_theora\.|xineplug_decode_speex\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_ogg
%{__grep} -v -E "xineplug_decode_vorbis\.|xineplug_dmx_ogg\.|xineplug_decode_theora\.|xineplug_decode_speex\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif
%if %BUILD_JACK
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_jack
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_ao_out_jack\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_jack
%{__grep} -v "xineplug_ao_out_jack\." ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif
%if %BUILD_PULSE
echo "%defattr(-,root,root)" > ${RPM_BUILD_DIR}/filelist_%{name}_pulse
%{__mv} ${RPM_BUILD_DIR}/filelist_%{name} ${RPM_BUILD_DIR}/filelist_%{name}_old
%{__grep} "xineplug_ao_out_pulse" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - >> ${RPM_BUILD_DIR}/filelist_%{name}_pulse
%{__grep} -v "xineplug_ao_out_pulse" ${RPM_BUILD_DIR}/filelist_%{name}_old | %{__cat} - > ${RPM_BUILD_DIR}/filelist_%{name}
%{__rm} ${RPM_BUILD_DIR}/filelist_%{name}_old
%endif


%clean
[ "${RPM_BUILD_ROOT}" != "/" ] && %{__rm} -rf ${RPM_BUILD_ROOT}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files -f ../filelist_%{name}

%if %BUILD_DEVEL
%files -n %{shortname}-devel -f ../filelist_%{name}_devel
%endif

%if %BUILD_ALSA
%files alsa -f ../filelist_%{name}_alsa
%endif

%if %BUILD_ARTS
%files arts -f ../filelist_%{name}_arts
%endif

%if %BUILD_ESD
%files esd -f ../filelist_%{name}_esd
%endif

%if %BUILD_DXR3
%files dxr3 -f ../filelist_%{name}_dxr3
%endif

%if %BUILD_SDL
%files sdl -f ../filelist_%{name}_sdl
%endif

%if %BUILD_AA
%files aa -f ../filelist_%{name}_aa
%endif

%if %BUILD_OPENGL
%files opengl -f ../filelist_%{name}_opengl
%endif

%if %BUILD_SYNCFB
%files syncfb -f ../filelist_%{name}_syncfb
%endif

%if %BUILD_DIRECTFB
%files directfb -f ../filelist_%{name}_directfb
%endif

%if %BUILD_STK
%files stk -f ../filelist_%{name}_stk
%endif

%if %BUILD_XVMC
%files xvmc -f ../filelist_%{name}_xvmc
%endif

%if %BUILD_W32DLL
%ifarch i386 i486 i586 i686 i786 i868 i986 k6 k7 athlon
%files w32dll -f ../filelist_%{name}_w32dll
%endif
%endif

%if %BUILD_DVB
%files dvb -f ../filelist_%{name}_dvb
%endif

%if %BUILD_DVD
%files dvd -f ../filelist_%{name}_dvd
%endif

%if %BUILD_GNOME_VFS
%files gnome-vfs -f ../filelist_%{name}_gnome_vfs
%endif

%if %BUILD_FLAC
%files flac -f ../filelist_%{name}_flac
%endif

%if %BUILD_OGG
%files ogg -f ../filelist_%{name}_ogg
%endif

%if %BUILD_JACK
%files jack -f ../filelist_%{name}_jack
%endif

%if %BUILD_PULSE
%files pulse -f ../filelist_%{name}_pulse
%endif

%changelog
* Sun Dec 09 2007 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- stk and arts plugins are no longer build by default, enable
  them, when subpackages are selected
- added optional subpackage for pulseaudio
- switched to external vcdlibs
- using macros for shell commands, when rpm provides them
- some other cleanups
* Sun Oct 15 2006 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- some little changes to enable caca plugin
* Sat Aug 26 2006 František Dvořák <valtri@users.sourceforge.net>
- tiny translation update
- fixed rpmbuild
* Mon Aug 14 2006 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- added jack subpackage for the jack soundserver plugin
- renamed alsa09 subpackage to alsa
* Wed Feb 15 2006 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- added dirs to filelist
* Tue Feb 14 2006 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- i386 section of libavcodec doesn't work with -O2 or -O3
* Thu Jan 06 2005 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- enabled defining build or not to build subpackages at runtime
  using '--define "BUILD_XYZ 1"', you can find possible BUILD_
  defines at the top of the spec-file
* Tue Dec 07 2004 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- included xxmc video out plugin into xvmc sub-rpm
* Sat Sep 11 2004 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- added missing Requires-Statements in the subpackages
* Fri Sep 03 2004 František Dvořák <valtri@users.sourceforge.net>
- Czech translation update
* Thu Sep 02 2004 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- konverted to UTF8
- some fixes for non IA32 systems (especialy Athlon64/Opteron)
* Sat May 01 2004 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- activated opengl plugin
* Tue Apr 06 2004 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- removed xvid modules from -devel package and put it back
  to main package.
* Fri Feb 13 2004 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- added new sub-rpm for stk videoout plugin
* Fri Nov 21 2003 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- use internal vcdlibs to keep dependencies low
* Sun Oct 19 2003 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- added separate directfb-package to solve dependendies
* Tue Oct 07 2003 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- activated separte xvmc package, when this is wanted.
* Sun Jul 20 2003 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- made ogg-package out of oggvorbis, oggtheora and the new
  oggspeex xine-plugins
* Sat May 24 2003 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- complete rework of the specfile
* Wed May 14 2003 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- Update for > 1-beta13
* Sat Mar 08 2003 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- added missing doc-files
* Wed Jan 15 2003 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- removed man3 manpages in devel-filelist
* Tue Dec 24 2002 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- Update for libxine 1.0beta1 added pkgconfig-directory
* Wed Dec 11 2002 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- Update for libxine 1.0beta0, fonts-directory has been changed
* Sat Dec 07 2002 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- added translation for finnish by Tuukka Pasanen <illuusio@mailcity.com>
- some updates to german and english descriptions
- added post-directory to file-list
* Sat Nov 09 2002 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- Final changes for libxine 1.0alpha1
* Mon Nov 04 2002 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- another change to make it run with next version
* Sat Nov 02 2002 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- Changes for next xine version
* Sun Oct 27 2002 Manfred Tremmel <Manfred.Tremmel@iiv.de>
- Update of the german descriptions.
* Thu Jun 06 2002 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- ability to pass args to configure. Fix vidix/dhahelper inclusion.
* Mon May 27 2002 Matthias Dahl <matthew2k@web.de>
- added passing of build parameter to configure
* Sun May 26 2002 Matthias Dahl <matthew2k@web.de>
- added german translations by Manfred Tremmel <Manfred.Tremmel@iiv.de>
- added fixes (missing -l cs) by Manfred Tremmel <Manfred.Tremmel@iiv.de>
* Sat May 18 2002 Matthias Dahl <matthew2k@web.de>
- added czech translations by valtri@atlas.cz
* Thu May 16 2002 Matthias Dahl <matthew2k@web.de>
- replaced %configure because it was causing trouble on certain configurations
* Sat May 11 2002 Matthias Dahl <matthew2k@web.de>
- reworked/revamped spec file, still needs some tuning (BuildPreReq, ...)
* Thu May 2 2002 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- sync with new logo stuff.
* Wed May 1 2002 Matthias Dahl <matthew2k@web.de>
- added cinepak, cyuv and msvc decode plugins
- commented out sdl related parts because this is currently broken in xine-lib
- added 'cs,pl_PL' translation catalogs
* Sun Mar 31 2002 Matthias Dahl <matthew2k@web.de>
- added mms input plugin, spucc decoder and missing xine_logo.zyuy2.
- commented out video fill decoder for now as it seems to be no longer built
* Wed Feb 6 2002 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- added 'de,fr,pt_BR' translation catalogs.
* Sat Dec 26 2001 Matthias Dahl <matthew2k@web.de>
- added sputext decode plugin and fonts.
* Sat Dec 8 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- ASF plugin is optional.
* Thu Dec 6 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- Add cda plugins.
* Wed Nov 14 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- fixed dxr3 header files inclusion, aalib deps: thanks to Andrew Meredith <andrew@anvil.org>.
* Mon Oct 29 2001 Matthias Dahl <matthew2k@web.de>
- added http input plugin
* Thu Oct 18 2001 Matthias Dahl <matthew2k@web.de>
- added asf demuxer plugin
* Sun Oct 14 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- move vorbis in separate package. Add DivX4 decoder plugin.
* Wed Oct 10 2001 Matthias Dahl <matthew2k@web.de>
- added vorbis files and missing man pages to filelist.
* Thu Sep 27 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- Add desktop stuff from patches by Miguel Freitas <miguel@cetuc.puc-rio.br>
- Fixed xine.m4 installation from Andrew Meredith <andrew@anvil.org>
* Fri Sep 21 2001 Matthias Dahl <matthew2k@web.de>
- added two missing files (xine-config man page and xine.m4)
* Sun Sep 16 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- Merge patch from Jos�Carlos Monteiro <jcm@netcabo.pt>:
  - Filelist and other minor updates,
  - Fixed some SuSE compatibility issues,
  - Added Portuguese summary.
* Sun Sep 16 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- Add missing files.
* Sun Aug 19 2001 Matthias Dahl <matthew2k@web.de>
- The usual update to the filelist :)
- temporarily removed mpg123 decoder plugin from filelist cause it is not
  built with the recent CVS tree
* Thu Jul 26 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- Made oss, aa, xv, esd, w32dll, documentation as separate packages.
* Thu Jul 26 2001 Matthias Dahl <matthew2k@web.de>
- added seperate arts package and one missing demuxer plugin to filelist
* Wed Jul 18 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- list all plugins to avoid *strange* inclusion ;-).
* Sun Jun 10 2001 Matthias Dahl <matthew2k@web.de>
- updated filelist
- re-activated execution of /sbin/ldconfig as post install script
* Thu Mar 28 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- add korean summary, patch from Louis JANG <louis@ns.mizi.com>
* Thu Jan 11 2001 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- patch from Sung-Hyun Nam <namsh@lgic.co.kr> applied.
* Fri Oct 17 2000 Daniel Caujolle-Bert <f1rmb@users.sourceforge.net>
- first spec file.
