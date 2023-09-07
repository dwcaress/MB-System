
#include "mxdebug.hpp"

#ifdef __cplusplus
pthread_mutex_t MXDebug::m_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MXDebug::m_write_mutex = PTHREAD_MUTEX_INITIALIZER;
mlist_t *MXDebug::m_mlist = mlist_new();
bool MXDebug::m_auto_newline = false;
#endif
