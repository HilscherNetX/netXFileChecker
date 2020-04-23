
#include "netXFileChecker.h"
#include "Hil_FileHeaderV3.h"


#define STR(tok) #tok

static struct
{
  char* szName;
  uint32_t ulCode;
} s_atCookieLookupTable[] =
{

		{STR(HIL_FILE_HEADER_FIRMWARE_COOKIE    ),HIL_FILE_HEADER_FIRMWARE_COOKIE    },
		{STR(HIL_FILE_HEADER_FIRMWARE_8_COOKIE  ),HIL_FILE_HEADER_FIRMWARE_8_COOKIE  },
		{STR(HIL_FILE_HEADER_FIRMWARE_16_COOKIE ),HIL_FILE_HEADER_FIRMWARE_16_COOKIE },
		{STR(HIL_FILE_HEADER_FIRMWARE_32_COOKIE ),HIL_FILE_HEADER_FIRMWARE_32_COOKIE },
		{STR(HIL_FILE_HEADER_FIRMWARE_NXI_COOKIE),HIL_FILE_HEADER_FIRMWARE_NXI_COOKIE},
		{STR(HIL_FILE_HEADER_FIRMWARE_NXE_COOKIE),HIL_FILE_HEADER_FIRMWARE_NXE_COOKIE},
		{STR(HIL_FILE_HEADER_FIRMWARE_MXF_COOKIE),HIL_FILE_HEADER_FIRMWARE_MXF_COOKIE},
		{STR(HIL_FILE_HEADER_FIRMWARE_NAI_COOKIE),HIL_FILE_HEADER_FIRMWARE_NAI_COOKIE},
		{STR(HIL_FILE_HEADER_FIRMWARE_NAE_COOKIE),HIL_FILE_HEADER_FIRMWARE_NAE_COOKIE},

        {STR(HIL_HBOOT_STANDARD_COOKIE),                      HIL_HBOOT_STANDARD_COOKIE},
		{STR(HIL_HBOOT_NO_AUTO_DETECTION_SQI_FLASHES_COOKIE), HIL_HBOOT_NO_AUTO_DETECTION_SQI_FLASHES_COOKIE},
		{STR(HIL_HBOOT_ALTERNATIVE_IMAGE_COOKIE),             HIL_HBOOT_ALTERNATIVE_IMAGE_COOKIE},

  	  /* This always must be the last entry in this table */
	    { "Unknown code", 0xffffffff },

};








