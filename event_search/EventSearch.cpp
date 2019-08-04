#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "EventSearch.h"
#include "modules.h"

#include "Settings.h"


CEventSearch::CEventSearch (char *pszName, uint8_t nQueNum)
	:CThreadMgrBase (pszName, nQueNum)
{
	mSeqs [EN_SEQ_EVENT_SEARCH__MODULE_UP] =
		{(PFN_SEQ_BASE)&CEventSearch::onReq_moduleUp,                              (char*)"onReq_moduleUp"};
	mSeqs [EN_SEQ_EVENT_SEARCH__MODULE_DOWN] =
		{(PFN_SEQ_BASE)&CEventSearch::onReq_moduleDown,                            (char*)"onReq_moduleDown"};
	mSeqs [EN_SEQ_EVENT_SEARCH__ADD_REC_RESERVE__KEYWORD_SEARCH] =
		{(PFN_SEQ_BASE)&CEventSearch::onReq_addRecReserve_keywordSearch,           (char*)"onReq_addRecReserve_keywordSearch"};
	setSeqs (mSeqs, EN_SEQ_EVENT_SEARCH__NUM);


	m_event_name_keywords.clear();
}

CEventSearch::~CEventSearch (void)
{
}


void CEventSearch::onReq_moduleUp (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_REG_TUNE_COMP_NOTIFY,
		SECTID_WAIT_REG_TUNE_COMP_NOTIFY,
		SECTID_END_SUCCESS,
		SECTID_END_ERROR,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

//
// do nothing
//

	pIf->reply (EN_THM_RSLT_SUCCESS);

	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CEventSearch::onReq_moduleDown (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

//
// do nothing
//

	pIf->reply (EN_THM_RSLT_SUCCESS);

	sectId = THM_SECT_ID_INIT;
	enAct = EN_THM_ACT_DONE;
	pIf->setSectId (sectId, enAct);
}

void CEventSearch::onReq_addRecReserve_keywordSearch (CThreadMgrIf *pIf)
{
	uint8_t sectId;
	EN_THM_ACT enAct;
	enum {
		SECTID_ENTRY = THM_SECT_ID_INIT,
		SECTID_REQ_GET_EVENTS,
		SECTID_WAIT_GET_EVENTS,
		SECTID_REQ_ADD_RESERVE,
		SECTID_WAIT_ADD_RESERVE,
		SECTID_CHECK_LOOP,
		SECTID_END,
	};

	sectId = pIf->getSectId();
	_UTL_LOG_D ("(%s) sectId %d\n", pIf->getSeqName(), sectId);

	EN_THM_RSLT enRslt = EN_THM_RSLT_SUCCESS;
	static std::vector<std::string>::const_iterator s_iter ;
	static CEventScheduleManagerIf::EVENT_t s_events [30];
	static int s_events_idx = 0;
	static int s_get_events_num = 0;


	switch (sectId) {
	case SECTID_ENTRY:

		loadEventNameKeywords ();
		dumpEventNameKeywords ();


		s_iter = m_event_name_keywords.begin();
		if (s_iter == m_event_name_keywords.end()) {
			sectId = SECTID_END;
			enAct = EN_THM_ACT_CONTINUE;
		}

		sectId = SECTID_REQ_GET_EVENTS;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_REQ_GET_EVENTS: {

		memset (s_events, 0x00, sizeof(s_events));
		s_events_idx = 0;
		s_get_events_num = 0;


		CEventScheduleManagerIf::REQ_EVENT_PARAM_t _param;
		_param.arg.p_keyword = s_iter->c_str();
		_param.p_out_event = s_events;
		_param.array_max_num = 30;

		CEventScheduleManagerIf _if(getExternalIf());
		_if.reqGetEvent_keyword (&_param);


		sectId = SECTID_WAIT_GET_EVENTS;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_GET_EVENTS:
		enRslt = pIf->getSrcInfo()->enRslt;
		if (enRslt == EN_THM_RSLT_SUCCESS) {
			s_get_events_num = *(int*)(pIf->getSrcInfo()->msg.pMsg);
			if (s_get_events_num > 0) {
				if (s_get_events_num > 30) {
					_UTL_LOG_W ("trancate s_get_events_num");
					s_get_events_num = 30;
				}

				sectId = SECTID_REQ_ADD_RESERVE;
				enAct = EN_THM_ACT_CONTINUE;

			} else {
				sectId = SECTID_CHECK_LOOP;
				enAct = EN_THM_ACT_CONTINUE;
			}

		} else {
			s_get_events_num = 0;
			sectId = SECTID_CHECK_LOOP;
			enAct = EN_THM_ACT_CONTINUE;
		}

		break;

	case SECTID_REQ_ADD_RESERVE: {

		CRecManagerIf::ADD_RESERVE_PARAM_t _param;
		_param.transport_stream_id = s_events [s_events_idx].transport_stream_id;
		_param.original_network_id = s_events [s_events_idx].original_network_id;
		_param.service_id = s_events [s_events_idx].service_id;
		_param.event_id = s_events [s_events_idx].event_id;
		_param.repeatablity = EN_RESERVE_REPEATABILITY__NONE;

		CRecManagerIf _if(getExternalIf());
		_if.reqAddReserve_event (&_param);

		sectId = SECTID_WAIT_ADD_RESERVE;
		enAct = EN_THM_ACT_WAIT;

		}
		break;

	case SECTID_WAIT_ADD_RESERVE:
//TODO 暫定 結果見ない

		sectId = SECTID_CHECK_LOOP;
		enAct = EN_THM_ACT_CONTINUE;
		break;

	case SECTID_CHECK_LOOP:

		++ s_events_idx;

		if (s_events_idx < s_get_events_num) {
			sectId = SECTID_REQ_ADD_RESERVE;
			enAct = EN_THM_ACT_CONTINUE;

		} else {
			s_events_idx = 0;
			++ s_iter;

			if (s_iter == m_event_name_keywords.end()) {
				sectId = SECTID_END;
				enAct = EN_THM_ACT_CONTINUE;

			} else {
				sectId = SECTID_REQ_GET_EVENTS;
				enAct = EN_THM_ACT_CONTINUE;
			}
		}

		break;

	case SECTID_END:

		pIf->reply (EN_THM_RSLT_SUCCESS);

		sectId = THM_SECT_ID_INIT;
		enAct = EN_THM_ACT_DONE;
		break;

	default:
		break;
	}

	pIf->setSectId (sectId, enAct);
}


void CEventSearch::dumpEventNameKeywords (void) const
{
	_UTL_LOG_I ("----- event name keywords -----");
	std::vector<std::string>::const_iterator iter = m_event_name_keywords.begin();
	for (; iter != m_event_name_keywords.end(); ++ iter) {
		_UTL_LOG_I ("  [%s]", iter->c_str());
	}
}



//--------------------------------------------------------------------------------

void CEventSearch::saveEventNameKeywords (void)
{
	std::stringstream ss;
	{
		cereal::JSONOutputArchive out_archive (ss);
		out_archive (CEREAL_NVP(m_event_name_keywords));
	}

	std::string *p_path = CSettings::getInstance()->getParams()->getEventNameKeywordsJsonPath();
	std::ofstream ofs (p_path->c_str(), std::ios::out);
	ofs << ss.str();

	ofs.close();
	ss.clear();
}

void CEventSearch::loadEventNameKeywords (void)
{
	std::string *p_path = CSettings::getInstance()->getParams()->getEventNameKeywordsJsonPath();
	std::ifstream ifs (p_path->c_str(), std::ios::in);
	if (!ifs.is_open()) {
		_UTL_LOG_I ("event_name_keywords.json is not found.");
		return;
	}

	std::stringstream ss;
	ss << ifs.rdbuf();

	cereal::JSONInputArchive in_archive (ss);
	in_archive (CEREAL_NVP(m_event_name_keywords));

	ifs.close();
	ss.clear();
}