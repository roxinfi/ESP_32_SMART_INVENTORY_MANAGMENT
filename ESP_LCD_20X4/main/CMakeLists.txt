idf_component_register(
    SRCS 
        "main.c"
        "item_sorting.c"
        "shelf_manager.c"
        "lcd_20x4_driver.c"    # ← make sure this is here!
        "BMX_20.c"
        "sensors.c"
        INCLUDE_DIRS 
        "."                   # or wherever your .h files live
)
