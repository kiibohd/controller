#
# Sources
#
c_lib = [
    'delay.c',
    'entropy.c',
    'gpio.c',
    'periodic.c',
    'storage.c',
    'time.c',
    'utf8.c',
]

c_clang = [
    'clang.c',
]

c_arm_cortex = [
    'arm_cortex.c',
]

c_kinetis = [
    c_lib,
    c_arm_cortex,
    'kinetis.c',
]

c_atsam = [
    c_lib,
    c_arm_cortex,
    'sam.c',
    'Lib/ASF/common/services/clock/sam4s/sysclk.c',
    'Lib/ASF/common/utils/interrupt/interrupt_sam_nvic.c',
    'Lib/ASF/sam/drivers/efc/efc.c',
    'Lib/ASF/sam/drivers/pio/pio.c',
    'Lib/ASF/sam/drivers/pmc/pmc.c',
    'Lib/ASF/sam/utils/cmsis/sam4s/source/templates/system_sam4s.c',
    'Lib/ASF/sam/services/flash_efc/flash_efc.c',
]

c_nrf5 = [
    c_lib,
    c_arm_cortex,
    'Lib/nRF5_SDK/components/boards/boards.c',
    'Lib/nRF5_SDK/components/libraries/atomic/nrf_atomic.c',
    'Lib/nRF5_SDK/components/libraries/balloc/nrf_balloc.c',
    'Lib/nRF5_SDK/components/libraries/experimental_section_vars/nrf_section_iter.c',
    'Lib/nRF5_SDK/components/libraries/log/src/nrf_log_backend_rtt.c',
    'Lib/nRF5_SDK/components/libraries/log/src/nrf_log_backend_serial.c',
    'Lib/nRF5_SDK/components/libraries/log/src/nrf_log_backend_uart.c',
    'Lib/nRF5_SDK/components/libraries/log/src/nrf_log_default_backends.c',
    'Lib/nRF5_SDK/components/libraries/log/src/nrf_log_frontend.c',
    'Lib/nRF5_SDK/components/libraries/log/src/nrf_log_str_formatter.c',
    'Lib/nRF5_SDK/components/libraries/memobj/nrf_memobj.c',
    'Lib/nRF5_SDK/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c',
    'Lib/nRF5_SDK/components/libraries/ringbuf/nrf_ringbuf.c',
    'Lib/nRF5_SDK/components/libraries/strerror/nrf_strerror.c',
    'Lib/nRF5_SDK/components/libraries/util/app_error.c',
    'Lib/nRF5_SDK/components/libraries/util/app_error_handler_gcc.c',
    'Lib/nRF5_SDK/components/libraries/util/app_error_weak.c',
    'Lib/nRF5_SDK/components/libraries/util/app_util_platform.c',
    'Lib/nRF5_SDK/components/libraries/util/nrf_assert.c',
    'Lib/nRF5_SDK/external/fprintf/nrf_fprintf.c',
    'Lib/nRF5_SDK/external/fprintf/nrf_fprintf_format.c',
    'Lib/nRF5_SDK/external/segger_rtt/SEGGER_RTT.c',
    'Lib/nRF5_SDK/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c',
    'Lib/nRF5_SDK/external/segger_rtt/SEGGER_RTT_printf.c',
    'Lib/nRF5_SDK/integration/nrfx/legacy/nrf_drv_ppi.c',
    'Lib/nRF5_SDK/integration/nrfx/legacy/nrf_drv_uart.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_ppi.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_saadc.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_timer.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_uart.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_uarte.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/prs/nrfx_prs.c',
    'Lib/nRF5_SDK/modules/nrfx/mdk/gcc_startup_nrf52.S',
    'Lib/nRF5_SDK/modules/nrfx/mdk/system_nrf52.c',
    'Lib/nRF5_SDK/modules/nrfx/soc/nrfx_atomic.c',
    'Lib/nRF5_SDK/components/drivers_nrf/nrf_soc_nosd/nrf_nvic.c',
    'Lib/nRF5_SDK/components/drivers_nrf/nrf_soc_nosd/nrf_soc.c',
    'Lib/nRF5_SDK/components/libraries/bsp/bsp.c',
    'Lib/nRF5_SDK/components/libraries/button/app_button.c',
    'Lib/nRF5_SDK/components/libraries/gfx/nrf_gfx.c',
    'Lib/nRF5_SDK/components/libraries/queue/nrf_queue.c',
    'Lib/nRF5_SDK/components/libraries/scheduler/app_scheduler.c',
    'Lib/nRF5_SDK/components/libraries/spi_mngr/nrf_spi_mngr.c',
    'Lib/nRF5_SDK/components/libraries/timer/app_timer.c',
    'Lib/nRF5_SDK/components/libraries/util/nrf_assert.c',
    'Lib/nRF5_SDK/external/fprintf/nrf_fprintf.c',
    'Lib/nRF5_SDK/external/fprintf/nrf_fprintf_format.c',
    'Lib/nRF5_SDK/external/segger_rtt/SEGGER_RTT.c',
    'Lib/nRF5_SDK/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c',
    'Lib/nRF5_SDK/external/segger_rtt/SEGGER_RTT_printf.c',
    'Lib/nRF5_SDK/external/thedotfactory_fonts/orkney24pts.c',
    'Lib/nRF5_SDK/external/thedotfactory_fonts/orkney8pts.c',
    'Lib/nRF5_SDK/integration/nrfx/legacy/nrf_drv_clock.c',
    'Lib/nRF5_SDK/integration/nrfx/legacy/nrf_drv_spi.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_clock.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_gpiote.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_spi.c',
    'Lib/nRF5_SDK/modules/nrfx/drivers/src/nrfx_spim.c',
]

c_host = [
    c_lib,
    'host.c',
]


#
# Subdirectories
#
subdir('ld')
