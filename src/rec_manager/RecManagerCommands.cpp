#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <regex>

#include "RecManagerIf.h"
#include "CommandTables.h"
#include "CommandServerLog.h"
#include "Utils.h"


static void addReserve_currentEvent (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 1) {
		_COM_SVR_PRINT ("invalid arguments. (usage: ce {group_id})\n");
		return ;
	}

	std::regex regex ("^[0-9]+$");
	if (!std::regex_match (argv[0], regex)) {
		_COM_SVR_PRINT ("invalid arguments. (group_id)\n");
		return;
	}
	uint8_t group_id = atoi(argv[0]);

	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqAddReserve_currentEvent (group_id);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

// yyyyMMddHHmmss to epoch time 
static time_t dateString2epoch (char *pszDate)
{
	if (!pszDate) {
		return 0;
	}

	std::regex regex("[0-9]{14}");
	if (!std::regex_match (pszDate, regex)) {
		return 0;
	}

	struct tm t;
	int off = 0;

	// year
	char sz_year [4+1] = {0};
	strncpy (sz_year, pszDate, 4);
	int y = atoi (sz_year);
	t.tm_year = y -1900;

	off += 4;

	// month
	char sz_month [2+1] = {0};
	strncpy (sz_month, pszDate + off, 2);
	int mon = atoi (sz_month);
	t.tm_mon = mon -1;
	
	off += 2;

	// day
	char sz_day [2+1] = {0};
	strncpy (sz_day, pszDate + off, 2);
	int d = atoi (sz_day);
	t.tm_mday = d;

	off += 2;

	// hour
	char sz_hour [2+1] = {0};
	strncpy (sz_hour, pszDate + off, 2);
	int h = atoi (sz_hour);
	t.tm_hour = h;

	off += 2;

	// min
	char sz_min [2+1] = {0};
	strncpy (sz_min, pszDate + off, 2);
	int min = atoi (sz_min);
	t.tm_min = min;

	off += 2;

	// sec
	char sz_sec [2+1] = {0};
	strncpy (sz_sec, pszDate + off, 2);
	int s = atoi (sz_sec);
	t.tm_sec = s;

	return mktime (&t);
}

static void addReserve_manual (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 6) {
		_COM_SVR_PRINT ("invalid arguments.\n");
		return;
	}

	uint16_t _tsid = 0;
	std::regex regex_tsid ("^[0-9]+$");
	if (!std::regex_match (argv[0], regex_tsid)) {
		std::regex regex_tsid ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[0], regex_tsid)) {
			_COM_SVR_PRINT ("invalid arguments. (tsid)\n");
			return;
		} else {
			_tsid = strtol (argv[0], NULL, 16);
		}
	} else {
		_tsid = atoi (argv[0]);
	}

	uint16_t _org_nid = 0;
	std::regex regex_org_nid ("^[0-9]+$");
	if (!std::regex_match (argv[1], regex_org_nid)) {
		std::regex regex_org_nid ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[1], regex_org_nid)) {
			_COM_SVR_PRINT ("invalid arguments. (org_nid)\n");
			return;
		} else {
			_org_nid = strtol (argv[1], NULL, 16);
		}
	} else {
		_org_nid = atoi (argv[1]);
	}

	uint16_t _svc_id = 0;
	std::regex regex_svc_id("^[0-9]+$");
	if (!std::regex_match (argv[2], regex_svc_id)) {
		std::regex regex_svc_id ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[2], regex_svc_id)) {
			_COM_SVR_PRINT ("invalid arguments. (svc_id)\n");
			return;
		} else {
			_svc_id = strtol (argv[2], NULL, 16);
		}
	} else {
		_svc_id = atoi (argv[2]);
	}

	std::regex regex_start_time("^[0-9]{14}$");
	if (!std::regex_match (argv[3], regex_start_time)) {
		_COM_SVR_PRINT ("invalid arguments. (start_time)\n");
		return;
	}

	std::regex regex_end_time("^[0-9]{14}$");
	if (!std::regex_match (argv[4], regex_end_time)) {
		_COM_SVR_PRINT ("invalid arguments. (end_time)\n");
		return;
	}

	time_t _start = dateString2epoch (argv[3]);
	if (_start == 0) {
		_COM_SVR_PRINT ("invalid arguments. (start_time:dateString2epoch)\n");
		return;
	}

	time_t _end = dateString2epoch (argv[4]);
	if (_end == 0) {
		_COM_SVR_PRINT ("invalid arguments. (end_time:dateString2epoch)\n");
		return;
	}

	if (_start >= _end) {
		_COM_SVR_PRINT ("invalid arguments. (start_time >= end_time)\n");
		return;
	}

	CEtime s (_start);
	CEtime e (_end);

	std::regex regex_repeat("^[0-2]$");
	if (!std::regex_match (argv[5], regex_repeat)) {
		_COM_SVR_PRINT ("invalid arguments. (repeat)\n");
		return;
	}
	EN_RESERVE_REPEATABILITY r = (EN_RESERVE_REPEATABILITY) atoi (argv[5]);


	CRecManagerIf::ADD_RESERVE_PARAM_t _param;
	_param.transport_stream_id = _tsid ;
	_param.original_network_id = _org_nid;
	_param.service_id = _svc_id;
	_param.start_time = s;
	_param.end_time = e;
	_param.repeatablity = r;

	_param.dump();


	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqAddReserve_manual (&_param);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

