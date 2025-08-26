idf_component_register(SRCS "storage_manager.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES "ble_manager" "data_manager"
                    REQUIRES
                        nvs_flash
                        json
                    )