static struct
{
  char* szName;
  uint16_t ulCode;
} s_atComClassLookupTable[] =
{
{STR(HIL_COMM_CLASS_UNDEFINED                     ),HIL_COMM_CLASS_UNDEFINED                     },
{STR(HIL_COMM_CLASS_UNCLASSIFIABLE                ),HIL_COMM_CLASS_UNCLASSIFIABLE                },
{STR(HIL_COMM_CLASS_MASTER                        ),HIL_COMM_CLASS_MASTER                        },
{STR(HIL_COMM_CLASS_SLAVE                         ),HIL_COMM_CLASS_SLAVE                         },
{STR(HIL_COMM_CLASS_SCANNER                       ),HIL_COMM_CLASS_SCANNER                       },
{STR(HIL_COMM_CLASS_ADAPTER                       ),HIL_COMM_CLASS_ADAPTER                       },
{STR(HIL_COMM_CLASS_MESSAGING                     ),HIL_COMM_CLASS_MESSAGING                     },
{STR(HIL_COMM_CLASS_CLIENT                        ),HIL_COMM_CLASS_CLIENT                        },
{STR(HIL_COMM_CLASS_SERVER                        ),HIL_COMM_CLASS_SERVER                        },
{STR(HIL_COMM_CLASS_IO_CONTROLLER                 ),HIL_COMM_CLASS_IO_CONTROLLER                 },
{STR(HIL_COMM_CLASS_IO_DEVICE                     ),HIL_COMM_CLASS_IO_DEVICE                     },
{STR(HIL_COMM_CLASS_IO_SUPERVISOR                 ),HIL_COMM_CLASS_IO_SUPERVISOR                 },
{STR(HIL_COMM_CLASS_GATEWAY                       ),HIL_COMM_CLASS_GATEWAY                       },
{STR(HIL_COMM_CLASS_MONITOR                       ),HIL_COMM_CLASS_MONITOR                       },
{STR(HIL_COMM_CLASS_PRODUCER                      ),HIL_COMM_CLASS_PRODUCER                      },
{STR(HIL_COMM_CLASS_CONSUMER                      ),HIL_COMM_CLASS_CONSUMER                      },
{STR(HIL_COMM_CLASS_SWITCH                        ),HIL_COMM_CLASS_SWITCH                        },
{STR(HIL_COMM_CLASS_HUB                           ),HIL_COMM_CLASS_HUB                           },
{STR(HIL_COMM_CLASS_COMBI                         ),HIL_COMM_CLASS_COMBI                         },
{STR(HIL_COMM_CLASS_MANAGING_NODE                 ),HIL_COMM_CLASS_MANAGING_NODE                 },
{STR(HIL_COMM_CLASS_CONTROLLED_NODE               ),HIL_COMM_CLASS_CONTROLLED_NODE               },
{STR(HIL_COMM_CLASS_PLC                           ),HIL_COMM_CLASS_PLC                           },
{STR(HIL_COMM_CLASS_HMI                           ),HIL_COMM_CLASS_HMI                           },
{STR(HIL_COMM_CLASS_ITEM_SERVER                   ),HIL_COMM_CLASS_ITEM_SERVER                   },
{STR(HIL_COMM_CLASS_SCADA                         ),HIL_COMM_CLASS_SCADA                         },
{STR(HIL_COMM_CLASS_IO_CONTROLLER_SYSTEMREDUNDANCY),HIL_COMM_CLASS_IO_CONTROLLER_SYSTEMREDUNDANCY},
{STR(HIL_COMM_CLASS_IO_DEVICE_SYSTEMREDUNDANCY    ),HIL_COMM_CLASS_IO_DEVICE_SYSTEMREDUNDANCY    },
  /* This always must be the last entry in this table */
  { "Unknown code", 0xffff },

};




