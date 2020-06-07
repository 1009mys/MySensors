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
 * 이 예제는 센서 값으로 배터리의 남은 용량을 알려주는 예제다.
 * A0핀으로 배터리 용량 측정은 여기서 확인가능:
 * http://www.mysensors.org/build/battery
 *
 */


// 시리얼 모니터의 디버그 출력 사용
#define MY_DEBUG

// 연결된 무선 타입 선택
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <MySensors.h>

int BATTERY_SENSE_PIN = A0;  // 배터리 감지 입력 핀을 선택

uint32_t SLEEP_TIME = 900000;  // 인식 사이 시간(밀리초)
int oldBatteryPcnt = 0;

void setup()
{
	// 1.1V 내부 참조 사용
#if defined(__AVR_ATmega2560__)
	analogReference(INTERNAL1V1);
#else
	analogReference(INTERNAL);
#endif
}

void presentation()
{
	// 스케치버전 정보를 게이트웨이와 턴트롤러로 전송
	sendSketchInfo("Battery Meter", "1.0");
}

void loop()
{
	// 배터리 전압 얻음
	int sensorValue = analogRead(BATTERY_SENSE_PIN);
#ifdef MY_DEBUG
	Serial.println(sensorValue);
#endif

	// 배터리 간 1M, 470K 분배기와 1.1v의 내부 ADC 레퍼런스 사용
	// 감지 지점을 0.1 uF 캡으로 바이 패스하여 해당 지점에서 오차를 줄입니다.
	// ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
	// 3.44/1023 = (비트당 전압) = 0.003363075

	int batteryPcnt = sensorValue / 10;

#ifdef MY_DEBUG
	float batteryV  = sensorValue * 0.003363075;
	Serial.print("Battery Voltage: ");
	Serial.print(batteryV);
	Serial.println(" V");

	Serial.print("Battery percent: ");
	Serial.print(batteryPcnt);
	Serial.println(" %");
#endif

	if (oldBatteryPcnt != batteryPcnt) {
		// 대기 후 전원 켜기
		sendBatteryLevel(batteryPcnt);
		oldBatteryPcnt = batteryPcnt;
	}
	sleep(SLEEP_TIME);
}
