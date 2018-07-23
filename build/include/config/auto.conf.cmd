deps_config := \
	/home/nobotro/esp/esp-idf/components/app_trace/Kconfig \
	/home/nobotro/esp/esp-idf/components/aws_iot/Kconfig \
	/home/nobotro/esp/esp-idf/components/bt/Kconfig \
	/home/nobotro/esp/esp-idf/components/driver/Kconfig \
	/home/nobotro/esp/esp-idf/components/esp32/Kconfig \
	/home/nobotro/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/nobotro/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/nobotro/esp/esp-idf/components/ethernet/Kconfig \
	/home/nobotro/esp/esp-idf/components/fatfs/Kconfig \
	/home/nobotro/esp/esp-idf/components/freertos/Kconfig \
	/home/nobotro/esp/esp-idf/components/heap/Kconfig \
	/home/nobotro/esp/esp-idf/components/libsodium/Kconfig \
	/home/nobotro/esp/esp-idf/components/log/Kconfig \
	/home/nobotro/esp/esp-idf/components/lwip/Kconfig \
	/home/nobotro/esp/esp-idf/components/mbedtls/Kconfig \
	/home/nobotro/esp/esp-idf/components/openssl/Kconfig \
	/home/nobotro/esp/esp-idf/components/pthread/Kconfig \
	/home/nobotro/esp/esp-idf/components/spi_flash/Kconfig \
	/home/nobotro/esp/esp-idf/components/spiffs/Kconfig \
	/home/nobotro/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/nobotro/esp/esp-idf/components/vfs/Kconfig \
	/home/nobotro/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/nobotro/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/nobotro/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/nobotro/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/nobotro/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
