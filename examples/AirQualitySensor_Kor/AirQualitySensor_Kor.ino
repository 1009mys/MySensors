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
 * MQ2 센서를 아래와 같이 연결 :
 *
 *   A H A   >>> 5V
 *   B       >>> A0
 *   H       >>> GND
 *   B       >>> 10K ohm >>> GND
 *
 * 작성자: epierre
 * http://sandboxelectronics.com/?p=165 에 기초함
 * 라이센스: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 * 1.4로 HEK에 의해 업데이트됨
 *
 */

// 시리얼 모니터에 버그 출력 활성화
#define MY_DEBUG

// 신호 타입 선택과 활성화
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <MySensors.h>

#define 	CHILD_ID_MQ                   0
/************************하드웨어 관련 매크로*************************************/
#define 	MQ_SENSOR_ANALOG_PIN         (0)  //사용할 아날로그 입력 채널 정의
#define         RL_VALUE                     (5)     //킬로옴 단위로 보드의 저항값 설정
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //RO_CLEAR_AIR_FACTOR=(깨끗한 공기에서의 센서 저항)/RO,
//데이터시트에서 파생됨
/***********************소프트웨어 관력 매크로************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //측정 단계에서 사용할 샘플 개수 정의
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //밀리초 단위로 샘플 측정 간격 설정
#define         READ_SAMPLE_INTERVAL         (50)    //일반적인 가동상황에서 사용할 샘플 개수 정의
#define         READ_SAMPLE_TIMES            (5)     //샘플과 가동 사이 간격을 밀리초 단위로 정의
/**********************어플리케이션 관력 매크로**********************************/
#define         GAS_LPG                      (0)
#define         GAS_CO                       (1)
#define         GAS_SMOKE                    (2)
/*****************************전역 변수***********************************************/
uint32_t SLEEP_TIME = 30000; // 밀리초 단위로 읽는 간격 설정
//가변적인 변수
float Ro = 10000.0;    // 10K Ohm으로 설정해야함
int val = 0;           // 센서값 저장 변수
uint16_t lastMQ = 0;
float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //곡선의 2개의 점
//이 두 점으로, 선이 거의 원래 곡선만큼 동등해짐
//데이터 초기화:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59)
float           COCurve[3]  =  {2.3,0.72,-0.34};    //two points are taken from the curve.
//이 두 점으로, 선이 거의 원래 곡선만큼 동등해짐
//데이터 초기화:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15)
float           SmokeCurve[3] = {2.3,0.53,-0.44};   //two points are taken from the curve.
//이 두 점으로, 선이 거의 원래 곡선만큼 동등해짐
//데이터 초기화:{ x, y, slope}; point1: (lg200, 0.53), point2:(lg10000,-0.22)


MyMessage msg(CHILD_ID_MQ, V_LEVEL);

void setup()
{
	Ro = MQCalibration(
	         MQ_SENSOR_ANALOG_PIN);         //센서 초기화. 깨끗한 공기중에 둘 것
}

void presentation()
{
	// 게이트웨이와 컨트롤러에 스케치 버젼 정보 보냄
	sendSketchInfo("Air Quality Sensor", "1.0");

	// 게이트웨이에 모든 센서 등록(자식 디바이스로 만들어짐)
	present(CHILD_ID_MQ, S_AIR_QUALITY);
}

void loop()
{
	uint16_t valMQ = MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_CO);
	Serial.println(val);

	Serial.print("LPG:");
	Serial.print(MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_LPG) );
	Serial.print( "ppm" );
	Serial.print("    ");
	Serial.print("CO:");
	Serial.print(MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_CO) );
	Serial.print( "ppm" );
	Serial.print("    ");
	Serial.print("SMOKE:");
	Serial.print(MQGetGasPercentage(MQRead(MQ_SENSOR_ANALOG_PIN)/Ro,GAS_SMOKE) );
	Serial.print( "ppm" );
	Serial.print("\n");

	if (valMQ != lastMQ) {
		send(msg.set((int16_t)ceil(valMQ)));
		lastMQ = ceil(valMQ);
	}

	sleep(SLEEP_TIME); //sleep for: sleepTime
}

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - adc에서 전압 측정값
Output:  계산된 센서 저항값
Remarks: 센서와 레지스터는 전압분배기를 형성함. 부하저항 양단의 저항으로 센서값 도출
************************************************************************************/
float MQResistanceCalculation(int raw_adc)
{
	return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - 아날로그 채널
Output:  센서의 Ro
Remarks: 이 기능은 센서가 깨끗한 공기에 있다고 가정한다. 
         MQResistanceCalculation를 사용해 깨끗한 공기에서의 센서값을 계산하고, RO_CLEAN_AIR_FACTOR로 나눔.
         RO_CLEAN_AIR_FACTOR은 다른 센서 사이간의 차이인 10이다.
************************************************************************************/
float MQCalibration(int mq_pin)
{
	int i;
	float inVal=0;

	for (i=0; i<CALIBARAION_SAMPLE_TIMES; i++) {          //여러 샘플 측정
		inVal += MQResistanceCalculation(analogRead(mq_pin));
		delay(CALIBRATION_SAMPLE_INTERVAL);
	}
	inVal = inVal/CALIBARAION_SAMPLE_TIMES;                   //편균값 계산

	inVal = inVal/RO_CLEAN_AIR_FACTOR;                        //RO_CLEAN_AIR_FACTOR로 나눈 값은 Ro
	//according to the chart in the datasheet

	return inVal;
}
/*****************************  MQRead *********************************************
Input:   mq_pin - 아날로그 채널
Output:  센서의 Rs
Remarks: 이 기능은 MQResistanceCalculation로 센서값을 계산한다. Rs 변화는 목표 기체에 따라 달라짐.
         샘플 시간과 값 사이시간은 매크로 정의 변화에 의해 구성된다.
************************************************************************************/
float MQRead(int mq_pin)
{
	int i;
	float rs=0;

	for (i=0; i<READ_SAMPLE_TIMES; i++) {
		rs += MQResistanceCalculation(analogRead(mq_pin));
		delay(READ_SAMPLE_INTERVAL);
	}

	rs = rs/READ_SAMPLE_TIMES;

	return rs;
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Ro로 나뉜 Rs
         gas_id      - 목표 기체 타입
Output:  목표 기체의 ppm
Remarks: 이 기능은 서로 다른 곡선을 MQGetPercentage로 전달해 목표 가스의 ppm(백만 분의 1)을 계산한다.
************************************************************************************/
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
	if ( gas_id == GAS_LPG ) {
		return MQGetPercentage(rs_ro_ratio,LPGCurve);
	} else if ( gas_id == GAS_CO ) {
		return MQGetPercentage(rs_ro_ratio,COCurve);
	} else if ( gas_id == GAS_SMOKE ) {
		return MQGetPercentage(rs_ro_ratio,SmokeCurve);
	}

	return 0;
}

/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Ro로 나뉜 Rs
         pcurve      - 목표 기체의 곡선에 대한 포인터
Output:  목표 기체의 ppm
Remarks: 기울기와 선의 점을 사용한다. y(rs_ro_ratio)가 제공되면 선의 x(로그의 ppm값)를 도출 할 수 있습니다.
         로그 좌표이므로, 10의 거듭제곱으로 결과를 비 로그 값으로 변환한다.
************************************************************************************/
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
	return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
