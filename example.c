/**
  ******************************************************************************
  * file           : example.c
  * brief          : example program body
  ******************************************************************************
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "example.h"
#include "stm32_utils_i3c.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BROADCAST_ADDR       0x7EU    /*!< I3C broadcast address */
#define BROADCAST_RSTDAA     0x06U    /*!< Reset Dynamic Address Assignment (broadcast CCC) */
#define BROADCAST_DISEC      0x01U    /*!< Disable Events Command (broadcast CCC) */

/* Private function prototypes -----------------------------------------------*/
int32_t i3c_rstdaa(hal_i3c_handle_t *hi3c);
int32_t i3c_disec(hal_i3c_handle_t *hi3c);
int32_t i3c_set_bus_frequency(hal_i3c_handle_t *hi3c, uint32_t i3c_freq);

/* Private variables */
lps22df_object_t *pLps22df0; /* pointer referencing the LPS22DF object instance */
float gTempData; /* This variable store the temperature measurement (unit: deg C) */
float gPressData; /* This variable store the pression measurement (unit: hPa) */
hal_i3c_handle_t *pI3C;   /* Pointer referencing the I3C handle from the generated code */
uint32_t TargetCount = 0U; /* Number of targets detected and assigned during ENTDAA */

/**
  * @brief  Send a broadcast RSTDAA CCC to reset all targets dynamic addresses.
  * @param  hi3c the I3C handle
  * @retval 0 in case of success, an error code otherwise
  *
  */
int32_t i3c_rstdaa(hal_i3c_handle_t *hi3c)
{
  int32_t ret = 0;
  /* Transfer context aggregating the control buffer for the broadcast CCC frame */
  hal_i3c_transfer_ctx_t ctx;
  /* Control buffer (one word per broadcast CCC descriptor) computed by the HAL */
  uint32_t control_buffer[1];
  hal_i3c_ccc_desc_t desc;

  desc.tgt_addr = BROADCAST_ADDR;
  desc.ccc = BROADCAST_RSTDAA;
  desc.data_size_byte = 0U;
  desc.direction = HAL_I3C_DIRECTION_WRITE;

  if (HAL_I3C_CTRL_ResetTransferCtx(&ctx) != HAL_OK)
  {
    ret = -1;
  }
  else if (HAL_I3C_CTRL_InitTransferCtxTc(&ctx, control_buffer,
                                          HAL_I3C_GET_CTRL_BUFFER_SIZE_WORD(1U,
                                                                            HAL_I3C_CCC_BROADCAST_WITHOUT_DEFBYTE_RESTART)) != HAL_OK)
  {
    ret = -1;
  }
  else if (HAL_I3C_CTRL_BuildTransferCtxCCC(&ctx, &desc, 1U,
                                            HAL_I3C_CCC_BROADCAST_WITHOUT_DEFBYTE_RESTART) != HAL_OK)
  {
    ret = -1;
  }
  else if (HAL_I3C_CTRL_Transfer(hi3c, &ctx, 0x1000) != HAL_OK)
  {
    ret = -1;
  }

  return ret;
}

/**
  * @brief  Send a broadcast DISEC CCC to disable target events (e.g. Hot-Join).
  * @param  hi3c the I3C handle
  * @retval 0 in case of success, an error code otherwise
  *
  */
int32_t i3c_disec(hal_i3c_handle_t *hi3c)
{
  int32_t ret = 0;
  /* Transfer context aggregating the control/Tx buffers for the broadcast CCC frame */
  hal_i3c_transfer_ctx_t ctx;
  /* Control buffer (one word per broadcast CCC descriptor) computed by the HAL */
  uint32_t control_buffer[1];
  hal_i3c_ccc_desc_t desc;
  uint8_t disec_data[1] = {0x08U};

  desc.tgt_addr = BROADCAST_ADDR;
  desc.ccc = BROADCAST_DISEC;
  desc.data_size_byte = (uint32_t)1U;
  desc.direction = HAL_I3C_DIRECTION_WRITE;

  if (HAL_I3C_CTRL_ResetTransferCtx(&ctx) != HAL_OK)
  {
    ret = -1;
  }
  else if (HAL_I3C_CTRL_InitTransferCtxTc(&ctx, control_buffer, 2U) != HAL_OK)
  {
    ret = -1;
  }
  else if (HAL_I3C_CTRL_InitTransferCtxTx(&ctx, disec_data, 1U) != HAL_OK)
  {
    ret = -1;
  }
  else if (HAL_I3C_CTRL_BuildTransferCtxCCC(&ctx, &desc, 1U,
                                            HAL_I3C_CCC_BROADCAST_WITHOUT_DEFBYTE_RESTART) != HAL_OK)
  {
    ret = -1;
  }
  else if (HAL_I3C_CTRL_Transfer(hi3c, &ctx, 0x1000) != HAL_OK)
  {
    ret = -1;
  }

  return ret;
}

