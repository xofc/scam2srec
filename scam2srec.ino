// Scam2Srec.ino
// adapted from https://github.com/Seeed-Studio/Grove_Serial_Camera_Kit/tree/master/SerialCameral_DemoCode_CJ_OV528_SoftSer
// xof april 2015 : send S-records through Serial instead of writing SD file
//  File SerialCamera_DemoCode_CJ-OV528.ino
//  8/8/2013 Jack Shao
//  Demo code for using seeeduino or Arduino board to cature jpg format
//  picture from seeed serial camera and save it into sd card. Push the
//  button to take the a picture .

//  For more details about the product please check http://www.seeedstudio.com/depot/

#include <SoftwareSerial.h>

#define PIC_PKT_LEN    32        //data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_SERIAL     softSerial

#define PIC_FMT        PIC_FMT_VGA

SoftwareSerial softSerial(2, 3);  //rx,tx (11-13 is used by sd shield)
				// yellow=2 - white=3

#define CAM_ADDR       0
const byte cameraAddr = (CAM_ADDR << 5);  // addr

unsigned long picTotalLen = 0;            // picture length

/*********************************************************************/
void setup()
	{
	Serial.begin(9600);
	CAM_SERIAL.begin(9600);       //cant be faster than 9600, maybe difference with diff board.
	initialize();
	}
/*********************************************************************/
void loop()
	{
	Serial.println("hit a key to take a picture");
	while (Serial.available() <= 0)
		;
//	if (Serial.available() > 0)
		{
		preCapture();
		Capture();
		GetData();
		while (Serial.available() > 0)
			Serial.read();
		}
	}
/*********************************************************************/
void xbyte(uint8_t x)
	{
	Serial.print(x >> 4,   HEX);
	Serial.print(x & 0x0F, HEX);
	}
void S2_dump(uint32_t add, uint8_t *ucp, int len)
	{
	uint8_t	i, sum, uc;

	Serial.print("S2");
	sum = uc = len+3+1; xbyte(uc);
	uc = (add>>16) & 0xff; xbyte(uc); sum += uc;
	uc = (add>> 8) & 0xff; xbyte(uc); sum += uc;
	uc = (add    ) & 0xff; xbyte(uc); sum += uc;
	for (i = 0; i < len; i++)
		{
		uc = ucp[i]; xbyte(uc); sum += uc;
		}
	sum = ~sum;
	xbyte(sum);
	Serial.println("");
	}
/*********************************************************************/
void clearRxBuf()
	{
	while (CAM_SERIAL.available()) 
		CAM_SERIAL.read(); 
	}
/*********************************************************************/
void sendCmd(char *ucp, int len)
	{
	while (--len >= 0)
		CAM_SERIAL.write(*ucp++); 
	}
/*********************************************************************/
int readBytes(char *dest, int len, unsigned int timeout)
	{
	int read_len = 0;
	unsigned long t = millis();
	while (read_len < len)
		{
		while (CAM_SERIAL.available()<1)
			{
			if ((millis() - t) > timeout)
				{
//Serial.print(read_len); Serial.println(" bytes read (timeout)");
				return read_len;
				}
			}
		*(dest+read_len) = CAM_SERIAL.read();
//		Serial.write(*(dest+read_len));
		read_len++;
		}
// Serial.print(read_len); Serial.println(" bytes read");
	return(read_len);
	}
/*********************************************************************/
char CMD1[] =   {0xaa, 0x0d|cameraAddr, 0x00, 0x00, 0x00, 0x00};
char RESP1a[] = {0xaa, 0x0e|cameraAddr, 0x0d, 0x00, 0x00, 0x00}; //[3] unsure xof
void initialize()
	{
	unsigned char resp[6];
	char cmd[] = {0xaa, 0x0e | cameraAddr, 0x0d, 0x00, 0x00, 0x00};

//	Serial.println("initialize()");
	
	while (1) 
		{
		sendCmd(CMD1, sizeof(CMD1));
		if (readBytes((char *)resp, 6, 1000) != 6)
			{
			continue;
			}
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0) 
			{
			if (readBytes((char *)resp, 6, 500) != 6)
				continue;
			if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0
				&& resp[4] == 0 && resp[5] == 0)
				{
				break;
				}
			}
		}
	cmd[1] = 0x0e | cameraAddr;
	cmd[2] = 0x0d;
	sendCmd(cmd, 6); 
	}
/*********************************************************************/
char CMD2[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };  
void preCapture()
	{
	unsigned char resp[6]; 
	
	while (1)
		{
		clearRxBuf();
		sendCmd(CMD2, sizeof(CMD2));
		if (readBytes((char *)resp, 6, 100) != 6)
			continue; 
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0)
			break; 
		}
	}
char CMD3[] = {0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN>>8) & 0xff ,0}; 
char CMD4[] = {0xaa, 0x05 | cameraAddr, 0x00, 0x00, 0x00, 0x00};
char CMD5[] = {0xaa, 0x04 | cameraAddr, 0x01, 0x00, 0x00, 0x00};
void Capture()
	{
	unsigned char resp[6];

	while (1)
		{
		clearRxBuf();
		sendCmd(CMD3, sizeof(CMD3));
		if (readBytes((char *)resp, 6, 100) != 6)
			continue;
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0)
			break; 
		}
	while (1)
		{
		clearRxBuf();
		sendCmd(CMD4, sizeof(CMD4));
		if (readBytes((char *)resp, 6, 100) != 6)
			continue;
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0)
			break;
		}
	while (1) 
		{
		clearRxBuf();
		sendCmd(CMD5, sizeof(CMD5));
		if (readBytes((char *)resp, 6, 100) != 6)
			continue;
		if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
			{
			if (readBytes((char *)resp, 6, 1000) != 6)
				continue;
			if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
				{
				picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16); 
				break;
				}
			}
		}
	}
/*********************************************************************/
void GetData()
	{
	uint32_t	offset = 0;
	uint32_t	rest = picTotalLen;
	uint16_t	len;
	uint8_t		pkt[PIC_PKT_LEN];


	char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };  
	
	for (int pkt_nr = 0; rest > 0; pkt_nr++)
		{
		cmd[4] = pkt_nr & 0xff;
		cmd[5] = (pkt_nr >> 8) & 0xff;
			
		int retry_cnt = 0;
	retry:
		delay(10);
		clearRxBuf(); 
		sendCmd(cmd, 6);
		len = (rest > (PIC_PKT_LEN-6)) ? PIC_PKT_LEN : (rest+6);
		len = readBytes((char *)pkt, len, 200);
			
		unsigned char sum = 0; 
		for (int y = 0; y < len - 2; y++)
			sum += pkt[y];
		if (sum != pkt[len-2])
			{
			if (++retry_cnt < 100)
				goto retry;
			else break;
			}
		len -= 6; // payload = pkt_len - header - chksum
		S2_dump(offset, pkt+4, len);
		offset += len;
		rest -= len;
		}
	cmd[4] = 0xf0;
	cmd[5] = 0xf0; 
	sendCmd(cmd, 6);	// ?! xof
	}
