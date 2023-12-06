/*************************************************************************
	> File Name: ic_tm1650.c
	> Author: 
	> Mail: 
	> Created Time: Mon 05 Dec 2022 00:47:43 HKT
 ************************************************************************/
#include "ic_tm1650.h"
#include "ic_tm1650_config.h"


//产生IIC总线起始信号
static void TM1650_IIC_start(void){
	TM1650_IIC_SCL_HIGH;     //SCL=1
	TM1650_IIC_SDA_HIGH;    //SDA=1
	TM1650_IIC_DELAY_4US;
	TM1650_IIC_SDA_LOW;     //SDA=0
	TM1650_IIC_DELAY_4US;
	TM1650_IIC_SCL_LOW;      //SCL=0
}
 
//产生IIC总线结束信号
static void TM1650_IIC_stop(void){
	TM1650_IIC_SCL_LOW;      //SCL=0
	TM1650_IIC_SDA_LOW;      //SDA=0
	TM1650_IIC_DELAY_4US;
	TM1650_IIC_SCL_HIGH;     //SCL=1
	TM1650_IIC_DELAY_4US;
	TM1650_IIC_SDA_HIGH;    //SDA=1
}
 
//通过IIC总线发送一个字节
static void TM1650_IIC_write_byte(unsigned char dat){
	unsigned char i;
	
	TM1650_IIC_SCL_LOW;
	for(i=0;i<8;i++){
		TM1650_IIC_SDA_WR(dat&0x80);
		dat<<=1;	
		
		TM1650_IIC_DELAY_2US;
		TM1650_IIC_SCL_HIGH;
		TM1650_IIC_DELAY_2US;
		TM1650_IIC_SCL_LOW;
		TM1650_IIC_DELAY_2US;
	}
}
 
//通过IIC总线接收从机响应的ACK信号
static unsigned char TM1650_IIC_wait_ack(void){
	unsigned char ack_signal = 0;
	
	TM1650_IIC_SDA_HIGH;    //SDA=1
	TM1650_IIC_DELAY_2US;
	TM1650_IIC_SCL_HIGH;
	TM1650_IIC_DELAY_2US;
	if(TM1650_IIC_SDA_RD()) ack_signal = 1;   //如果读取到的是NACK信号
	TM1650_IIC_SCL_LOW;
	TM1650_IIC_DELAY_2US;
	return ack_signal;
}
 
 
//TM1650初始化
void TM1650_init(void){
	TM1650_IIC_SCL_MODE_OD;  //SCL开漏输出
    TM1650_IIC_SDA_MODE_OD;  //SDA开漏输出
 
    TM1650_IIC_SDA_HIGH;   //释放SDA线
    TM1650_IIC_SCL_HIGH;   //释放SCL线
	
	TM1650_cfg_display(TM1650_BRIGHT5);   //初始化为5级亮度，打开显示
	TM1650_clear();     //将显存内容清0
}
 
 
//作用：设置显示参数
//备注：这个操作不影响显存中的数据
//用例：
//	设置亮度并打开显示:TM1650_cfg_display(TM1650_BRIGHTx)
//	关闭显示:TM1650_cfg_display(TM1650_DSP_OFF)
void TM1650_cfg_display(unsigned char param){
	TM1650_IIC_start();
	TM1650_IIC_write_byte(0x48);  TM1650_IIC_wait_ack();     //固定命令
	TM1650_IIC_write_byte(param); TM1650_IIC_wait_ack();    //参数值
	TM1650_IIC_stop();
}
 
 
//将显存数据全部刷为0，清空显示
void TM1650_clear(void){
	unsigned char dig;
	for(dig = TM1650_DIG1 ; dig<= TM1650_DIG4 ;dig++){
		TM1650_print(dig,0);   //将显存数据刷为0
	}
}
 
//往一个指定的数码管位写入指定的显示数据
//共阴数码管段码表：
//const unsigned char TUBE_TABLE_0[10] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};                                  //共阴，0~9的数字
//const unsigned char TUBE_TABLE_0[16] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};    //共阴，0~9~A~F
//用例：
//	在DIG1位上显示数字3: TM1650_print(TM1650_DIG1,TUBE_TABLE_0[3]);
void TM1650_print(unsigned char dig,unsigned char seg_data){
	TM1650_IIC_start();
	TM1650_IIC_write_byte(dig*2+0x68); TM1650_IIC_wait_ack();  //显存起始地址为0x68
	TM1650_IIC_write_byte(seg_data); TM1650_IIC_wait_ack();    //发送段码
	TM1650_IIC_stop();
}