static struct
{
  char* szName;
  uint16_t ulCode;
} s_atProtClassLookupTable[] =
{
{STR(HIL_PROT_CLASS_UNDEFINED         ), HIL_PROT_CLASS_UNDEFINED         },
{STR(HIL_PROT_CLASS_3964R             ), HIL_PROT_CLASS_3964R             },
{STR(HIL_PROT_CLASS_ASINTERFACE       ), HIL_PROT_CLASS_ASINTERFACE       },
{STR(HIL_PROT_CLASS_ASCII             ), HIL_PROT_CLASS_ASCII             },
{STR(HIL_PROT_CLASS_CANOPEN           ), HIL_PROT_CLASS_CANOPEN           },
{STR(HIL_PROT_CLASS_CCLINK            ), HIL_PROT_CLASS_CCLINK            },
{STR(HIL_PROT_CLASS_COMPONET          ), HIL_PROT_CLASS_COMPONET          },
{STR(HIL_PROT_CLASS_CONTROLNET        ), HIL_PROT_CLASS_CONTROLNET        },
{STR(HIL_PROT_CLASS_DEVICENET         ), HIL_PROT_CLASS_DEVICENET         },
{STR(HIL_PROT_CLASS_ETHERCAT          ), HIL_PROT_CLASS_ETHERCAT          },
{STR(HIL_PROT_CLASS_ETHERNET_IP       ), HIL_PROT_CLASS_ETHERNET_IP       },
{STR(HIL_PROT_CLASS_FOUNDATION_FB     ), HIL_PROT_CLASS_FOUNDATION_FB     },
{STR(HIL_PROT_CLASS_FL_NET            ), HIL_PROT_CLASS_FL_NET            },
{STR(HIL_PROT_CLASS_INTERBUS          ), HIL_PROT_CLASS_INTERBUS          },
{STR(HIL_PROT_CLASS_IO_LINK           ), HIL_PROT_CLASS_IO_LINK           },
{STR(HIL_PROT_CLASS_LON               ), HIL_PROT_CLASS_LON               },
{STR(HIL_PROT_CLASS_MODBUS_PLUS       ), HIL_PROT_CLASS_MODBUS_PLUS       },
{STR(HIL_PROT_CLASS_MODBUS_RTU        ), HIL_PROT_CLASS_MODBUS_RTU        },
{STR(HIL_PROT_CLASS_OPEN_MODBUS_TCP   ), HIL_PROT_CLASS_OPEN_MODBUS_TCP   },
{STR(HIL_PROT_CLASS_PROFIBUS_DP       ), HIL_PROT_CLASS_PROFIBUS_DP       },
{STR(HIL_PROT_CLASS_PROFIBUS_MPI      ), HIL_PROT_CLASS_PROFIBUS_MPI      },
{STR(HIL_PROT_CLASS_PROFINET_IO       ), HIL_PROT_CLASS_PROFINET_IO       },
{STR(HIL_PROT_CLASS_RK512             ), HIL_PROT_CLASS_RK512             },
{STR(HIL_PROT_CLASS_SERCOS_II         ), HIL_PROT_CLASS_SERCOS_II         },
{STR(HIL_PROT_CLASS_SERCOS_III        ), HIL_PROT_CLASS_SERCOS_III        },
{STR(HIL_PROT_CLASS_TCP_IP_UDP_IP     ), HIL_PROT_CLASS_TCP_IP_UDP_IP     },
{STR(HIL_PROT_CLASS_POWERLINK         ), HIL_PROT_CLASS_POWERLINK         },
{STR(HIL_PROT_CLASS_HART              ), HIL_PROT_CLASS_HART              },
{STR(HIL_PROT_CLASS_COMBI             ), HIL_PROT_CLASS_COMBI             },
{STR(HIL_PROT_CLASS_PROG_GATEWAY      ), HIL_PROT_CLASS_PROG_GATEWAY      },
{STR(HIL_PROT_CLASS_PROG_SERIAL       ), HIL_PROT_CLASS_PROG_SERIAL       },
{STR(HIL_PROT_CLASS_PLC_CODESYS       ), HIL_PROT_CLASS_PLC_CODESYS       },
{STR(HIL_PROT_CLASS_PLC_PROCONOS      ), HIL_PROT_CLASS_PLC_PROCONOS      },
{STR(HIL_PROT_CLASS_PLC_IBH_S7        ), HIL_PROT_CLASS_PLC_IBH_S7        },
{STR(HIL_PROT_CLASS_PLC_ISAGRAF       ), HIL_PROT_CLASS_PLC_ISAGRAF       },
{STR(HIL_PROT_CLASS_VISU_QVIS         ), HIL_PROT_CLASS_VISU_QVIS         },
{STR(HIL_PROT_CLASS_ETHERNET          ), HIL_PROT_CLASS_ETHERNET          },
{STR(HIL_PROT_CLASS_RFC1006           ), HIL_PROT_CLASS_RFC1006           },
{STR(HIL_PROT_CLASS_DF1               ), HIL_PROT_CLASS_DF1               },
{STR(HIL_PROT_CLASS_VARAN             ), HIL_PROT_CLASS_VARAN             },
{STR(HIL_PROT_CLASS_3S_PLC_HANDLER    ), HIL_PROT_CLASS_3S_PLC_HANDLER    },
{STR(HIL_PROT_CLASS_ATVISE            ), HIL_PROT_CLASS_ATVISE            },
{STR(HIL_PROT_CLASS_MQTT              ), HIL_PROT_CLASS_MQTT              },
{STR(HIL_PROT_CLASS_OPCUA             ), HIL_PROT_CLASS_OPCUA             },
{STR(HIL_PROT_CLASS_CCLINK_IE_BASIC   ), HIL_PROT_CLASS_CCLINK_IE_BASIC   },
{STR(HIL_PROT_CLASS_CCLINK_IE_FIELD   ), HIL_PROT_CLASS_CCLINK_IE_FIELD   },
{STR(HIL_PROT_CLASS_NETWORK_SERVICES  ), HIL_PROT_CLASS_NETWORK_SERVICES  },
{STR(HIL_PROT_CLASS_NETPROXY          ), HIL_PROT_CLASS_NETPROXY          },
  /* This always must be the last entry in this table */
  { "Unknown code", 0xffff },

};