/**
  * @brief  Set the I3C bus frequency.
  * @param  hi3c the I3C handle
  * @param  i3c_freq the bus frequency to be set in Hz
  * @retval 0 in case of success, an error code otherwise
  *
  * @note   The controller timing is computed from the current I3C kernel clock and
  *         applied through the HAL controller configuration.
  */
int32_t i3c_set_bus_frequency(hal_i3c_handle_t *hi3c, uint32_t i3c_freq)
{
  int32_t ret = 0;
  stm32_utils_i3c_ctrl_timing_config_t timing_cfg;
  hal_i3c_ctrl_config_t ctrl_cfg;

  timing_cfg.clock_src_freq_hz   = HAL_RCC_I3C_GetKernelClkFreq((I3C_TypeDef *)((hal_i3c_t)(hi3c->instance)));
  timing_cfg.i3c_pp_freq_hz      = i3c_freq;
  timing_cfg.i2c_od_freq_hz      = 0U;
  timing_cfg.duty_cycle_purcent  = 50U;
  timing_cfg.wait_time           = STM32_UTILS_I3C_ACTIVITY_STATE_0;
  timing_cfg.bus_type            = STM32_UTILS_I3C_PURE_I3C_BUS;

  if (STM32_UTILS_I3C_CTRL_ComputeTiming(&timing_cfg, &ctrl_cfg.timing_reg0, &ctrl_cfg.timing_reg1)
      != STM32_UTILS_I3C_OK)
  {
    ret = -1;
  }
  else if (HAL_I3C_CTRL_SetConfig(hi3c, &ctrl_cfg) != HAL_OK)
  {
    ret = -1;
  }

  return ret;
}

/** ########## Step 1 ##########
  * The init of LPS22DF is triggered by the applicative code
  */
