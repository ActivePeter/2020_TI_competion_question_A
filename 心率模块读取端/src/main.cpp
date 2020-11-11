#include <ads1292r/ads1292r.h>
#include <SPI.h>

ads1292r ADS1292; // define class
/************************************************************************************/
//Packet format
#define CES_CMDIF_PKT_START_1 0x0A
#define CES_CMDIF_PKT_START_2 0xFA
#define CES_CMDIF_TYPE_DATA 0x02
#define CES_CMDIF_PKT_STOP 0x0B

#define FILTERORDER 161
#define NRCOEFF (0.992)

/***********************************************************************************/
void ECG_ProcessCurrSample(int16_t *CurrAqsSample, int16_t *FilteredOut);
void ECG_FilterProcess(int16_t *WorkingBuff, int16_t *CoeffBuf, int16_t *FilterOut);
// uint32_t Bfilter_high(uint32_t input_data);
// uint32_t Bfilter_low(uint32_t input_data);

/***********************************************************************************/
/*SPI参数定义*/
volatile uint8_t SPI_Dummy_Buff[30];
uint8_t DataPacketHeader[6];
volatile signed long s32DaqVals[8];
uint8_t data_len = 8;
volatile byte SPI_RX_Buff[15];
volatile static int SPI_RX_Buff_Count = 0;
volatile char *SPI_RX_Buff_Ptr;
volatile bool ads1292dataReceived = false;
unsigned long uecgtemp = 0;
signed long secgtemp = 0;
int i, j;

int16_t CoeffBuf_40Hz_LowPass[FILTERORDER] = //滤波器参数
    {
        -72, 122, -31, -99, 117, 0, -121, 105, 34,
        -137, 84, 70, -146, 55, 104, -147, 20, 135,
        -137, -21, 160, -117, -64, 177, -87, -108, 185,
        -48, -151, 181, 0, -188, 164, 54, -218, 134,
        112, -238, 90, 171, -244, 33, 229, -235, -36,
        280, -208, -115, 322, -161, -203, 350, -92, -296,
        361, 0, -391, 348, 117, -486, 305, 264, -577,
        225, 445, -660, 93, 676, -733, -119, 991, -793,
        -480, 1486, -837, -1226, 2561, -865, -4018, 9438, 20972,
        9438, -4018, -865, 2561, -1226, -837, 1486, -480, -793,
        991, -119, -733, 676, 93, -660, 445, 225, -577,
        264, 305, -486, 117, 348, -391, 0, 361, -296,
        -92, 350, -203, -161, 322, -115, -208, 280, -36,
        -235, 229, 33, -244, 171, 90, -238, 112, 134,
        -218, 54, 164, -188, 0, 181, -151, -48, 185,
        -108, -87, 177, -64, -117, 160, -21, -137, 135,
        20, -147, 104, 55, -146, 70, 84, -137, 34,
        105, -121, 0, 117, -99, -31, 122, -72};

int16_t ECG_WorkingBuff[2 * FILTERORDER];

/* Variable which will hold the calculated heart rate */
int16_t ecg_wave_buff[1], ecg_filterout[1];