static struct
{
  char* szName;
  uint16_t ulCode;
} s_atDevClassLookupTable[] =
{
{STR(HIL_HW_DEV_CLASS_UNDEFINED                         ), HIL_HW_DEV_CLASS_UNDEFINED                         },
{STR(HIL_HW_DEV_CLASS_UNCLASSIFIABLE                    ), HIL_HW_DEV_CLASS_UNCLASSIFIABLE                    },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_500                     ), HIL_HW_DEV_CLASS_CHIP_NETX_500                     },
{STR(HIL_HW_DEV_CLASS_CIFX                              ), HIL_HW_DEV_CLASS_CIFX                              },
{STR(HIL_HW_DEV_CLASS_COMX_100                          ), HIL_HW_DEV_CLASS_COMX_100                          },
{STR(HIL_HW_DEV_CLASS_EVA_BOARD                         ), HIL_HW_DEV_CLASS_EVA_BOARD                         },
{STR(HIL_HW_DEV_CLASS_NETDIMM                           ), HIL_HW_DEV_CLASS_NETDIMM                           },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_100                     ), HIL_HW_DEV_CLASS_CHIP_NETX_100                     },
{STR(HIL_HW_DEV_CLASS_NETX_HMI                          ), HIL_HW_DEV_CLASS_NETX_HMI                          },
{STR(HIL_HW_DEV_CLASS_NETIO_50                          ), HIL_HW_DEV_CLASS_NETIO_50                          },
{STR(HIL_HW_DEV_CLASS_NETIO_100                         ), HIL_HW_DEV_CLASS_NETIO_100                         },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_50                      ), HIL_HW_DEV_CLASS_CHIP_NETX_50                      },
{STR(HIL_HW_DEV_CLASS_GW_NETPAC                         ), HIL_HW_DEV_CLASS_GW_NETPAC                         },
{STR(HIL_HW_DEV_CLASS_GW_NETTAP                         ), HIL_HW_DEV_CLASS_GW_NETTAP                         },
{STR(HIL_HW_DEV_CLASS_NETSTICK                          ), HIL_HW_DEV_CLASS_NETSTICK                          },
{STR(HIL_HW_DEV_CLASS_NETANALYZER                       ), HIL_HW_DEV_CLASS_NETANALYZER                       },
{STR(HIL_HW_DEV_CLASS_NETSWITCH                         ), HIL_HW_DEV_CLASS_NETSWITCH                         },
{STR(HIL_HW_DEV_CLASS_NETLINK                           ), HIL_HW_DEV_CLASS_NETLINK                           },
{STR(HIL_HW_DEV_CLASS_NETIC_50                          ), HIL_HW_DEV_CLASS_NETIC_50                          },
{STR(HIL_HW_DEV_CLASS_NPLC_C100                         ), HIL_HW_DEV_CLASS_NPLC_C100                         },
{STR(HIL_HW_DEV_CLASS_NPLC_M100                         ), HIL_HW_DEV_CLASS_NPLC_M100                         },
{STR(HIL_HW_DEV_CLASS_GW_NETTAP_50                      ), HIL_HW_DEV_CLASS_GW_NETTAP_50                      },
{STR(HIL_HW_DEV_CLASS_NETBRICK                          ), HIL_HW_DEV_CLASS_NETBRICK                          },
{STR(HIL_HW_DEV_CLASS_NPLC_T100                         ), HIL_HW_DEV_CLASS_NPLC_T100                         },
{STR(HIL_HW_DEV_CLASS_NETLINK_PROXY                     ), HIL_HW_DEV_CLASS_NETLINK_PROXY                     },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_10                      ), HIL_HW_DEV_CLASS_CHIP_NETX_10                      },
{STR(HIL_HW_DEV_CLASS_NETJACK_10                        ), HIL_HW_DEV_CLASS_NETJACK_10                        },
{STR(HIL_HW_DEV_CLASS_NETJACK_50                        ), HIL_HW_DEV_CLASS_NETJACK_50                        },
{STR(HIL_HW_DEV_CLASS_NETJACK_100                       ), HIL_HW_DEV_CLASS_NETJACK_100                       },
{STR(HIL_HW_DEV_CLASS_NETJACK_500                       ), HIL_HW_DEV_CLASS_NETJACK_500                       },
{STR(HIL_HW_DEV_CLASS_NETLINK_10_USB                    ), HIL_HW_DEV_CLASS_NETLINK_10_USB                    },
{STR(HIL_HW_DEV_CLASS_COMX_10                           ), HIL_HW_DEV_CLASS_COMX_10                           },
{STR(HIL_HW_DEV_CLASS_NETIC_10                          ), HIL_HW_DEV_CLASS_NETIC_10                          },
{STR(HIL_HW_DEV_CLASS_COMX_50                           ), HIL_HW_DEV_CLASS_COMX_50                           },
{STR(HIL_HW_DEV_CLASS_NETRAPID_10                       ), HIL_HW_DEV_CLASS_NETRAPID_10                       },
{STR(HIL_HW_DEV_CLASS_NETRAPID_50                       ), HIL_HW_DEV_CLASS_NETRAPID_50                       },
{STR(HIL_HW_DEV_CLASS_NETSCADA_T51                      ), HIL_HW_DEV_CLASS_NETSCADA_T51                      },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_51                      ), HIL_HW_DEV_CLASS_CHIP_NETX_51                      },
{STR(HIL_HW_DEV_CLASS_NETRAPID_51                       ), HIL_HW_DEV_CLASS_NETRAPID_51                       },
{STR(HIL_HW_DEV_CLASS_GW_EU5C                           ), HIL_HW_DEV_CLASS_GW_EU5C                           },
{STR(HIL_HW_DEV_CLASS_NETSCADA_T50                      ), HIL_HW_DEV_CLASS_NETSCADA_T50                      },
{STR(HIL_HW_DEV_CLASS_NETSMART_50                       ), HIL_HW_DEV_CLASS_NETSMART_50                       },
{STR(HIL_HW_DEV_CLASS_IOLINK_GW_51                      ), HIL_HW_DEV_CLASS_IOLINK_GW_51                      },
{STR(HIL_HW_DEV_CLASS_NETHMI_B500                       ), HIL_HW_DEV_CLASS_NETHMI_B500                       },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_52                      ), HIL_HW_DEV_CLASS_CHIP_NETX_52                      },
{STR(HIL_HW_DEV_CLASS_COMX_51                           ), HIL_HW_DEV_CLASS_COMX_51                           },
{STR(HIL_HW_DEV_CLASS_NETJACK_51                        ), HIL_HW_DEV_CLASS_NETJACK_51                        },
{STR(HIL_HW_DEV_CLASS_NETHOST_T100                      ), HIL_HW_DEV_CLASS_NETHOST_T100                      },
{STR(HIL_HW_DEV_CLASS_NETSCOPE_C100                     ), HIL_HW_DEV_CLASS_NETSCOPE_C100                     },
{STR(HIL_HW_DEV_CLASS_NETRAPID_52                       ), HIL_HW_DEV_CLASS_NETRAPID_52                       },
{STR(HIL_HW_DEV_CLASS_NETSMART_T51                      ), HIL_HW_DEV_CLASS_NETSMART_T51                      },
{STR(HIL_HW_DEV_CLASS_NETSCADA_T52                      ), HIL_HW_DEV_CLASS_NETSCADA_T52                      },
{STR(HIL_HW_DEV_CLASS_NETSAFETY_51                      ), HIL_HW_DEV_CLASS_NETSAFETY_51                      },
{STR(HIL_HW_DEV_CLASS_NETSAFETY_52                      ), HIL_HW_DEV_CLASS_NETSAFETY_52                      },
{STR(HIL_HW_DEV_CLASS_NETPLC_J500                       ), HIL_HW_DEV_CLASS_NETPLC_J500                       },
{STR(HIL_HW_DEV_CLASS_NETIC_52                          ), HIL_HW_DEV_CLASS_NETIC_52                          },
{STR(HIL_HW_DEV_CLASS_GW_NETTAP_151                     ), HIL_HW_DEV_CLASS_GW_NETTAP_151                     },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM                ), HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM                },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_90_COM                  ), HIL_HW_DEV_CLASS_CHIP_NETX_90_COM                  },
{STR(HIL_HW_DEV_CLASS_NETRAPID_51_IO                    ), HIL_HW_DEV_CLASS_NETRAPID_51_IO                    },
{STR(HIL_HW_DEV_CLASS_GW_NETTAP_151_CCIES               ), HIL_HW_DEV_CLASS_GW_NETTAP_151_CCIES               },
{STR(HIL_HW_DEV_CLASS_CIFX_CCIES                        ), HIL_HW_DEV_CLASS_CIFX_CCIES                        },
{STR(HIL_HW_DEV_CLASS_COMX_51_CCIES                     ), HIL_HW_DEV_CLASS_COMX_51_CCIES                     },
{STR(HIL_HW_DEV_CLASS_NIOT_E_NPEX_BP52_IO               ), HIL_HW_DEV_CLASS_NIOT_E_NPEX_BP52_IO               },
{STR(HIL_HW_DEV_CLASS_NIOT_E_NPEX_BP52_IOL              ), HIL_HW_DEV_CLASS_NIOT_E_NPEX_BP52_IOL              },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM_HIFSDR         ), HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM_HIFSDR         },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM_SDR            ), HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM_SDR            },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_90_COM_HIFSDR           ), HIL_HW_DEV_CLASS_CHIP_NETX_90_COM_HIFSDR           },
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_A), HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_A},
{STR(HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_C), HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_C},
{STR(HIL_HW_DEV_CLASS_NETFIELD_COM                      ), HIL_HW_DEV_CLASS_NETFIELD_COM                      },
{STR(HIL_HW_DEV_CLASS_NETFIELD_APP_FOR_NETFIELD_COM     ), HIL_HW_DEV_CLASS_NETFIELD_APP_FOR_NETFIELD_COM     },
  /* This always must be the last entry in this table */
  { "Unknown code", 0xffff },

};