app_status_t app_init(void)
{
  app_status_t return_status = EXEC_STATUS_ERROR;
  hal_status_t hal_status;       /* Memorizes the HAL status of the I3C operations */
  hal_i3c_ctrl_config_t orig_config;
  hal_i3c_target_detection_status_t target_detection_status = HAL_I3C_TGT_DETECTED;
  uint64_t target_payload = 0U;

  /* Initialize the I3C peripheral */
  pI3C = mx_example_i3c_init();

  /* Save I3C original frequency settings */
  HAL_I3C_CTRL_GetConfig(pI3C, &orig_config);

  /* Set the bus speed to 1MHz for the dynamic addressing phase */
  if (i3c_set_bus_frequency(pI3C, 1000000U) != 0)
  {
    PRINTF("[ERROR] Step 1: I3C frequency set error\n");
    goto _app_init_exit;
  }
  /* Disable target events (e.g. Hot-Join) on the bus */
  if (i3c_disec(pI3C) != 0)
  {
    PRINTF("[ERROR] Step 1: DISEC broadcast error\n");
    goto _app_init_exit;
  }
  /* Reset all targets dynamic addresses */
  if (i3c_rstdaa(pI3C) != 0)
  {
    PRINTF("[ERROR] Step 1: RSTDAA broadcast error\n");
    goto _app_init_exit;
  }
  HAL_Delay(100);
  /* Try to wake-up LIS2DUXS12 from deep power down */
  hal_status = HAL_I3C_CTRL_PoolForDeviceI3cReady(pI3C, 0x19, 0x1000);
  if (hal_status != HAL_OK)
  {
    /* Error occurred during DAA process initiation. */
    PRINTF("[ERROR] Step 1: HAL_I3C_CTRL_PoolForDeviceI3cReady error\n");
    goto _app_init_exit;
  }
  HAL_Delay(25);
  /* Perform ENTDAA */
  do
  {
    target_payload = 0U;
    /* Initiate Dynamic Address Assignment (DAA) process for the controller */
    hal_status = HAL_I3C_CTRL_DynAddrAssign(pI3C, &target_payload, HAL_I3C_DYN_ADDR_ONLY_ENTDAA, &target_detection_status, 0x1000);
    if (hal_status != HAL_OK)
    {
      /* Error occurred during DAA process initiation. */
      PRINTF("[ERROR] Step 1: ENTDAA error\n");
      goto _app_init_exit;

    }
    if (target_detection_status == HAL_I3C_TGT_DETECTED)
    {
      PRINTF("[INFO] Step 1: Found target_payload= 0x%" PRIx64 ".\n", target_payload);
      if (target_payload == LPS22DF_0_I3C_DCR_BCR_PID)
      {
        /* Assign the dynamic address selected by the user to the LSM6DSV16X */
        hal_status = HAL_I3C_CTRL_SetDynAddr(pI3C, LPS22DF_0_I3C_DYNAMIC_ADDRESS);
        if (hal_status != HAL_OK)
        {
          /* Error occurred during DAA process initiation. */
          PRINTF("[ERROR] Step 1: SetDynAddr error\n");
          goto _app_init_exit;
        }
      }
      else
      {
        /* Assign random dynamic addresses to the other I3C devices on the expansion board */
        hal_status = HAL_I3C_CTRL_SetDynAddr(pI3C, (0x10 + TargetCount));
        if (hal_status != HAL_OK)
        {
          /* Error occurred during DAA process initiation. */
          PRINTF("[ERROR] Step 1: SetDynAddr error. TargetCount= %" PRIu32 ".\n", TargetCount);
          goto _app_init_exit;
        }
      }
      TargetCount++;
    }
  } while (target_detection_status == HAL_I3C_TGT_DETECTED);


  PRINTF("[INFO] Step 1: DAA initiation COMPLETED. Found %" PRIu32 " devices\n", TargetCount);
  /* Restore the bus frequency to the original nominal value */
  if (HAL_I3C_CTRL_SetConfig(pI3C, &orig_config) != HAL_OK)
  {
    PRINTF("[ERROR] Step 1: I3C frequency set config error\n");
    goto _app_init_exit;
  }
  /* Retrieve and store the LPS22DF object pointer */
  pLps22df0 = mx_lps22df_0_getobject();
  /* Initialize the LPS22DF device 0 */
  if (lps22df_drv_init(pLps22df0, MX_LPS22DF) != 0)
  {
    PRINTF("[ERROR] Step 1: LPS22DF sensor init error\r\n");
    goto _app_init_exit;
  }
  PRINTF("[INFO] Step 1: LPS22DF sensor init completed\r\n");
  /* LPS22DF device 0: enable the temperature and the pressure feature */
  if (lps22df_drv_enable(pLps22df0) != 0)
  {
    PRINTF("[ERROR] Enabling the TEMP and PRESS feature failed\r\n");
    goto _app_init_exit;
  }
  PRINTF("[INFO] Enabling TEMP and PRESS feature SUCCESS\r\n");
  return_status = EXEC_STATUS_INIT_OK;

_app_init_exit:
  return return_status;
}

/**
  * ########## Step 2 ##########
  * Gets the values from LPS22DF.
  * The values are displayed on the terminal.
  * output: EXEC_STATUS_OK if OK, EXEC_STATUS_ERROR in case of error
  */
app_status_t app_process(void)
{
  app_status_t return_status = EXEC_STATUS_ERROR;
  /* LPS22DF device 0: get the temperature value and print it */
  if (lps22df_drv_get_temperature(pLps22df0, &gTempData) != 0)
  {
    PRINTF("[ERROR] Step 2: Reading temperature error\r\n");
    goto _app_process_exit;
  }
  PRINTF("[INFO] Step 2: TEMP: %" PRIi32 " degC\n", (int32_t)gTempData);
  /* LPS22DF device 0: get the pressure value and print it */
  if (lps22df_drv_get_pressure(pLps22df0, &gPressData) != 0)
  {
    PRINTF("[ERROR] Step 2: Reading pressure error\n");
    goto _app_process_exit;
  }
  PRINTF("[INFO] Step 2: PRESS: %" PRIi32 " hPa\n", (int32_t)gPressData);
  return_status = EXEC_STATUS_OK;

_app_process_exit:
  return return_status;
}

/** ########## Step 3 ##########
  * In this example, app_deinit is never called and is provided as a reference only.
  */
app_status_t app_deinit(void)
{
  lps22df_drv_deinit(pLps22df0);

  return EXEC_STATUS_OK;
}
