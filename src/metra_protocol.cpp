#include "metra_protocol.h"
#include <iostream>
#include <assert.h>
#include <cstring>

#define SOH 0xff
#define ETX 0x03
#define DLE 0x10

unsigned char tab_crc[]={
                0x00,0x5E,0xBC,0xE2,0x61,0x3F,0xDD,0x83,
                0xc2,0x9c,0x7e,0x20,0xa3,0xfd,0x1f,0x41,
                0x9d,0xc3,0x21,0x7f,0xfc,0xa2,0x40,0x1e,
                0x5f,0x01,0xe3,0xbd,0x3e,0x60,0x82,0xdc,
                0x23,0x7D,0x9F,0xC1,0x42,0x1C,0xFE,0xA0,
                0xE1,0xBF,0x5D,0x03,0x80,0xDE,0x3C,0x62,
                0xBE,0xE0,0x02,0x5C,0xDF,0x81,0x63,0x3D,
                0x7C,0x22,0xC0,0x9E,0x1D,0x43,0xA1,0xFF,
                0x46,0x18,0xFA,0xA4,0x27,0x79,0x9B,0xC5,
                0x84,0xDA,0x38,0x66,0xE5,0xBB,0x59,0x07,
                0xDB,0x85,0x67,0x39,0xBA,0xE4,0x06,0x58,
                0x19,0x47,0xA5,0xFB,0x78,0x26,0xC4,0x9A,
                0x65,0x3B,0xD9,0x87,0x04,0x5A,0xB8,0xE6,
                0xA7,0xF9,0x1B,0x45,0xC6,0x98,0x7A,0x24,
                0xF8,0xA6,0x44,0x1A,0x99,0xC7,0x25,0x7B,
                0x3A,0x64,0x86,0xD8,0x5B,0x05,0xE7,0xB9,
                0x8C,0xD2,0x30,0x6E,0xED,0xB3,0x51,0x0F,
                0x4E,0x10,0xF2,0xAC,0x2F,0x71,0x93,0xCD,
                0x11,0x4F,0xAD,0xF3,0x70,0x2E,0xCC,0x92,
                0xD3,0x8D,0x6F,0x31,0xB2,0xEC,0x0E,0x50,
                0xAF,0xF1,0x13,0x4D,0xCE,0x90,0x72,0x2C,
                0x6D,0x33,0xD1,0x8F,0x0C,0x52,0xB0,0xEE,
                0x32,0x6C,0x8E,0xD0,0x53,0x0D,0xEF,0xB1,
                0xF0,0xAE,0x4C,0x12,0x91,0xCF,0x2D,0x73,
                0xCA,0x94,0x76,0x28,0xAB,0xF5,0x17,0x49,
                0x08,0x56,0xB4,0xEA,0x69,0x37,0xD5,0x8B,
                0x57,0x09,0xEB,0xB5,0x36,0x68,0x8A,0xD4,
                0x95,0xCB,0x29,0x77,0xF4,0xAA,0x48,0x16,
                0xE9,0xB7,0x55,0x0B,0x88,0xD6,0x34,0x6A,
                0x2B,0x75,0x97,0xC9,0x4A,0x14,0xF6,0xA8,
                0x74,0x2A,0xC8,0x96,0x15,0x4B,0xA9,0xF7,
                0xB6,0xE8,0x0A,0x54,0xD7,0x89,0x6B,0x35
     };
//-----------------------------------------------------------------------------

MetraProtocol::MetraProtocol( unsigned int size_value)
{
    max_size   = size_value;

    packet = new unsigned char[size_value];

    start_byte_received = false;
	packet_received = false;
}
//-----------------------------------------------------------------------------

MetraProtocol::~MetraProtocol()
{
    delete[] packet;
}

//-----------------------------------------------------------------------------
// Вычисление контрольной суммы
//-----------------------------------------------------------------------------
unsigned char MetraProtocol::calc_crc (unsigned char size, unsigned char *values)
{
    unsigned char crc = 0;
    for (auto i = 0; i < size; i++)
    {
       crc = tab_crc[ *values ^ crc ];
       values++;
    }
    return crc;
}

//-----------------------------------------------------------------------------
// Замена служебных символов альтернативной комбинацией
//-----------------------------------------------------------------------------
bool MetraProtocol::add_staff(void)
{
    unsigned char value;
    bool b_result = true;

    for( auto indx = 1; indx < size; indx++ )
    {
        value = packet[indx];
        if( (value == SOH) || (value == ETX) || (value == DLE) )
        {  
            if( ++size >= max_size )
            {
                b_result = false;
                break;
            }
             
            for( unsigned int i = size - 1; i > indx; i--) 
            {
                packet[i] = packet[i-1];
            }

            packet[indx] = DLE;
            packet[indx+1] = ~value;
        }
    }

    return b_result;
}
//------------------------------------------------------------------------------
// Замена комбинацией DLE ~X
//------------------------------------------------------------------------------
void MetraProtocol::del_staff(void)
{
    for(auto indx = 1; indx < size; indx++ )
    {
        if( packet[indx] == DLE )
        {
            size--;
            for( unsigned int i = indx; i < size; i++) 
            {
                packet[i] = packet[i+1];
            }
            packet[indx] =~packet[indx] ;
        }
    }
}

//------------------------------------------------------------------------------
// Извлечь пакет
//------------------------------------------------------------------------------
bool MetraProtocol::get_packet (s_mpr_param *param, unsigned int max_data_size){

    unsigned char result = false;

	if(packet_received){
	    packet_received = false;
        del_staff();

        if ( calc_crc(size, packet) == 0 && packet[1] ==param->rx_address && ((size-4) <= max_data_size))
        {
                param->rx_address = packet[1];
                param->tx_address = packet[2];
                param->command    = packet[3];
                param->data_size  = size - 4;

                if( param->data_size ){
                    memmove( param->data, (packet+4),  param->data_size);
                }

                result = true;
        }
	}

	return  result;
}

//------------------------------------------------------------------------------
// Сформировать пакет
//------------------------------------------------------------------------------
bool MetraProtocol::set_packet (s_mpr_param *param){
    unsigned char result = false;

    if( max_size > (param->data_size +4)  ){
        packet[0]  = SOH;

        packet[1] = param->rx_address;
        packet[2] = param->tx_address;
        packet[3] = param->command;

        if( param->data_size ){
            memmove( (packet+4), param->data,   param->data_size);
        }

        size =  param->data_size + 4;
        packet[size] = calc_crc(size, packet);

        size++;

        if( add_staff() ){
            packet[size] = ETX;
            ++size;
            result = false;
        }

        start_byte_received = false;
        packet_received = false;
    }

    return  result;
}

//------------------------------------------------------------------------------
// Побайтный прием данных
//------------------------------------------------------------------------------
bool MetraProtocol::received_byte(const unsigned char &data)
{   
    if( start_byte_received )
    {
        if (data == ETX)                            // конец пакета?
        {
            start_byte_received = false;

            size = indx_pack + 1;

            packet_received = true;

        }
        else
        {
            indx_pack++;
            if (indx_pack >= max_size )              // число принятых байт превышает допустимый предел?
            {
                start_byte_received = true;             // начало нового пакета
            }
            else
            {
                packet[indx_pack] = data;                // поместить полученнный байт в буфер приема
            }
        }
    }
    else
    {
        if( data == SOH )                           // новый пакет?
        {
            start_byte_received = true;             // начало нового пакета
            indx_pack = 0;                          // настраиваем буфер приема на начало
            packet[indx_pack] = SOH;
        }
    }// end if( start_byte_received )

    return packet_received;
}
//------------------------------------------------------------------------------
