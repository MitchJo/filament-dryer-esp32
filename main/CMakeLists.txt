set(COMPONENT_SRCS "filament.c")
set(COMPONENT_PRIV_REQUIRES "ble_manager" "storage_manager" "data_manager" "temperature_manager")

idf_component_register(
    SRCS "${COMPONENT_SRCS}"
    INCLUDE_DIRS "."
    PRIV_REQUIRES "${COMPONENT_PRIV_REQUIRES}"
)