static struct
{
  char* szName;
  uint16_t ulCode;
} s_atChipTypeLookupTable[] =
{
{STR(HIL_DEV_CHIP_TYPE_UNKNOWN     ), HIL_DEV_CHIP_TYPE_UNKNOWN     },
{STR(HIL_DEV_CHIP_TYPE_NETX500     ), HIL_DEV_CHIP_TYPE_NETX500     },
{STR(HIL_DEV_CHIP_TYPE_NETX100     ), HIL_DEV_CHIP_TYPE_NETX100     },
{STR(HIL_DEV_CHIP_TYPE_NETX50      ), HIL_DEV_CHIP_TYPE_NETX50      },
{STR(HIL_DEV_CHIP_TYPE_NETX10      ), HIL_DEV_CHIP_TYPE_NETX10      },
{STR(HIL_DEV_CHIP_TYPE_NETX51      ), HIL_DEV_CHIP_TYPE_NETX51      },
{STR(HIL_DEV_CHIP_TYPE_NETX52      ), HIL_DEV_CHIP_TYPE_NETX52      },
{STR(HIL_DEV_CHIP_TYPE_NETX4000    ), HIL_DEV_CHIP_TYPE_NETX4000    },
{STR(HIL_DEV_CHIP_TYPE_NETX4100    ), HIL_DEV_CHIP_TYPE_NETX4100    },
{STR(HIL_DEV_CHIP_TYPE_NETX90      ), HIL_DEV_CHIP_TYPE_NETX90      },
{STR(HIL_DEV_CHIP_TYPE_NETIOL      ), HIL_DEV_CHIP_TYPE_NETIOL      },
{STR(HIL_DEV_CHIP_TYPE_NETXXXL_MPW ), HIL_DEV_CHIP_TYPE_NETXXXL_MPW },
/* This always must be the last entry in this table */
{ "Unknown code", 0xffff },

};


