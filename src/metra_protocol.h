/*
* Протокол связи с условным названием "Метра"
*
* служебные символы SOH (0xff) - начало пакета, ETH (0x03) - конец пакета, DLE (0x10) - символ замены служебных символов внутри пакета
*
*/
#pragma once

// формат пакета
struct s_mpr_param
{
    unsigned char *data;          // указатель на данные
    unsigned int   data_size;     // размер данных
    unsigned char  tx_address,    // адрес получателя
                   rx_address,    // адрес получателя
                   command;       // команда
};

// Протокол "Метра"
class MetraProtocol
{
private:
    bool           start_byte_received;
    bool           packet_received;
    unsigned int   indx_pack;

    bool add_staff(void);      // добавить служебные символы
    void del_staff(void);      // удалить служебные символы
public:
    unsigned char  *packet;    // сырые данные полученные или подготовленные для отправки
    unsigned int   size,
                   max_size;

    unsigned long        bytes_received,
                         bytes_sended,
                         packets_sended,
                         packets_received;

    // извлечь формат пакета из сырых полученных данных
    bool get_packet (s_mpr_param *param, unsigned int max_data_size);
    // сформировать данные для отправки согласно формата пакета
    bool set_packet (s_mpr_param *param);
    // обработка полученного байта с проверкой условия получения начала и конца пакета
    bool received_byte   (const unsigned char &data);
    //расчет контрольной суммы
    unsigned char calc_crc(unsigned char size, unsigned char *values);

    MetraProtocol(unsigned int size_value);
    ~MetraProtocol();
};


