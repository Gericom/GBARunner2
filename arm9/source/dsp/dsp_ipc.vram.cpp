#include "vram.h"
#include "dsp.h"
#include "dsp_fifo.h"
#include "dsp_ipc.h"

struct dsp_ipc_cmd_t
{
    u32 cmd;
    u32 arg;
};

extern "C" bool dsp_sendIpcCommand(u32 cmd, u32 arg)
{
    struct 
    {
        u16 readPtr;
        u16 writePtr;
    } header;    
    dsp_fifoReadData(0x500, (u16*)&header, 2);
    int writeLength = header.readPtr - header.writePtr - 1;
	if (writeLength < 0)
		writeLength += 64;

    if(writeLength == 0)
        return false;
    
    dsp_ipc_cmd_t ipcCmd = {cmd, arg};
    dsp_fifoWriteData((const u16*)&ipcCmd, 0x502 + header.writePtr * 4, 4);

    header.writePtr = (header.writePtr + 1) % 64;
    dsp_fifoWriteData(&header.writePtr, 0x501, 1);

    dsp_spinWait();
    while(!(REG_DSP_PSTS & DSP_PSTS_WR_FIFO_EMPTY) || (REG_DSP_PSTS & DSP_PSTS_WR_XFER_BUSY));

    dsp_sendData(1, 0);
    dsp_receiveData(1);

    return true;
}