static void addReserve_event (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 5) {
		_COM_SVR_PRINT ("invalid arguments.\n");
		return;
	}

	uint16_t _tsid = 0;
	std::regex regex_tsid ("^[0-9]+$");
	if (!std::regex_match (argv[0], regex_tsid)) {
		std::regex regex_tsid ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[0], regex_tsid)) {
			_COM_SVR_PRINT ("invalid arguments. (tsid)\n");
			return;
		} else {
			_tsid = strtol (argv[0], NULL, 16);
		}
	} else {
		_tsid = atoi (argv[0]);
	}

	uint16_t _org_nid = 0;
	std::regex regex_org_nid ("^[0-9]+$");
	if (!std::regex_match (argv[1], regex_org_nid)) {
		std::regex regex_org_nid ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[1], regex_org_nid)) {
			_COM_SVR_PRINT ("invalid arguments. (org_nid)\n");
			return;
		} else {
			_org_nid = strtol (argv[1], NULL, 16);
		}
	} else {
		_org_nid = atoi (argv[1]);
	}

	uint16_t _svc_id = 0;
	std::regex regex_svc_id("^[0-9]+$");
	if (!std::regex_match (argv[2], regex_svc_id)) {
		std::regex regex_svc_id ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[2], regex_svc_id)) {
			_COM_SVR_PRINT ("invalid arguments. (svc_id)\n");
			return;
		} else {
			_svc_id = strtol (argv[2], NULL, 16);
		}
	} else {
		_svc_id = atoi (argv[2]);
	}

	uint16_t _evt_id = 0;
	std::regex regex_evt_id("^[0-9]+$");
	if (!std::regex_match (argv[3], regex_evt_id)) {
		std::regex regex_evt_id ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[3], regex_evt_id)) {
			_COM_SVR_PRINT ("invalid arguments. (evt_id)\n");
			return;
		} else {
			_evt_id = strtol (argv[3], NULL, 16);
		}
	} else {
		_evt_id = atoi (argv[3]);
	}


	std::regex regex_repeat("^[0-1]$");
	if (!std::regex_match (argv[4], regex_repeat)) {
		_COM_SVR_PRINT ("invalid arguments. (repeat)\n");
		return;
	}
	int rpt = atoi (argv[4]);
	EN_RESERVE_REPEATABILITY r = rpt == 0 ? EN_RESERVE_REPEATABILITY__NONE: EN_RESERVE_REPEATABILITY__AUTO;


	CRecManagerIf::ADD_RESERVE_PARAM_t _param;
	_param.transport_stream_id = _tsid ;
	_param.original_network_id = _org_nid;
	_param.service_id = _svc_id;
	_param.event_id = _evt_id;
	_param.repeatablity = r;

	_param.dump();


	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqAddReserve_event (&_param);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

static void addReserve_eventHelper (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 2) {
		_COM_SVR_PRINT ("invalid arguments.\n");
		return;
	}

	std::regex regex_idx ("^[0-9]+$");
	if (!std::regex_match (argv[0], regex_idx)) {
		_COM_SVR_PRINT ("invalid arguments. (index)\n");
		return;
	}
	uint16_t _idx = atoi (argv[0]);


	std::regex regex_repeat("^[0-1]$");
	if (!std::regex_match (argv[1], regex_repeat)) {
		_COM_SVR_PRINT ("invalid arguments. (repeat)\n");
		return;
	}
	int rpt = atoi (argv[1]);
	EN_RESERVE_REPEATABILITY r = rpt == 0 ? EN_RESERVE_REPEATABILITY__NONE: EN_RESERVE_REPEATABILITY__AUTO;


	CRecManagerIf::ADD_RESERVE_HELPER_PARAM_t _param;
	_param.index = _idx ;
	_param.repeatablity = r;


	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqAddReserve_eventHelper (&_param);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

