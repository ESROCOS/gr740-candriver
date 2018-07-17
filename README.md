# GR740 candriver

This package contains the **gr740\_candriver** component which initializes the infrastructure on RTMEMS5.1 to trasnmit/receive CAN messages.
It is meant to get **joint** commands, transform it into suitable CAN messages and then get new **joint** samples from the device.

## Caveats

Here we need a user\_init\_pre script to add an include path to the GR740 board-support-package.
Additionally, the misc/aadl-library/ocarina-components.aadl file has to be changed such that the CFLAGS contain *-DCONFIGURE_DRIVER_AMBAPP_GAISLER_CAN*.
Otherwise the gr740.rtems5\_posix code would not include the necessary driver.

After conversation with thanassis.tsiodras@esa.int there is now the possibility to assign to a function so called contextual parameters of type Taste\_directive.
With

> compiler_option:"..."

as value the CFLAGS of the build process (namely the orchestrator) can be extended.
