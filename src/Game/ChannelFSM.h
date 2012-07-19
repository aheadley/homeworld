#ifndef ___ChannelFSM_H
#define ___ChannelFSM_H

bool cCreateChannel(wchar_t *name, wchar_t *desc);
bool cJoinChannelRequest(wchar_t *name, wchar_t *desc);
void cNotifyChatConnected(void);
void cResetFSM(void);
bool cJoinADefaultRoom(void);
void cNotifyChatBadResponse(void);

#endif