static void removeReserve (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 5) {
		_COM_SVR_PRINT ("invalid arguments.\n");
		return ;
	}

	uint16_t _tsid = 0;
	std::regex regex_tsid ("^[0-9]+$");
	if (!std::regex_match (argv[0], regex_tsid)) {
		std::regex regex_tsid ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[0], regex_tsid)) {
			_COM_SVR_PRINT ("invalid arguments. (tsid)\n");
			return;
		} else {
			_tsid = strtol (argv[0], NULL, 16);
		}
	} else {
		_tsid = atoi (argv[0]);
	}

	uint16_t _org_nid = 0;
	std::regex regex_org_nid ("^[0-9]+$");
	if (!std::regex_match (argv[1], regex_org_nid)) {
		std::regex regex_org_nid ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[1], regex_org_nid)) {
			_COM_SVR_PRINT ("invalid arguments. (org_nid)\n");
			return;
		} else {
			_org_nid = strtol (argv[1], NULL, 16);
		}
	} else {
		_org_nid = atoi (argv[1]);
	}

	uint16_t _svc_id = 0;
	std::regex regex_svc_id("^[0-9]+$");
	if (!std::regex_match (argv[2], regex_svc_id)) {
		std::regex regex_svc_id ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[2], regex_svc_id)) {
			_COM_SVR_PRINT ("invalid arguments. (svc_id)\n");
			return;
		} else {
			_svc_id = strtol (argv[2], NULL, 16);
		}
	} else {
		_svc_id = atoi (argv[2]);
	}

	uint16_t _evt_id = 0;
	std::regex regex_evt_id("^[0-9]+$");
	if (!std::regex_match (argv[3], regex_evt_id)) {
		std::regex regex_evt_id ("^0x([0-9]|[a-f]|[A-F])+$");
		if (!std::regex_match (argv[3], regex_evt_id)) {
			_COM_SVR_PRINT ("invalid arguments. (evt_id)\n");
			return;
		} else {
			_evt_id = strtol (argv[3], NULL, 16);
		}
	} else {
		_evt_id = atoi (argv[3]);
	}

	std::regex regex_rep ("^[0-1]$");
	if (!std::regex_match (argv[4], regex_rep)) {
		_COM_SVR_PRINT ("invalid arguments. (consider repeatability)\n");
		return;
	}
	bool isConsiderRepeatability = (atoi(argv[4]) == 0 ? false : true);


	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf::REMOVE_RESERVE_PARAM_t param; 
	param.arg.key.transport_stream_id = _tsid;
	param.arg.key.original_network_id = _org_nid;
	param.arg.key.service_id = _svc_id;
	param.arg.key.event_id = _evt_id;
	param.isConsiderRepeatability = isConsiderRepeatability;
	param.isApplyResult = true;

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqRemoveReserve (&param);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

static void removeReserve_byIndex (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 2) {
		_COM_SVR_PRINT ("invalid arguments.\n");
		return ;
	}

	std::regex regex_idx ("^[0-9]+$");
	if (!std::regex_match (argv[0], regex_idx)) {
		_COM_SVR_PRINT ("invalid arguments. (index)\n");
		return;
	}

	std::regex regex_rep ("^[0-1]$");
	if (!std::regex_match (argv[1], regex_rep)) {
		_COM_SVR_PRINT ("invalid arguments. (consider repeatability)\n");
		return;
	}

	int index = atoi(argv[0]);
	bool isConsiderRepeatability = (atoi(argv[1]) == 0 ? false : true);

	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf::REMOVE_RESERVE_PARAM_t param; 
	param.arg.index = index;
	param.isConsiderRepeatability = isConsiderRepeatability;
	param.isApplyResult = true;

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqRemoveReserve_byIndex (&param);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

