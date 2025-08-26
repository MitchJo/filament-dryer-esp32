idf_component_register(SRCS "temperature_manager.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "storage_manager" "data_manager" "ble_manager"
                    REQUIRES
                        sht4x
                    )
