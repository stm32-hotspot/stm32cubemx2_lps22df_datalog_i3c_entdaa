# __Example: *lps22df_datalog_i3c_entdaa*__


How to use the part API with I3C bus and ENTDAA.

It illustrates it by getting the value of the temperature (in celsius) and of pressure (in hPascal) and displaying them on a terminal.


## __1. Detailed scenario__

__Initialization phase__: At main program start, the `mx_system_init()` function is called. It initializes the peripherals, nonvolatile memory (such as flash memory, NVM, or external memories), MPU regions (if applicable), the system clock, and the SysTick.

The application executes the following __example steps__:

__Step 1__: Initializes and enables temperature feature of LPS22DF

__Step 2__: Gets the values of the temperature (in celsius) and of pressure (in hPascal) and display them on a terminal

__End of example__: It is an endless example that loops infinitely on step 2

You can follow these execution steps in the terminal logs:
```text
[INFO] Step 1: LPS22DF sensor init completed
[INFO] Step 1: Enabling TEMP and PRESS features completed
[INFO] Step 2: TEMP 31 degC
[INFO] Step 2: PRESS 1081 hPa
```


## __2. Example configuration__


This example demonstrates the following driver:

- Part lps22df.c/.h
- Part lps22df/interfaces/i3c/lps22df_io.c/.h


In this example, the LPS22DF component is configured through the I3C IO operations defined under the folder interfaces/i3c.
Once the I3C is initialized, the LPS22DF can be initialized too through the call of lps22df_drv_init() API.
After this step, the MEMS sensor will be ready to provide the values of the temperature and pressure.


## __3. Hardware environment and setup__

### __3.1. Generic Setup__

This section describes the hardware setup principles that apply to any board.

### __3.2. Specific board setups__

<details>
<summary>On STM32C5 series.</summary>
  <summary>On board NUCLEO-C562RE.</summary>

  | Board connector | MCU pin | Signal name | ARDUINO <br> connector pin |
  | :-------------: | :-----: | :---------: | :------------------------: |
  |      CN5-10     |   PB6   |  I3C1_SCL   |  ARDUINO CONNECTOR - D15   |
  |      CN5-9      |   PB7   |  I3C1_SDA   |  ARDUINO CONNECTOR - D14   |
  - I3C Static ADDw = 0x5D (7-bit address).

</details>

## __4. Software setup__

To create a functional project, complete the following steps:
- Select the appropriate IoC2 file based on the combination of NUCLEO and X-NUCLEO boards. For example, use c562re_iks4a1_lps22df_i3c_entdaa.ioc2 for NUCLEO-C562RE and X-NUCLEO-IKS4A1.
- Open the IoC2 file with STM32CubeMX2.
- Select the preferred toolchain and generate the source code.
- Copy the main.c, main.h, example.c, and example.h files into the project folder of the generated code.
- Open the Integrated Development Environment (IDE), add the example.c and example.h files to the project.
- Add the USE_TRACE=1 to the global variables of the project.
- Compile the project.

## __5. Troubleshooting__

No specific debug tips.


## __6. See Also__

More information about LPS22DF part driver can be found in the [LPS22DF Part Driver](https://dev.st.com/stm32cube-docs/part-drivers-lps22df/1.1.0/en/index.html)

More information about the STM32 ecosystem can be found in the [STM32 MCU Developer Zone](https://www.st.com/content/st_com/en/stm32-mcu-developer-zone.html).


## __7. License__

Copyright (c) 2026 STMicroelectronics.

This software is licensed under terms that can be found in the LICENSE file in the root directory
of this software component.
If no LICENSE file comes with this software, it is provided AS-IS.


