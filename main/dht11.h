#ifndef DHT11_H
#define DHT11_H

/* 
    DHT11 DATA FORMAT (FROM DATASHEET):
        The DHT11 always sends **40 bits** in the following order:

            a. 8 bits — Integral Relative Humidity (RH)
            b. 8 bits — Decimal RH (usually 0 for DHT11)
            c. 8 bits — Integral Temperature
            d. 8 bits — Decimal Temperature (usually 0 for DHT11)
            e. 8 bits — Checksum = sum of the previous 4 bytes (lower 8 bits)
*/

/* 
    DHT11 COMMUNICATION PROCESS(FROM DATASHEET):
        1. Initiating START condition: MCU has to pull down (3.3V) VCC for at least 18ms and then pull up for 20-40µs

        2. DHT11 detects START: DHT11 pulls down VCC for 80µs acknowledging the START condition, then it pulls VCC high for 80µs 
                                as it prepares for data transmission

        3. Sending data: Data is sent by the DHT11 sensor in a pair. First is a low voltage level that lasts 50µs 
                         followed by the actual value of the bit(0 or 1)

                         Whether the value of the bit is 0 or 1 depends on the length that VCC is kept high. 

                         A value of 0 correponds to a voltage length of 26-28µs
                         A value of 1 correponds to a voltage length of 70µs

                         In this way, each bit is transmitted. When the DHT11 sends the last bit in the 40 bit sequence, 
                         it holds VCC low for 50µs and then automatically sets the sensor to the FREE status.

        An entire communication process is roughly 4ms
*/


void dht11_init(int gpio_pin);

void dht11_read(int gpio_pin, uint8_t* temperature, uint8_t* humidity);

#endif