static struct
{
  char* szName;
  uint32_t ulCode;
} s_atDevTypeLookupTable[] =
{
{STR(HIL_SRC_DEVICE_TYPE_PAR_FLASH_SRAM),HIL_SRC_DEVICE_TYPE_PAR_FLASH_SRAM},
{STR(HIL_SRC_DEVICE_TYPE_SER_FLASH     ),HIL_SRC_DEVICE_TYPE_SER_FLASH     },
{STR(HIL_SRC_DEVICE_TYPE_EEPROM        ),HIL_SRC_DEVICE_TYPE_EEPROM        },
{STR(HIL_SRC_DEVICE_TYPE_SD_MMC        ),HIL_SRC_DEVICE_TYPE_SD_MMC        },
{STR(HIL_SRC_DEVICE_TYPE_DPM           ),HIL_SRC_DEVICE_TYPE_DPM           },
{STR(HIL_SRC_DEVICE_TYPE_DPM_EXT       ),HIL_SRC_DEVICE_TYPE_DPM_EXT       },
{STR(HIL_SRC_DEVICE_TYPE_PAR_FLASH_EXT ),HIL_SRC_DEVICE_TYPE_PAR_FLASH_EXT },
/* This always must be the last entry in this table */
{ "Unknown code", 0xffff },

};





