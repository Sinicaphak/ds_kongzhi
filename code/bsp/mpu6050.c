#include "bsp.h"

/******************************************************************
 * 函 数 名 称：MPU6050_WriteReg
 * 函 数 说 明：IIC连续写入数据
 * 函 数 形 参：addr器件地址 regaddr寄存器地址 num要写入的长度 regdata写入的数据地址
 * 函 数 返 回：0=读取成功   其他=读取失败
 * 作       者：LC
 * 备       注：无
******************************************************************/
char MPU6050_WriteReg(uint8_t addr,uint8_t regaddr,uint8_t num,uint8_t *regdata)
{
    uint16_t i = 0;
    IIC_Start();
    Send_Byte((addr<<1)|0);
    if( I2C_WaitAck() == 1 ) {IIC_Stop();return 1;}
    Send_Byte(regaddr);
    if( I2C_WaitAck() == 1 ) {IIC_Stop();return 2;}

    for(i=0;i<num;i++)
    {
        Send_Byte(regdata[i]);
        if( I2C_WaitAck() == 1 ) {IIC_Stop();return (3+i);}
    }

    IIC_Stop();
    return 0;
}


/******************************************************************
 * 函 数 名 称：MPU6050_ReadData
 * 函 数 说 明：IIC连续读取数据
 * 函 数 形 参：addr器件地址 regaddr寄存器地址 num要读取的长度 Read读取到的数据要存储的地址
 * 函 数 返 回：0=读取成功   其他=读取失败
 * 作       者：LC
 * 备       注：无
******************************************************************/
char MPU6050_ReadData(uint8_t addr, uint8_t regaddr,uint8_t num,uint8_t* Read)
{
        uint8_t i;
        IIC_Start();
        Send_Byte((addr<<1)|0);
        if( I2C_WaitAck() == 1 ) {IIC_Stop();return 1;}
        Send_Byte(regaddr);
        if( I2C_WaitAck() == 1 ) {IIC_Stop();return 2;}

        IIC_Start();
        Send_Byte((addr<<1)|1);
        if( I2C_WaitAck() == 1 ) {IIC_Stop();return 3;}

        for(i=0;i<(num-1);i++){
                Read[i]=Read_Byte();
                IIC_Send_Ack(0);
        }
        Read[i]=Read_Byte();
        IIC_Send_Ack(1);
        IIC_Stop();
        return 0;
}


/******************************************************************
 * 函 数 名 称：MPU_Set_Gyro_Fsr
 * 函 数 说 明：设置MPU6050陀螺仪传感器满量程范围
 * 函 数 形 参：fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
 * 函 数 返 回：0,设置成功  其他,设置失败
 * 作       者：LC
 * 备       注：无
******************************************************************/
uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr)
{
        return MPU6050_WriteReg(0x68,MPU_GYRO_CFG_REG,1,(uint8_t*)(fsr<<3)); //设置陀螺仪满量程范围
}

/******************************************************************
 * 函 数 名 称：MPU_Set_Accel_Fsr
 * 函 数 说 明：设置MPU6050加速度传感器满量程范围
 * 函 数 形 参：fsr:0,±2g;1,±4g;2,±8g;3,±16g
 * 函 数 返 回：0,设置成功  其他,设置失败
 * 作       者：LC
 * 备       注：无
******************************************************************/
uint8_t MPU_Set_Accel_Fsr(uint8_t fsr)
{
        return MPU6050_WriteReg(0x68,MPU_ACCEL_CFG_REG,1,(uint8_t*)(fsr<<3)); //设置加速度传感器满量程范围
}

/******************************************************************
 * 函 数 名 称：MPU_Set_LPF
 * 函 数 说 明：设置MPU6050的数字低通滤波器
 * 函 数 形 参：lpf:数字低通滤波频率(Hz)
 * 函 数 返 回：0,设置成功  其他,设置失败
 * 作       者：LC
 * 备       注：无
******************************************************************/
uint8_t MPU_Set_LPF(uint16_t lpf)
{
      uint8_t data=0;

      if(lpf>=188)data=1;
      else if(lpf>=98)data=2;
      else if(lpf>=42)data=3;
      else if(lpf>=20)data=4;
      else if(lpf>=10)data=5;
      else data=6;
      return data=MPU6050_WriteReg(0x68,MPU_CFG_REG,1,&data);//设置数字低通滤波器
}
/******************************************************************
 * 函 数 名 称：MPU_Set_Rate
 * 函 数 说 明：设置MPU6050的采样率(假定Fs=1KHz)
 * 函 数 形 参：rate:4~1000(Hz)  初始化中rate取50
 * 函 数 返 回：0,设置成功  其他,设置失败
 * 作       者：LC
 * 备       注：无
******************************************************************/
uint8_t MPU_Set_Rate(uint16_t rate)
{
        uint8_t data;
        if(rate>1000)rate=1000;
        if(rate<4)rate=4;
        data=1000/rate-1;
        data=MPU6050_WriteReg(0x68,MPU_SAMPLE_RATE_REG,1,&data);        //设置数字低通滤波器
        return MPU_Set_LPF(rate/2);            //自动设置LPF为采样率的一半
}


