/**
 **********************************************************************************
 * File: [gpNvm.h]
 * Author: [Maciej Sliwinski]
 * Description: [Non-volatile memory storage component]
 *
 * Copyright (c) 2024 Maciej Sliwinski. All rights reserved.
 * 
 * This software is provided "as is" without any warranties.
 */

typedef unsigned char UInt8;
typedef UInt8 gpNvm_AttrId;
typedef UInt8 gpNvm_Result;

gpNvm_Result gpNvm_GetAttribute(gpNvm_AttrId attrId, UInt8* pLength, UInt8* pValue);

gpNvm_Result gpNvm_SetAttribute(gpNvm_AttrId attrId, UInt8 length, UInt8* pValue);