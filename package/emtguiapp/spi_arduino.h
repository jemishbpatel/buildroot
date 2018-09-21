#ifndef SPI_ARDUINO
#define SPI_ARDUINO

/* TODO :Need to find runtime */
#define ARDUINO_IOC_MAGIC	0xf4

typedef struct _spi_arduino_config {
	unsigned int data_length;
	unsigned int number_of_slaves;
	unsigned int cards_per_slave;
	unsigned int total_data_length;
} spi_arduino_config;

typedef enum _piboard_gpio_dir {
        GPIO_DIR_OUTPUT,
        GPIO_DIR_INPUT
}piboard_gpio_dir;

typedef enum _piboard_gpio_value {
        GPIO_VALUE_RESET,
        GPIO_VALUE_SET
}piboard_gpio_value;


#define GPIO_A0_PIN		2
#define GPIO_A1_PIN	        3
#define GPIO_A2_PIN	        4
#define GPIO_A3_PIN	        17
#define GPIO_A4_PIN	        27
#define GPIO_A5_PIN	        22
#define GPIO_A6_PIN	        6
#define GPIO_A7_PIN	        13
#define GPIO_A8_PIN	        19
#define GPIO_A9_PIN	        26
#define GPIO_LEVEL_TRANSLATOR	5
#define GPIO_OUTPUT_ENABLE	21
#define GPIO_LATCH_DATA  	14
#define GPIO_CLEAR_DATA		15
#define GPIO_KNOWLEDGE_TRANSFER	20
#define GPIO_ARDUINO_RESET	16

#define ENABLE			1
#define DISABLE			0

#define ARDUINO_IOC_MAXNR               (3)
#define ARDUINO_SEND_DATA		_IOW(ARDUINO_IOC_MAGIC, 0, unsigned char*)
#define ARDUINO_CONFIGURE_DATA		_IOW(ARDUINO_IOC_MAGIC, 1, spi_arduino_config)
#define ARDUINO_STOP_DATA		_IOW(ARDUINO_IOC_MAGIC, 2, void*)
#define ARDUINO_CHANGE_DATALINES	_IOW(ARDUINO_IOC_MAGIC, 3, unsigned int)

#endif /* SPI_ARDUINO */
