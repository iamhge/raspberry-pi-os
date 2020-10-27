#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

/*
	utils.S에 구현
	put32 : 32bit 레지스터에 데이터 씀
	get32 : 32bit 레지스터에 데이터 읽어옴
*/

void uart_send(char c)
{
	while (1)
	{
		if (get32(AUX_MU_LSR_REG) & 0x20)
			break;
	}
	put32(AUX_MU_IO_REG, c);
}

char uart_recv(void)
{
	while (1)
	{
		if (get32(AUX_MU_LSR_REG) & 0x01)
			break;
	}
	return (get32(AUX_MU_IO_REG) & 0xFF);
}

void uart_send_string(char *str)
{
	for (int i = 0; str[i] != '\0'; i++)
	{
		uart_send((char)str[i]);
	}
}

void uart_init(void)
{
	unsigned int selector;

	// GPFSEL1 레지스터는 핀 10~19에 대한 고유기능을 제어
	selector = get32(GPFSEL1);
	selector &= ~(7 << 12); // clean gpio14
	selector |= 2 << 12;	// set alt5 for gpio14
	selector &= ~(7 << 15); // clean gpio15
	selector |= 2 << 15;	// set alt5 for gpio15
	put32(GPFSEL1, selector);

	// 핀 14, 15가 항상 연결된 상태이기 때문에 pull up도 pull down도 아닌 상태여야한다.
	// 핀은 재부팅 시에도 이전 상태를 유지하기 때문에 항상 초기화가 필요.
	// 이 과정은 GPPUD, GPPUDCLK 레지스터와 관련
	put32(GPPUD, 0);						 // 필요한 control signal을 설정하기 위해 GPPUD에 쓴다. (ex. pull up, down or neither )
	delay(150);								 // 제어신호에 필요한 설정 시간 제공
	put32(GPPUDCLK0, (1 << 14) | (1 << 15)); // control signal(제어 신호)을 원하는 GPIO패드에 클럭하기 위해 GPPUDCLK0/1에 쓴다.
	delay(150);								 // 제어신호에 필요한 유지 시간 제공
	put32(GPPUDCLK0, 0);					 // clock을 제거하기 위해 GPPUDCLK0/1 에 쓴다.

	put32(AUX_ENABLES, 1);	   // mini uart 활성화 (this also enables access to it registers)
	put32(AUX_MU_CNTL_REG, 0); // 초기화하는 동안 송수신은 비활성화 (for now)
	put32(AUX_MU_IER_REG, 0);  // Disable receive and transmit interrupts
	// mini UART는 7 bit 또는 8bit 연산을 지원. 우리는 8bit 모드 사용할 것.
	put32(AUX_MU_LCR_REG, 3); // Enable 8 bit mode
	put32(AUX_MU_MCR_REG, 0); // Set RTS line to be always high(사용하지 않고 항상 high로)
	// baudrate : 통신채널에서 정보가 전송되는 속도.
	// 115200 baud는 그 직렬포트가 초당 최대 115200 bit를 전송할 수 있다는 것을 의미.
	put32(AUX_MU_BAUD_REG, 270); // Set baud rate to 115200

	// mini UART 준비가 끝나면 이를 사용해 데이터 송수신을 시도할 수 있다.
	// uart_send와 uart_recv는 무한루프로 시작되어 AUX_MU_IO_REG를 사용해  송신한 문자를 저장하거나 수신한 문자를 읽는다.
	put32(AUX_MU_CNTL_REG, 3); // Finally, enable transmitter and receiver
}
