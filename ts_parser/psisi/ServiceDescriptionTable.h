#ifndef _SERVICE_DESCRIPTION_TABLE_H_
#define _SERVICE_DESCRIPTION_TABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <vector>

#include "Defs.h"
#include "TsCommon.h"
#include "SectionParser.h"
#include "DescriptorDefs.h"


#define SDT_FIX_LEN			(3) // original_etwork_id     16  uimsbf
								// reserved_future_use_2   8  bslbf

#define SDT_SERVICE_FIX_LEN	(5) // service_id                  16  uimsbf
								// reserved_future_use          3  bslbf
								// EIT_user_defined_flags       3  bslbf
								// EIT_schedule_flag            1  bslbf
								// EIT_present_following_flag   1  bslbf
								// running_status               3  uimsbf
								// free_CA_mode                 1  bslbf
								// descriptors_loop_length     12  uimsbf


class CServiceDescriptionTable : public CSectionParser
{
public:
	class CTable {
	public:
		class CService {
		public:
			CService (void)
				:service_id (0)
				,reserved_future_use (0)
				,EIT_user_defined_flags (0)
				,EIT_schedule_flag (0)
				,EIT_present_following_flag (0)
				,running_status (0)
				,free_CA_mode (0)
				,descriptors_loop_length (0)
			{
				descriptors.clear();
			}
			virtual ~CService (void) {}

			uint16_t service_id;
			uint8_t reserved_future_use;
			uint8_t EIT_user_defined_flags;
			uint8_t EIT_schedule_flag;
			uint8_t EIT_present_following_flag;
			uint8_t running_status;
			uint8_t free_CA_mode;
			uint16_t descriptors_loop_length;
			std::vector <CDescriptor> descriptors;
		};

	public:
		CTable (void)
			:original_network_id (0)
			,reserved_future_use_2 (0)
		{
			services.clear();
		}
		virtual ~CTable (void) {}

		ST_SECTION_HEADER header;
		uint16_t original_network_id;
		uint8_t reserved_future_use_2;
		std::vector <CService> services;
	};
public:
	CServiceDescriptionTable (void);
	virtual ~CServiceDescriptionTable (void);

	void onSectionCompleted (const CSectionInfo *pCompSection) override;

	void dumpTables (void) const;
	void dumpTable (const CTable* pTable) const;
	void clear (void);

private:
	bool parse (const CSectionInfo *pCompSection, CTable* pOutTable);
	void releaseTables (void);

	std::vector <CTable*> mTables;

};

#endif