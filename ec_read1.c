#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
static int fd;
bool port_dev_init(void)
{
    fd = open("/dev/port", O_RDWR);
    if (fd < 0) {
        return false;
    }  
    return true;
}
void port_dev_exit(void)
{
    close(fd);
}
uint8_t ioread8(uint16_t port)
{
    uint8_t buf[2] = { 0 };
    lseek(fd, port, SEEK_SET);
    read(fd, buf, 1);
    return buf[0];
}
void iowrite8(uint16_t port, uint8_t data)
{
    uint8_t buf[2] = { 0 };
    lseek(fd, port, SEEK_SET);
    buf[0] = data;
    write(fd, buf, 1);
}


#define EC_TIME_OUT		    10000
#define EC_S_IBF            0x02    // Input buffer is full/empty
#define EC_S_OBF            0x01    // Output buffer is full/empty
#define STATE_SUCCESS       0x00
#define STATE_ERROR         0x01
#define EC_D_PORT           0x62
#define EC_C_PORT           0x66
#define EC_C_READ_MEM       0x80    // Read the EC memory
#define EC_C_WRITE_MEM      0x81    // Write the EC memory

uint8_t WaitECIbe (
  uint8_t              CommandState
  )
{
  uint8_t                 KbdCmdState = 0;
  uint8_t                 Index;

  for (Index = 0; Index < EC_TIME_OUT; Index++) {
    KbdCmdState = (uint8_t)ioread8 (CommandState);
    if (!(KbdCmdState & EC_S_IBF)) {
      return STATE_SUCCESS;
    } else{
      usleep(15); 
    }
  }
  return STATE_ERROR;
}

uint8_t BldWaitKbcObf (
  uint8_t              CommandState
)
{
  uint8_t                 KbdCmdState = 0;
  uint8_t                 Index;

  for (Index = 0; Index < EC_TIME_OUT; Index++) {
    KbdCmdState = (uint8_t)ioread8 (CommandState);
    if (KbdCmdState & EC_S_OBF) {
      return STATE_SUCCESS;
    } else{
      usleep(15);
    }
  }

  return STATE_ERROR;
}

void SendDataToEc (
  uint8_t                               Port,
  uint8_t                               Data
  )
{
  WaitECIbe (Port);                   // Wait Input Buffer Empty
  iowrite8 (Port - 4, Data);
  WaitECIbe (Port);                   // Wait Input Buffer Empty
}

void SendCmdToEc (
  uint8_t                               Port,
  uint8_t                               Cmd
  )
{

  WaitECIbe (Port);                   // Wait Input Buffer Empty
  iowrite8 (Port, Cmd);
  WaitECIbe (Port);                   // Wait Input Buffer Empty

}




uint8_t GetDataFromEc (
  uint8_t                               Port
  )
{
  uint8_t         Data;

  WaitECIbe (Port);                   // Wait Input Buffer Empty
  BldWaitKbcObf (Port);                   // Wait Output Buffer Full
  Data = ioread8 (Port - 4);

  return Data;
}
  
void ECWrite(
  uint8_t    Index,
  uint8_t    Value
  )
{
  //
  // Write Command
  //
  SendCmdToEc (EC_C_PORT, EC_C_WRITE_MEM);

  //
  // send ECRAM offset to DATA port
  //
  SendDataToEc (EC_C_PORT, Index);

  //
  // Write DATA to EC ram
  //
  SendDataToEc (EC_C_PORT, Value);
}


uint8_t ECRead(
  uint8_t    Index
  )
{
  SendCmdToEc (EC_C_PORT, EC_C_READ_MEM);

  //
  // send ECRAM offset to DATA port
  //
  SendDataToEc (EC_C_PORT, Index);

  //
  // get DATA from EC
  //
  return GetDataFromEc (EC_C_PORT);
}
//lable001
void func()
{
	uint8_t value,pdate[2];
	value = 1;
	pdate[0] = 2;
	return;
}
//lable001
int main(int argc, char **argv)
{
    uint8_t value,pdate[2];
    uint8_t index;
    if (!port_dev_init()) {
        printf("Init error!\n");
        return 0;
    }
	printf("first read EC\n");
	pdate[0]=ECRead(0xAA);
	pdate[1]=ECRead(0xAB);
	printf("EC read data %d from index 0xAA,\n",pdate[0]);
	printf("EC read data %d from index 0xAB,\n",pdate[1]);
	printf("write EC\n");
	pdate[0]=0x55;
	pdate[1]=0x66;
	ECWrite(0xAA,pdate[0]);
	ECWrite(0xAB,pdate[1]);
	printf("EC write data %d to index 0xAA,\n",pdate[0]);
	printf("EC write data %d to index 0xAB,\n",pdate[1]);
	printf("verify\n");
	pdate[0]=ECRead(0xAA);
	pdate[1]=ECRead(0xAB);
	printf("EC read data %d from index 0xAA,\n",pdate[0]);
	printf("EC read data %d from index 0xAB,\n",pdate[1]);
    port_dev_exit();
    return 0;
}