long status_byte = 0;
uint8_t LeadStatus = 0;
boolean leadoff_deteted = true;
/*巴特沃斯滤波所用*/
// float x[3], y[3], x_1[3], y_1[3];
// float b[3] = {0.9996, -1.9992, 0.9996};
// float a[3] = {1.0000, -1.9992, 0.9992};
// float b_1[3] = {0.4997, 0.9994, 0.4997};
// float a_1[3] = {1.0000, 0.7310, 0.2677};
namespace Heart_Namespace
{
  float Momentfrequence = 0;
#define Heart_cache_len 40
  float cache[Heart_cache_len] = {0};
  int cacheStep = 0;
  short maxValueIndex = 0;
  short minValueIndex = 0;
  float getCacheDataByIndex(int index)
  {
    while (index < 0)
    {
      index += Heart_cache_len;
    }
    index %= Heart_cache_len;

    return cache[index];
  }
  int getMinValue()
  {
    return cache[minValueIndex];
  }
  int getMaxValue()
  {
    return cache[maxValueIndex];
  }
  int findMaxIndex()
  {
    int maxindex = 0;
    for (int i = 1; i < Heart_cache_len; i++)
    {
      if (cache[i] > cache[maxindex])
      {
        maxindex = i;
      }
    }
    return maxindex;
  }
  int findMinIndex()
  {
    int minindex = 0;
    for (int i = 1; i < Heart_cache_len; i++)
    {
      if (cache[i] < cache[minindex])
      {
        minindex = i;
      }
    }
    return minindex;
  }
  void pushDataToCache(double data)
  {
    if (data > cache[maxValueIndex])
    {
      maxValueIndex = cacheStep;
    }
    else
    {
      if (cacheStep == maxValueIndex)
      {
        cache[cacheStep] = data;
        maxValueIndex = findMaxIndex();
      }
    }
    if (data < cache[minValueIndex])
    {
      minValueIndex = cacheStep;
    }
    else
    {
      if (cacheStep == minValueIndex)
      {
        cache[cacheStep] = data;
        minValueIndex = findMinIndex();
      }
    }
    cache[cacheStep] = data;
    // if()
    cacheStep++;
    if (cacheStep == Heart_cache_len)
    {
      cacheStep = 0;
    }
  }
} // namespace Heart_Namespace
void setup()
{
  // initalize the  data ready and chip select pins:
  pinMode(27,INPUT_PULLUP);
  pinMode(ADS1292_DRDY_PIN, INPUT);   //6
  pinMode(ADS1292_CS_PIN, OUTPUT);    //7
  pinMode(ADS1292_START_PIN, OUTPUT); //5
  pinMode(ADS1292_PWDN_PIN, OUTPUT);  //4

  Serial.begin(115200); // Baudrate for serial communica

  ADS1292.ads1292_Init(); //initalize ADS1292 slave
}

