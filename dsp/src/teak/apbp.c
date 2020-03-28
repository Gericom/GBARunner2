#include "teak.h"
#include "apbp.h"

void apbp_sendData(u16 id, u16 data)
{
    switch(id)
    {
	    case 0:
            while(REG_APBP_STAT & APBP_STAT_REP0_UNREAD);
            REG_APBP_REP0 = data;
            break;
        case 1:
            while(REG_APBP_STAT & APBP_STAT_REP1_UNREAD);
            REG_APBP_REP1 = data;
            break;
        case 2:
            while(REG_APBP_STAT & APBP_STAT_REP2_UNREAD);
            REG_APBP_REP2 = data;
            break;
    }
}

u16 apbp_receiveData(u16 id)
{
    switch(id)
    {
	    case 0:
            while(!(REG_APBP_STAT & APBP_STAT_CMD0_NEW));
            return REG_APBP_CMD0;
        case 1:
            while(!(REG_APBP_STAT & APBP_STAT_CMD1_NEW));
            return REG_APBP_CMD1;
        case 2:
            while(!(REG_APBP_STAT & APBP_STAT_CMD2_NEW));
            return REG_APBP_CMD2;
    }
    return 0;
}