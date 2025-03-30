#ifndef REGISTERS_HPP
#define REGISTERS_HPP

namespace ClvHd
{
enum ADS1293_Reg
{
    //Operation Mode Registers
    CONFIG_REG = 0x00, // Main Configuration

    //Input Channel Selection Registers
    FLEX_CH0_CN_REG = 0x01,  // Flex Routing Switch Control for Channel 1
    FLEX_CH1_CN_REG = 0x02,  // Flex Routing Switch Control for Channel 2
    FLEX_CH2_CN_REG = 0x03,  // Flex Routing Switch Control for Channel 3
    FLEX_PACE_CN_REG = 0x04, //Flex Routing Switch Control for Pace Channel
    FLEX_VBAT_CN_REG = 0x05, //Flex Routing Switch for Battery Monitoring

    //Lead-off Detect Control Registers
    LOD_CN_REG = 0x06,      //Lead-Off Detect Control
    LOD_EN_REG = 0x07,      //Lead-Off Detect Enable
    LOD_CURRENT_REG = 0x08, //Lead-Off Detect Current
    LOD_AC_CN_REG = 0x09,   //Lead-Off Detect AC Control

    //Common-Mode Detection and Right-Leg Drive Feedback Control Registers
    CMDET_EN_REG = 0x0A, //Common-Mode Detection Enable
    CMDET_CN_REG = 0x0B, //Common-Mode Detection Control
    RLD_CN_REG = 0x0C,   //Right-Leg Drive Control

    //Wilson Control Registers
    WILSON_EN1_REG = 0x0D, //Wilson Reference Input one Selection
    WILSON_EN2_REG = 0x0E, //Wilson Reference Input two Selection
    WILSON_EN3_REG = 0x0F, //Wilson Reference Input three Selection
    WILSON_CN_REG = 0x10,  //Wilson Reference Control

    //Reference Registers
    REF_CN_REG = 0x11, //Reference Control

    //Clock Control Registers
    OSC_CN_REG = 0x12, //Clock Source and Output Clock Control

    //AFE Control Registers
    AFE_RES_REG = 0x13,      //Analog Front End Frequency and Resolution
    AFE_SHDN_CN_REG = 0x14,  //Analog Front End Shutdown Control
    AFE_FAULT_CN_REG = 0x15, //Analog Front End Fault Detection Control
    AFE_PACE_CN_REG = 0x17,  //Analog Front End Pace Channel Control

    //Error Status Registers
    ERROR_LOD_REG = 0x18,    //Lead-Off Detect Error Status
    ERROR_STATUS_REG = 0x19, //Other Error Status
    ERROR_RANGE1_REG = 0x1a, //Channel 1 AFE Out-of-Range Error Status
    ERROR_RANGE2_REG = 0x1b, //Channel 2 AFE Out-of-Range Error Status
    ERROR_RANGE3_REG = 0x1c, //Channel 3 AFE Out-of-Range Error Status
    ERROR_SYNC_REG = 0x1d,   //Synchronization Error Status
    ERROR_MISC_REG = 0x1e,   //Miscellaneous Error Status

    //Digital Control Registers
    DIGO_STRENGTH_REG = 0x1f, //Digital Output Drive Strength Control
    R2_RATE_REG = 0x21,       //R2 Decimation Rate Control
    R3_RATE_CH0_REG = 0x22,   // R3 Decimation Rate Control for Channel 1
    R3_RATE_CH1_REG = 0x23,   // R3 Decimation Rate Control for Channel 2
    R3_RATE_CH2_REG = 0x24,   // R3 Decimation Rate Control for Channel 3
    R1_RATE_REG = 0x25,       // R1 Decimation Rate Control
    DIS_EFILTER_REG = 0x26,   //ECG Filter Disable
    DRDYB_SRC_REG = 0x27,     //Data Ready Pin Source Control
    SYNCB_CN_REG = 0x28,      //SYNCB in/out Pin Control
    MASK_DRDYB_REG = 0x29,    //Data Ready Pin Mask Control
    MASK_ERR_REG = 0x2a,      //Error Pin Mask Control
    RESERVED_0x2D_REG = 0x2d,

    ALARM_FILTER_REG = 0x2E, //Digital Filter for Analog Alarm Signals
    CH_CNFG_REG = 0x2F,      //Configure Channel for Loop Read Back Mode

    //Pace and ECG Data Read Back Registers
    DATA_STATUS_REG = 0x30,   //ECG and Pace Data Ready Status
    DATA_CH0_PACE_REG = 0x31, //Channel 1 Pace Data
    DATA_CH1_PACE_REG = 0x33, //Channel 2 Pace Data
    DATA_CH2_PACE_REG = 0x35, //Channel 3 Pace Data
    DATA_CH0_ECG_REG = 0x37,  //Channel 1 ECG Data
    DATA_CH1_ECG_REG = 0x3a,  //Channel 2 ECG Data
    DATA_CH2_ECG_REG = 0x3d,  //Channel 3 ECG Data
    REVID_REG = 0x40,         //Revision ID

};
}
#endif // REGISTERS_HPP
