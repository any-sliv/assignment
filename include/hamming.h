/**
 **********************************************************************************
 * File: [hamming.c]
 * Author: [Maciej Sliwinski]
 * Description: [Non-volatile memory storage component]
 *
 * This software is provided "as is" without any warranties.
 */

typedef unsigned char UInt8;
typedef UInt8 gpNvm_AttrId;
typedef UInt8 gpNvm_Result;

void calculateParityBits(UInt8 *data, UInt8 *parity);
UInt8 decodeAndCorrect(UInt8 *data, UInt8 *parity);
void storeParityExternally(UInt8 *parity);