void loop()
{
  if ((digitalRead(ADS1292_DRDY_PIN)) == LOW) // Sampling rate is set to 125SPS ,DRDY ticks for every 8ms
  {
    SPI_RX_Buff_Ptr = ADS1292.ads1292_Read_Data(); // Read the data,point the data to a pointer

    for (i = 0; i < 9; i++)
    {
      SPI_RX_Buff[SPI_RX_Buff_Count++] = *(SPI_RX_Buff_Ptr + i); // store the result data in array
    }
    ads1292dataReceived = true;
  }

  if (ads1292dataReceived == true) // process the data
  {
    j = 0;
    for (i = 0; i < 6; i += 3) // data outputs is (24 status bits + 24 bits Respiration data +  24 bits ECG data)
    {

      uecgtemp = (unsigned long)(((unsigned long)SPI_RX_Buff[i + 3] << 16) | ((unsigned long)SPI_RX_Buff[i + 4] << 8) | (unsigned long)SPI_RX_Buff[i + 5]);
      uecgtemp = (unsigned long)(uecgtemp << 8);
      secgtemp = (signed long)(uecgtemp);
      secgtemp = (signed long)(secgtemp >> 8);

      s32DaqVals[j++] = secgtemp;
    }
    status_byte = (long)((long)SPI_RX_Buff[2] | ((long)SPI_RX_Buff[1]) << 8 | ((long)SPI_RX_Buff[0]) << 16); // First 3 bytes represents the status
    status_byte = (status_byte & 0x0f8000) >> 15;                                                            // bit15 gives the lead status
    LeadStatus = (unsigned char)status_byte;

    if (!((LeadStatus & 0x1f) == 0))
      leadoff_deteted = true;
    else
      leadoff_deteted = false;

    ecg_wave_buff[0] = (int16_t)(s32DaqVals[1] >> 8); // ignore the lower 8 bits out of 24bits

    if (leadoff_deteted == false)
    {
      ECG_ProcessCurrSample(&ecg_wave_buff[0], &ecg_filterout[0]); // filter out the line noise @40Hz cutoff 161 order
                                                                   //QRS_Algorithm_Interface(ecg_filterout[0]);             // calculate
    }
    else
      ecg_filterout[0] = 0;

    Heart_Namespace::pushDataToCache(ecg_filterout[0]);
    int sum = 0;
    sum=Heart_Namespace::getCacheDataByIndex(Heart_Namespace::cacheStep - 1)-Heart_Namespace::getCacheDataByIndex(Heart_Namespace::cacheStep - 4);
    // for (int i = 0; i < 5; i++)
    // {
    //   sum += Heart_Namespace::getCacheDataByIndex(Heart_Namespace::cacheStep - i);
    // }
    static uint64_t lastMillis=0;
    static uint16_t deltaTime=0;
    
    int threshold=0;
    if(digitalRead(27)){
      threshold=9;
    }else{
      threshold=30;
    }
    if (sum > threshold)
    {
      uint64_t curMillis=millis();
      if(curMillis-lastMillis>400){
        deltaTime= curMillis-lastMillis;
        
        // Serial.printf("heartRate1 %d %d\r\n",deltaTime,sum);
        lastMillis=curMillis;
      }
      
    }

    /*巴特沃斯滤波，有bug*/
    // uint32_t bdata = Bfilter_high(s32DaqVals[1]);
    // bdata = Bfilter_low(bdata);
    /*原始帧*/
    DataPacketHeader[0] = CES_CMDIF_PKT_START_1; // Packet header1 :0x0A
    DataPacketHeader[1] = CES_CMDIF_PKT_START_2; // Packet header2 :0xFA
    //DataPacketHeader[2] = (uint8_t)(data_len);   // data length
    //DataPacketHeader[3] = (uint8_t)(data_len >> 8);
    // DataPacketHeader[4] = CES_CMDIF_TYPE_DATA; // packet type: 0x02 -data 0x01 -commmand
    /*
    DataPacketHeader[2] = s32DaqVals[1]; // 4 bytes ECG data
    DataPacketHeader[3] = s32DaqVals[1] >> 8;
    DataPacketHeader[4] = s32DaqVals[1] >> 16;
    DataPacketHeader[5] = s32DaqVals[1] >> 24;
*/
    DataPacketHeader[2] = ecg_filterout[0];
    DataPacketHeader[3] = ecg_filterout[0] >> 8;
    DataPacketHeader[4] = deltaTime;
    DataPacketHeader[5] = deltaTime>>8;
    // DataPacketHeader[9] = s32DaqVals[0]; // 4 bytes Respiration data
    // DataPacketHeader[10] = s32DaqVals[0] >> 8;
    // DataPacketHeader[11] = s32DaqVals[0] >> 16;
    // DataPacketHeader[12] = s32DaqVals[0] >> 24;

    // DataPacketHeader[13] = CES_CMDIF_TYPE_DATA; // Packet footer1:0x00
    // DataPacketHeader[14] = CES_CMDIF_PKT_STOP;  // Packet footer2:0x0B
    Serial.write(DataPacketHeader,6);
    // for (i = 0; i < 6; i++)
    // {
    //   Serial.write(DataPacketHeader[i]); // transmit the data over USB
    // }
  }
  //Serial.printf("%ld %d \r\n", s32DaqVals[1], ecg_filterout[0]);

  ads1292dataReceived = false;
  SPI_RX_Buff_Count = 0;
}

