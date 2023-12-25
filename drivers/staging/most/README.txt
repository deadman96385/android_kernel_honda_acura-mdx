	HOW TO BUILD

export ARCH=... (i.e. "arm" or "x86_64)
export CROSS_COMPILE=... (i.e. "arm-linux-gnueabihf-")

src_dir=.../src/most-driver
build_dir=.../most-driver-build-dir
kernel_dir=".../kernel-build-dir"
mkdir -p "$build_dir"
touch "$build_dir/Makefile"
make -C .../kernel-build-dir \
    src="$src_dir" \
    M="$build_dir" \
    CONFIG_HDM_DIM2=n \
    CONFIG_HDM_USB=m \
    modules
make -C "$kernel_dir" \
    src="$src_dir" \
    M="$build_dir" \
    CONFIG_HDM_DIM2=n \
    CONFIG_HDM_USB=m \
    INSTALL_MOD_PATH=.../output/ \
    modules_install

The .ko files will be copied into the path designated by the INSTALL_MOD_PATH.


	HOW TO LOAD THE MODULES

The modules must be loaded in the order below:

    mostcore.ko
    aim_network.ko
    aim_cdev.ko
    aim_mlb150.ko
    aim_syncsound.ko
    aim_isostream.ko
    hdm_usb.ko

Unloading of the modules is best done in reverse order.

For troubleshooting, the modules can be loaded with active dynamic debugging
option. I.e.:

    # insmod aim_mlb150.ko dyndbg=+pfml

The tracing can be completely or partially disabled later:

    # echo module aim_mlb150 func aim_rx_completion -p >/sys/kernel/debug/dynamic_debug/control