/******************************************************************
 * 函 数 名 称：MPU6050ReadGyro
 * 函 数 说 明：读取陀螺仪数据
 * 函 数 形 参：陀螺仪数据存储地址
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void MPU6050ReadGyro(short *gyroData)
{
        uint8_t buf[6];
        uint8_t reg = 0;
        //MPU6050_GYRO_OUT = MPU6050陀螺仪数据寄存器地址
        //陀螺仪数据输出寄存器总共由6个寄存器组成，
        //输出X/Y/Z三个轴的陀螺仪传感器数据，高字节在前，低字节在后。
        //每一个轴16位，按顺序为xyz
        reg = MPU6050_ReadData(0x68,MPU6050_GYRO_OUT,6,buf);
        if( reg == 0 )
        {
                gyroData[0] = (buf[0] << 8) | buf[1];
                gyroData[1] = (buf[2] << 8) | buf[3];
                gyroData[2] = (buf[4] << 8) | buf[5];
        }
}

/******************************************************************
 * 函 数 名 称：MPU6050ReadAcc
 * 函 数 说 明：读取加速度数据
 * 函 数 形 参：加速度数据存储地址
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void MPU6050ReadAcc(short *accData)
{
        uint8_t buf[6];
        uint8_t reg = 0;
        //MPU6050_ACC_OUT = MPU6050加速度数据寄存器地址
        //加速度传感器数据输出寄存器总共由6个寄存器组成，
        //输出X/Y/Z三个轴的加速度传感器值，高字节在前，低字节在后。
        reg = MPU6050_ReadData(0x68, MPU6050_ACC_OUT, 6, buf);
        if( reg == 0)
        {
                accData[0] = (buf[0] << 8) | buf[1];
                accData[1] = (buf[2] << 8) | buf[3];
                accData[2] = (buf[4] << 8) | buf[5];
        }
}

/******************************************************************
 * 函 数 名 称：MPU6050_GetTemp
 * 函 数 说 明：读取MPU6050上的温度
 * 函 数 形 参：无
 * 函 数 返 回：温度值单位为℃
 * 作       者：LC
 * 备       注：温度换算公式为：Temperature = 36.53 + regval/340
******************************************************************/
float MPU6050_GetTemp(void)
{
    short temp3;
    uint8_t buf[2];
    float Temperature = 0;
    MPU6050_ReadData(0x68,MPU6050_RA_TEMP_OUT_H,2,buf);
    temp3= (buf[0] << 8) | buf[1];
    Temperature=((double) temp3/340.0)+36.53;
    return Temperature;
}

/******************************************************************
 * 函 数 名 称：MPU6050ReadID
 * 函 数 说 明：读取MPU6050的器件地址
 * 函 数 形 参：无
 * 函 数 返 回：0=检测不到MPU6050   1=能检测到MPU6050
 * 作       者：LC
 * 备       注：无
******************************************************************/
uint8_t MPU6050ReadID(void)
{
        unsigned char Re[2] = {0};
        //器件ID寄存器 = 0x75
        printf("mpu=%d\r\n",MPU6050_ReadData(0x68,0X75,1,Re)); //读器件地址

        if (Re[0] != 0x68)
        {
                  printf("检测不到 MPU6050 模块");
                  return 1;
         }
        else
        {
                printf("MPU6050 ID = %x\r\n",Re[0]);
                return 0;
        }
        return 0;
}

/******************************************************************
 * 函 数 名 称：MPU6050_Init
 * 函 数 说 明：MPU6050初始化
 * 函 数 形 参：无
 * 函 数 返 回：0成功  1没有检测到MPU6050
 * 作       者：LC
 * 备       注：无
******************************************************************/
char MPU6050_Init(void)
{
    SDA_OUT();
    delay_ms(10);
    //复位6050
    MPU6050_WriteReg(0x68,MPU6050_RA_PWR_MGMT_1, 1,(uint8_t*)(0x80));
    delay_ms(100);
    //电源管理寄存器
    //选择X轴陀螺作为参考PLL的时钟源，设置CLKSEL=001
    MPU6050_WriteReg(0x68,MPU6050_RA_PWR_MGMT_1,1, (uint8_t*)(0x00));

    MPU_Set_Gyro_Fsr(3);    //陀螺仪传感器,±2000dps
    MPU_Set_Accel_Fsr(0);   //加速度传感器,±2g
    MPU_Set_Rate(50);

    MPU6050_WriteReg(0x68,MPU_INT_EN_REG , 1,(uint8_t*)0x00);        //关闭所有中断
    MPU6050_WriteReg(0x68,MPU_USER_CTRL_REG,1,(uint8_t*)0x00);       //I2C主模式关闭
    MPU6050_WriteReg(0x68,MPU_FIFO_EN_REG,1,(uint8_t*)0x00);         //关闭FIFO
    MPU6050_WriteReg(0x68,MPU_INTBP_CFG_REG,1,(uint8_t*)0X80);       //INT引脚低电平有效

    if( MPU6050ReadID() == 0 )//检查是否有6050
    {
            MPU6050_WriteReg(0x68,MPU6050_RA_PWR_MGMT_1, 1,(uint8_t*)0x01);//设置CLKSEL,PLL X轴为参考
            MPU6050_WriteReg(0x68,MPU_PWR_MGMT2_REG, 1,(uint8_t*)0x00);//加速度与陀螺仪都工作
            MPU_Set_Rate(50);
            return 1;
    }
    return 0;
}