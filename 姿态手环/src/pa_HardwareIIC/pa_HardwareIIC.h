

#ifndef _pa_IIC_H
#define _pa_IIC_H
    typedef struct{
        unsigned short delay;
    }pa_IICSettingStruct;
    void pa_IIC_init();
    /** 
     * @brief 写iic
     * @param addr   器件地址
     * @param headByte   头字节
     * @param length   数据体长度，可以为0
     * @param data_t   数据体指针
     */
    void pa_IIC_writeLen(unsigned char addr, unsigned char headByte, unsigned char length, unsigned char* data_t,pa_IICSettingStruct pa_IICSettingStruct);
    /** 
     * @brief 读iic
     * @param addr   器件地址
     * @param headByte   头字节
     * @param length   数据体长度，最小为1
     * @param data_t   数据体指针
     */
    void pa_IIC_readLen(unsigned char addr, unsigned char headByte, unsigned char length, unsigned char* data_t,pa_IICSettingStruct pa_IICSettingStruct);
#endif