extern char* LookupCode(uint32_t ulCmd)
{
  int i;
  for (i = 0; i < sizeof(s_atCookieLookupTable) / sizeof(s_atCookieLookupTable[0]); i++)
  {
    if (s_atCookieLookupTable[i].ulCode == ulCmd)
    {
      return s_atCookieLookupTable[i].szName;
    }
  }

  /* Comamnd not in lookup table: Return last entry of table */
  return s_atCookieLookupTable[i - 1].szName;
}



extern char* LookupComClassCode(uint16_t ulCmd)
{
  int i;
  for (i = 0; i < sizeof(s_atComClassLookupTable) / sizeof(s_atComClassLookupTable[0]); i++)
  {
    if (s_atComClassLookupTable[i].ulCode == ulCmd)
    {
      return s_atComClassLookupTable[i].szName;
    }
  }

  /* Comamnd not in lookup table: Return last entry of table */
  return s_atComClassLookupTable[i - 1].szName;
}


extern char* LookupProtClassCode(uint16_t ulCmd)
{
  int i;
  for (i = 0; i < sizeof(s_atProtClassLookupTable) / sizeof(s_atProtClassLookupTable[0]); i++)
  {
    if (s_atProtClassLookupTable[i].ulCode == ulCmd)
    {
      return s_atProtClassLookupTable[i].szName;
    }
  }

  /* Comamnd not in lookup table: Return last entry of table */
  return s_atProtClassLookupTable[i - 1].szName;
}



extern char* LookupDevClassCode(uint16_t ulCmd)
{
  int i;
  for (i = 0; i < sizeof(s_atDevClassLookupTable) / sizeof(s_atDevClassLookupTable[0]); i++)
  {
    if (s_atDevClassLookupTable[i].ulCode == ulCmd)
    {
      return s_atDevClassLookupTable[i].szName;
    }
  }

  /* Comamnd not in lookup table: Return last entry of table */
  return s_atDevClassLookupTable[i - 1].szName;
}



extern char* LookupDevTypeCode(uint32_t ulCmd)
{
  int i;
  for (i = 0; i < sizeof(s_atDevTypeLookupTable) / sizeof(s_atDevTypeLookupTable[0]); i++)
  {
    if (s_atDevTypeLookupTable[i].ulCode == ulCmd)
    {
      return s_atDevTypeLookupTable[i].szName;
    }
  }

  /* Comamnd not in lookup table: Return last entry of table */
  return s_atDevTypeLookupTable[i - 1].szName;
}



extern char* LookupChipTypeCode(uint8_t ulCmd)
{
  int i;
  for (i = 0; i < sizeof(s_atChipTypeLookupTable) / sizeof(s_atChipTypeLookupTable[0]); i++)
  {
    if (s_atChipTypeLookupTable[i].ulCode == ulCmd)
    {
      return s_atChipTypeLookupTable[i].szName;
    }
  }

  /* Comamnd not in lookup table: Return last entry of table */
  return s_atChipTypeLookupTable[i - 1].szName;
}


