#ifndef __SMSGTYPESFACT_H__
#define __SMSGTYPESFACT_H__

namespace WONMsg
{
	enum SMsgTypeFact
	{
		FactAutoStartRequest			= 1,
		FactRunProcess					= 2,
		SmallFactStatusReply			= 3,
		FactGetProcessConfig			= 4,
		FactGetAllProcesses				= 5,

		FactStopProcess					= 6,
		FactGetProcessList				= 7,
		FactProcessListReply			= 8,
		FactProcessConfigReply			= 9,
		FactGetProcessPorts				= 10,

		FactGetAllProcessesReply		= 11,
		FactKillProcess					= 12,

		SmallFactStartProcess			= 13,
		SmallFactStartProcessUnicode	= 14,

		FactSaveConfigFile				= 15,
		FactPullConfigFile				= 16,

		FactGetFreeDiskSpace			= 17,
		FactGetFreeDiskSpaceReply		= 18,

		FactGetUsage					= 19,
		FactGetUsageReply				= 20,

		FactStreamFile					= 21,
		FactGetFileCRC					= 22,
		FactGetFileCRCReply				= 23

	};
};

#endif