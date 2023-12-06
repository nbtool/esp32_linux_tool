/*************************************************************************
	> File Name: ic_ds1302.c
	> Author: 
	> Mail: 
	> Created Time: Tue 01 Aug 2023 15:06:13 CST
 ************************************************************************/
#include "ic_ds1302.h"
#include "ic_ds1302_config.h"

#define DS1302_STOP 	        0x80 
 
#define DS1302_READ_BURST 		0xBF
#define DS1302_WRITE_BURST 		0xBE
#define DS1302_WRITE_SEC 		0x80
#define DS1302_READ_SEC 		0x81
#define DS1302_WRITE_MINUTE 	0x82
#define DS1302_READ_MINUTE 		0x83
#define DS1302_WRITE_HOUR 		0x84
#define DS1302_READ_HOUR 		0x85
#define DS1302_WRITE_DATE 		0x86
#define DS1302_READ_DATE 		0x87
#define DS1302_WRITE_MONTH 	0x88
#define DS1302_READ_MONTH		0x89
#define DS1302_WRITE_WEEK 	0x8A
#define DS1302_READ_WEEK		0x8B
#define DS1302_WRITE_YEAR	 	0x8C
#define DS1302_READ_YEAR		0x8D

#define DS1302_WRITE_CHARGE 	0x90
#define DS1302_READ_CHARGE 		0x91

static const char *TAG = "ic_ds1302";
#define PR_DEBUG(...) ESP_LOGI(TAG,##__VA_ARGS__)

void DS1302_Start() {   // START , CE = 1
    DS1302_IIC_CE_LOW;
    DS1302_IIC_SCL_LOW;
	DS1302_IIC_DELAY_200US;
    DS1302_IIC_CE_HIGH;
	DS1302_IIC_DELAY_200US;
}

void DS1302_Over(){    // OVER, CE = 0
    DS1302_IIC_CE_LOW;
    DS1302_IIC_IO_LOW;
    DS1302_IIC_SCL_LOW;
}

unsigned char gc1302_read_byte(unsigned char cmd){  // 读相应的寄存器数据
	DS1302_Start();
	unsigned char i = 0;
	// write cmd
	for(i=0;i<8;i++){	
		DS1302_IIC_IO_WRITE(cmd&0x01);
		cmd >>= 1;
		DS1302_IIC_DELAY_100US;
        DS1302_IIC_SCL_HIGH;
        if(i == 7) {  // 注意，当命令字的第7位在上升沿结束后，需要在下降沿之前马上把该IO设置为输入模式
            DS1302_IIC_IO_IN;
			DS1302_IIC_DELAY_200US; 
		}else{
            // 在上升沿和下降沿之间延时200us，即周期为400us，频率为2.5KHz左右
            // 该频率可以随意设置，只要不超过最大值即可
			DS1302_IIC_DELAY_200US;  
		}
        DS1302_IIC_SCL_LOW;
		DS1302_IIC_DELAY_100US;
	}
	// read data
	unsigned char data = 0;
	for(i=0;i<8;i++){      
		data >>= 1;
		if(DS1302_IIC_IO_READ){
			data |= 0x80;
		}
		DS1302_IIC_DELAY_100US;
        DS1302_IIC_SCL_HIGH;
		DS1302_IIC_DELAY_200US;
        DS1302_IIC_SCL_LOW;
		DS1302_IIC_DELAY_100US;
	}
	DS1302_IIC_IO_OUTPUT;
	DS1302_Over();
	return data;
}

void gc1302_send_byte(unsigned char data_byte){   // 写一个字节的时序
	for(unsigned char i=0;i<8;i++){
		DS1302_IIC_IO_WRITE(data_byte&0x01);
		data_byte >>= 1;
		DS1302_IIC_DELAY_100US;
        DS1302_IIC_SCL_HIGH;
		DS1302_IIC_DELAY_200US;
        DS1302_IIC_SCL_LOW;
		DS1302_IIC_DELAY_100US;
		//DS1302_IIC_DELAY_200US;
	}
}

void gc1302_write_byte(unsigned char cmd, unsigned char data){  // 写相应的寄存器数据
	DS1302_Start();
	gc1302_send_byte(cmd);
	gc1302_send_byte(data);
	DS1302_Over();
}

void DS1302_ClearWriteProtection(){
	DS1302_Start();
	gc1302_send_byte(0x8E);
	gc1302_send_byte(0x00);
	DS1302_Over();
}

void DS1302_SetWriteProtection(){ // 设置写保护
	DS1302_Start();
	gc1302_send_byte(0x8E);
	gc1302_send_byte(0x80);
	DS1302_Over();
}

static unsigned char bcd2int(unsigned char bcd){
	return  (bcd	& 0x0F) + ((bcd>>4)*10);
}

static unsigned char int2bcd(unsigned char inx){
	return ((inx/10)<<4) | (inx%10);
}

