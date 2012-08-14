#include "utils.h"

QString cacheSizeToString(int size)
{
    return QString("%1 Mb").arg(size / CACHE_SCALE, 0, 'f', 2);
}
