/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use ��DMIC�� to record and ��DPWM�� to play audio, and DMIC and DPWM both use PDAM to transfer data.
	(1)	Using EVB-I94124ADI-NAU85L40B (DMIC version) to demo.
	(2)	DMIC and DPWM using PDMA to get and send data between module.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1) Connects EVB-I94124ADI-AUDIO_V1.0.
	(2) Connect speaker to DPWM_R on JP5 on EVB.
        (3) Compiled to execute.
        (4) Program test procedure -
            1. User can speak to DMIC and speaker will playback simultaneously.

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
