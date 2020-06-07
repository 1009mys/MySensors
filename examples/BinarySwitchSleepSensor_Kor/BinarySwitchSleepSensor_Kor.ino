/*
 * Mysensors라이브러리는 직접만든 HA컨트롤러, 센서/엑츄에이터들의 무선통신과 프로토콜을 다룹니다.
 * 센서는 추가적인 리피터로 만들어진 자기 회복 통신을 형성합니다. 각각 리피터와 게이트웨이는 EEPROM에 라우팅 테이블을 작성함니다.
 *
 * Henrik Ekblad에 의해 제작 <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2020 Sensnology AB
 * 기여자 리스트: https://github.com/mysensors/MySensors/graphs/contributors
 *
 * 문서: http://www.mysensors.org
 * 지원 포럼: http://forum.mysensors.org
 *
 * 이 프로그램은 무료입니다. Free Software Foundation에서 배포된 GNU사용 허가 조항에 따라 배포, 수정 할 수 있습니다.
 *
 *******************************
 *
 * 안내
 *
 * 이중 인터럽트가있는 인터럽트 구동 바이너리 스위치 예제
 * 글쓴이: Patrick 'Anticimex' Fallberg
 * 버튼 한개 또는 문/창 리드 스위치를 디지털 3 및 
 * GND 및 다른 하나는 디지털 2번 핀에서 유사하게 작동됨
 * 이 예제는 아두이노 나노, 프로미니에 맞게 디자인됨
 *
 */


// 시리얼 모니터에 디버그 출력 활성화
#define MY_DEBUG

// 연결된 무선 타입 선택
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <MySensors.h>

#define SKETCH_NAME "Binary Sensor"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

#define PRIMARY_CHILD_ID 3
#define SECONDARY_CHILD_ID 4

#define PRIMARY_BUTTON_PIN 2   // 버튼/reed 스위치를 위한 아두이노 디지털핀
#define SECONDARY_BUTTON_PIN 3 // 버튼/reed 스위치를 위한 아두이노 디지털핀

#if (PRIMARY_BUTTON_PIN < 2 || PRIMARY_BUTTON_PIN > 3)
#error PRIMARY_BUTTON_PIN must be either 2 or 3 for interrupts to work
#endif
#if (SECONDARY_BUTTON_PIN < 2 || SECONDARY_BUTTON_PIN > 3)
#error SECONDARY_BUTTON_PIN must be either 2 or 3 for interrupts to work
#endif
#if (PRIMARY_BUTTON_PIN == SECONDARY_BUTTON_PIN)
#error PRIMARY_BUTTON_PIN and BUTTON_PIN2 cannot be the same
#endif
#if (PRIMARY_CHILD_ID == SECONDARY_CHILD_ID)
#error PRIMARY_CHILD_ID and SECONDARY_CHILD_ID cannot be the same
#endif


// 아래 에서 S_LIGHT를 쓰는 경우 V_LIGHT로 변경
MyMessage msg(PRIMARY_CHILD_ID, V_TRIPPED);
MyMessage msg2(SECONDARY_CHILD_ID, V_TRIPPED);

void setup()
{
	// 버튼 설정
	pinMode(PRIMARY_BUTTON_PIN, INPUT_PULLUP);
	pinMode(SECONDARY_BUTTON_PIN, INPUT_PULLUP);
}

void presentation()
{
	// 스케치버전 정보를 게이트웨이와 턴트롤러로 전송
	sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);

	// 이진 입력 센서를 센서_노드에 등록(자식 디바이스로 생성)
	// 사용법에 따라 S_DOOR, S_MOTION, S_LIGHT 사용 가능
	// S_LIGHT를 사용하는 경우, 변수 유형을 업데이트 해야합니다. 위의 "msg"를 참조하시오.
	present(PRIMARY_CHILD_ID, S_DOOR);
	present(SECONDARY_CHILD_ID, S_DOOR);
}

// BUTTON_PIN의 변경 사항에 대해서 루프 진행
void loop()
{
	uint8_t value;
	static uint8_t sentValue=2;
	static uint8_t sentValue2=2;

	// 안정된 버튼 인식을 위한 지연
	sleep(5);

	value = digitalRead(PRIMARY_BUTTON_PIN);

	if (value != sentValue) {
		// 마지막 전송에서 값이 변경되어 업데이트된 값을 보낸다
		send(msg.set(value==HIGH));
		sentValue = value;
	}

	value = digitalRead(SECONDARY_BUTTON_PIN);

	if (value != sentValue2) {
		// 마지막 전송에서 값이 변경되어 업데이트된 값을 보낸다
		send(msg2.set(value==HIGH));
		sentValue2 = value;
	}

	// 센서에 입력이 있을떄까지 딜레이
	sleep(PRIMARY_BUTTON_PIN-2, CHANGE, SECONDARY_BUTTON_PIN-2, CHANGE, 0);
}
