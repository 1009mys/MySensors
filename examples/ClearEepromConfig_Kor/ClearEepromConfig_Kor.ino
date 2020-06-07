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
 * 이 스케치는 EEPROM의 radioId와 relayId를 공장 출고값으로 되돌립니다.
 *
 */
// 핵심 묘듈만 로드
#define MY_CORE_ONLY

#include <MySensors.h>

void setup()
{
	Serial.begin(MY_BAUD_RATE);
	Serial.println("Started clearing. Please wait...");
	for (uint16_t i=0; i<EEPROM_LOCAL_CONFIG_ADDRESS; i++) {
		hwWriteConfig(i,0xFF);
	}
	Serial.println("Clearing done.");
}

void loop()
{
	// 아무것도 안한다.
}
