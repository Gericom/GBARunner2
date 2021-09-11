#include "teak/teak.h"
#include "gbaAudio.h"
#include "ipc.h"

ipc_fifo_t gIpcFifo __attribute__((section(".data.ipc")));

void ipc_init(void)
{
    gIpcFifo.readPtr = 0;
    gIpcFifo.writePtr = 0;
    REG_APBP_CONTROL = APBP_CONTROL_IRQ_CMD0_DISABLE | APBP_CONTROL_IRQ_CMD2_DISABLE;
    apbp_clearSemaphore(0xFFFF);
    cpu_disableIrqs();
    REG_ICU_IRQ_MODE |= ICU_IRQ_MASK_APBP;
    REG_ICU_IRQ_POLARITY &= ~ICU_IRQ_MASK_APBP;
    REG_ICU_IRQ_DISABLE &= ~ICU_IRQ_MASK_APBP;
    REG_ICU_IRQ_ACK = ICU_IRQ_MASK_APBP;
    REG_ICU_IRQ_INT0 = ICU_IRQ_MASK_APBP;
    cpu_enableInt0();
    cpu_enableIrqs();
    apbp_sendData(0, 1);
    apbp_sendData(1, 1);
    apbp_sendData(2, 1);
}

static u16 getCmdCount(void)
{
    int count = gIpcFifo.writePtr - gIpcFifo.readPtr;
    if(count < 0)
        count += IPC_FIFO_LENGTH;
    return count;
}

void onIpcCommandReceived(void)
{
    if(getCmdCount() == 0)
    {
        apbp_receiveData(1);
        return;
    }

    const ipc_cmd_t* ipcCmd = &gIpcFifo.fifo[gIpcFifo.readPtr];
    u32 cmd = ipcCmd->cmdLo | (ipcCmd->cmdHi << 16);
    u32 arg = ipcCmd->argLo | (ipcCmd->argHi << 16);
    gbaa_handleCommand(cmd, arg);

    gIpcFifo.readPtr = (gIpcFifo.readPtr + 1) % IPC_FIFO_LENGTH;
    
    apbp_receiveData(1);
    apbp_sendData(1, 0);
}