void gc1302_update( cmp_rtc_date_s *date){
	DS1302_ClearWriteProtection();    // 清除写保护
	gc1302_write_byte(DS1302_WRITE_SEC, DS1302_STOP);	// 停止计时
	
	gc1302_write_byte(DS1302_WRITE_YEAR,int2bcd(date->year-2000)); //2000+ DCD
	gc1302_write_byte(DS1302_WRITE_MONTH,int2bcd(date->month));
	gc1302_write_byte(DS1302_WRITE_DATE, int2bcd(date->monthday));    // 设置天
	//gc1302_write_byte(DS1302_WRITE_WEEK, int2bcd(week));   
	gc1302_write_byte(DS1302_WRITE_HOUR, int2bcd(date->hour));    // 设置时
	gc1302_write_byte(DS1302_WRITE_MINUTE, int2bcd(date->minute));  // 设置分
	gc1302_write_byte(DS1302_WRITE_SEC, int2bcd(date->second));     // 设置秒，并开始计时

	PR_DEBUG("DS1302 WRITE:%d-%d-%d %d:%d:%d\n",
	 date->year, date->month, date->monthday, date->hour, date->minute, date->second);
}


void gc1302_init( cmp_rtc_date_s *date){
    DS1302_IIC_IO_INIT;

    if(22 > bcd2int(gc1302_read_byte(DS1302_READ_YEAR))){
        PR_DEBUG("NO TIME\n");
        gc1302_update(date);
    }

    DS1302_ClearWriteProtection();    // 清除写保护
    gc1302_write_byte(DS1302_WRITE_CHARGE, 0xA5);      // 使能充电(3.3-0.7)/8K

    date->year = bcd2int(gc1302_read_byte(DS1302_READ_YEAR)) + 2000;
    date->month = bcd2int(gc1302_read_byte(DS1302_READ_MONTH));
    date->monthday = bcd2int(gc1302_read_byte(DS1302_READ_DATE));
    //read_data[3] = bcd2int(gc1302_read_byte(DS1302_READ_WEEK));
    date->hour = bcd2int(gc1302_read_byte(DS1302_READ_HOUR));
    date->minute = bcd2int(gc1302_read_byte(DS1302_READ_MINUTE));
    date->second = bcd2int(gc1302_read_byte(DS1302_READ_SEC));

    PR_DEBUG("DS1302 READ:%d-%d-%d %d:%d:%d\n",
            date->year, date->month, date->monthday, date->hour, date->minute, date->second);
}

void gc1302_test(){
    unsigned char read_data[7] = {0};
    
	if(22 != bcd2int(gc1302_read_byte(DS1302_READ_YEAR))){
		PR_DEBUG("NO TIME\n");
		unsigned char year = 22;
		unsigned char month = 12;
		unsigned char day = 17;
		unsigned char week = 6;
		unsigned char hour = 3;
		unsigned char minute = 1;
		unsigned char sec = 20;
		
		DS1302_ClearWriteProtection();    // 清除写保护
		gc1302_write_byte(DS1302_WRITE_SEC, DS1302_STOP);	// 停止计时
		
		gc1302_write_byte(DS1302_WRITE_YEAR,int2bcd(year)); //2000+ DCD
		gc1302_write_byte(DS1302_WRITE_MONTH,int2bcd(month));
		gc1302_write_byte(DS1302_WRITE_DATE, int2bcd(day));    // 设置天
		gc1302_write_byte(DS1302_WRITE_WEEK, int2bcd(week));   
		gc1302_write_byte(DS1302_WRITE_HOUR, int2bcd(hour));    // 设置时
		gc1302_write_byte(DS1302_WRITE_MINUTE, int2bcd(minute));  // 设置分
		gc1302_write_byte(DS1302_WRITE_SEC, int2bcd(sec));     // 设置秒，并开始计时
	}
	 
	DS1302_ClearWriteProtection();    // 清除写保护
    gc1302_write_byte(DS1302_WRITE_CHARGE, 0xA5);      // 使能充电(3.3-0.7)/8K
    
    read_data[0] = bcd2int(gc1302_read_byte(DS1302_READ_YEAR));
    read_data[1] = bcd2int(gc1302_read_byte(DS1302_READ_MONTH));
    read_data[2] = bcd2int(gc1302_read_byte(DS1302_READ_DATE));
    read_data[3] = bcd2int(gc1302_read_byte(DS1302_READ_WEEK));
	read_data[4] = bcd2int(gc1302_read_byte(DS1302_READ_HOUR));
    read_data[5] = bcd2int(gc1302_read_byte(DS1302_READ_MINUTE));
    read_data[6] = bcd2int(gc1302_read_byte(DS1302_READ_SEC));
	
	PR_DEBUG("DS1302:%d %d %d %d %d %d %d\n",
	read_data[0],read_data[1],read_data[2],read_data[3],read_data[4],read_data[5],read_data[6]);
}