static void get_reserves (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 0) {
		_COM_SVR_PRINT ("ignore arguments.\n");
	}

	CRecManagerIf::RESERVE_t reserves[50] = {0}; // max 50
	CRecManagerIf::GET_RESERVES_PARAM_t param = {reserves, 50};
	CRecManagerIf _if (pBase->getExternalIf());
	_if.syncGetReserves (&param);

	EN_THM_RSLT enRslt = pBase->getIf()->getSrcInfo()->enRslt;
	if (enRslt == EN_THM_RSLT_SUCCESS) {
		int n =  *(int*)(pBase->getIf()->getSrcInfo()->msg.pMsg);
		_COM_SVR_PRINT ("syncGetReserves success\n");
		_COM_SVR_PRINT ("num of reserves is [%d]\n", n);
		for (int i = 0; i < n; ++ i) {
			_COM_SVR_PRINT (
				"%d: tsid:[0x%04x] org_nid:[0x%04x] svcid:[0x%04x] evtid:[0x%04x]\n",
				i,
				reserves[i].transport_stream_id,
				reserves[i].original_network_id,
				reserves[i].service_id,
				reserves[i].event_id
			);
			_COM_SVR_PRINT (
				"    [%s - %s][%s][%s]\n",
				reserves[i].start_time.toString(),
				reserves[i].end_time.toString(),
				reserves[i].p_service_name->c_str(),
				reserves[i].p_title_name->c_str()
			);
			_COM_SVR_PRINT (
				"    is_event_type:[%d] repeatability:[%d]\n",
				reserves[i].is_event_type,
				reserves[i].repeatability
			);
		}

	} else {
		_COM_SVR_PRINT ("syncGetReserves error\n");
	}
}

static void stop (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 1) {
		_COM_SVR_PRINT ("invalid arguments. (usage: stop {group_id})\n");
		return ;
	}

	std::regex regex ("^[0-9]+$");
	if (!std::regex_match (argv[0], regex)) {
		_COM_SVR_PRINT ("invalid arguments. (group_id)\n");
		return;
	}
	uint8_t group_id = atoi(argv[0]);

	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqStopRecording (group_id);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

static void dump_reserves (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 0) {
		_COM_SVR_PRINT ("ignore arguments.\n");
	}

	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqDumpReserves (0);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

static void dump_results (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 0) {
		_COM_SVR_PRINT ("ignore arguments.\n");
	}

	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqDumpReserves (1);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}

static void dump_recording (int argc, char* argv[], CThreadMgrBase *pBase)
{
	if (argc != 0) {
		_COM_SVR_PRINT ("ignore arguments.\n");
	}

	uint32_t opt = pBase->getExternalIf()->getRequestOption ();
	opt |= REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);

	CRecManagerIf _if (pBase->getExternalIf());
	_if.reqDumpReserves (2);

	opt &= ~REQUEST_OPTION__WITHOUT_REPLY;
	pBase->getExternalIf()->setRequestOption (opt);
}



ST_COMMAND_INFO g_recManagerCommands [] = { // extern
	{
		"ce",
		"add reserve - CurrentEvent (usage: ce {group_id})",
		addReserve_currentEvent,
		NULL,
	},
	{
		"e",
		"add reserve - event\n\
                                (usage: e {tsid} {org_nid} {svcid} {evtid} {repeat})\n\
                                           - repeat is 0 (none), 1 (auto)",
		addReserve_event,
		NULL,
	},
	{
		"eh",
		"add reserve - event (helper)\n\
                                (usage: eh {index} {repeat})\n\
                                           - repeat is 0 (none), 1 (auto)",
		addReserve_eventHelper,
		NULL,
	},
	{
		"m",
		"add reserve - Manual\n\
                                (usage: m {tsid} {org_nid} {svcid} {start_time} {end_time} {repeat})\n\
                                           - start_time, end_time format is \"yyyyMMddHHmmss\"\n\
                                           - repeat is 0 (none), 1 (daily), 2 (weekly)",
		addReserve_manual,
		NULL,
	},
	{
		"r",
		"remove reserve (usage: r {tsid} {org_nid} {svcid} {evtid} {consider repeatability})\n\
                                           - consider repeatability is 0 (false), 1 (true)",
		removeReserve,
		NULL,
	},
	{
		"rx",
		"remove reserve by index (usage: rx {index} {consider repeatability})\n\
                                           - consider repeatability is 0 (false), 1 (true)",
		removeReserve_byIndex,
		NULL,
	},
	{
		"g",
		"get reserves",
		get_reserves,
		NULL,
	},
	{
		"stop",
		"force stop recording (usage: stop {group_id})",
		stop,
		NULL,
	},
	{
		"dr",
		"dump reserves",
		dump_reserves,
		NULL,
	},
	{
		"drl",
		"dump results",
		dump_results,
		NULL,
	},
	{
		"drec",
		"dump recording",
		dump_recording,
		NULL,
	},
	//-- term --//
	{
		NULL,
		NULL,
		NULL,
		NULL,
	},
};