void ECG_FilterProcess(int16_t *WorkingBuff, int16_t *CoeffBuf, int16_t *FilterOut)
{

  int32_t acc = 0; // accumulator for MACs
  int k;

  // perform the multiply-accumulate
  for (k = 0; k < 161; k++)
  {
    acc += (int32_t)(*CoeffBuf++) * (int32_t)(*WorkingBuff--);
  }
  // saturate the result
  if (acc > 0x3fffffff)
  {
    acc = 0x3fffffff;
  }
  else if (acc < -0x40000000)
  {
    acc = -0x40000000;
  }
  // convert from Q30 to Q15
  *FilterOut = (int16_t)(acc >> 15);
  //*FilterOut = *WorkingBuff;
}

void ECG_ProcessCurrSample(int16_t *CurrAqsSample, int16_t *FilteredOut)
{
  static uint16_t ECG_bufStart = 0, ECG_bufCur = FILTERORDER - 1, ECGFirstFlag = 1;
  static int16_t ECG_Pvev_DC_Sample, ECG_Pvev_Sample; /* Working Buffer Used for Filtering*/
  //  static short ECG_WorkingBuff[2 * FILTERORDER];
  int16_t *CoeffBuf;
  int16_t temp1, temp2, ECGData;

  /* Count variable*/
  uint16_t Cur_Chan;
  int16_t FiltOut = 0;
  //  short FilterOut[2];
  CoeffBuf = CoeffBuf_40Hz_LowPass; // Default filter option is 40Hz LowPass

  if (ECGFirstFlag) // First Time initialize static variables.
  {
    for (Cur_Chan = 0; Cur_Chan < FILTERORDER; Cur_Chan++)
    {
      ECG_WorkingBuff[Cur_Chan] = 0;
    }
    ECG_Pvev_DC_Sample = 0;
    ECG_Pvev_Sample = 0;
    ECGFirstFlag = 0;
  }

  temp1 = NRCOEFF * ECG_Pvev_DC_Sample; //First order IIR
  ECG_Pvev_DC_Sample = (CurrAqsSample[0] - ECG_Pvev_Sample) + temp1;
  ECG_Pvev_Sample = CurrAqsSample[0];
  temp2 = ECG_Pvev_DC_Sample >> 2;
  ECGData = (int16_t)temp2;

  /* Store the DC removed value in Working buffer in millivolts range*/
  ECG_WorkingBuff[ECG_bufCur] = ECGData;
  ECG_FilterProcess(&ECG_WorkingBuff[ECG_bufCur], CoeffBuf, (int16_t *)&FiltOut);
  /* Store the DC removed value in ECG_WorkingBuff buffer in millivolts range*/
  ECG_WorkingBuff[ECG_bufStart] = ECGData;

  /* Store the filtered out sample to the LeadInfo buffer*/
  FilteredOut[0] = FiltOut; //(CurrOut);

  ECG_bufCur++;
  ECG_bufStart++;

  if (ECG_bufStart == (FILTERORDER - 1))
  {
    ECG_bufStart = 0;
    ECG_bufCur = FILTERORDER - 1;
  }
  return;
}

/*
uint32_t Bfilter_high(uint32_t input_data)
{
  int i;
  for (i = 2; i > 0; i--)
  {
    y[i] = y[i - 1];
    x[i] = x[i - 1];
  }
  x[0] = input_data;
  y[0] = 0;
  for (i = 1; i < 3; i++)
  {
    y[0] = y[0] + b[i] * x[i];
    y[0] = y[0] - a[i] * y[i];
  }
  y[0] = y[0] + b[0] * x[0];
  return y[0];
}
uint32_t Bfilter_low(uint32_t input_data)
{
  int i;
  for (i = 2; i > 0; i--)
  {
    y_1[i] = y_1[i - 1];
    x_1[i] = x_1[i - 1];
  }
  x_1[0] = input_data;
  y_1[0] = 0;
  for (i = 1; i < 3; i++)
  {
    y_1[0] = y_1[0] + b_1[i] * x_1[i];
    y_1[0] = y_1[0] - a_1[i] * y_1[i];
  }
  y_1[0] = y_1[0] + b_1[0] * x_1[0];
  return y_1[0];
}
*/