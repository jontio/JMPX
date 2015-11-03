#include "nowplaying_mac.h"

NowPlaying::NowPlaying(QObject *parent) : QObject(parent)
{
    rt_title.clear();
}

