/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use TIMER0 to trigger EADC to get conversion result.
	(1)	Single end input from channel 2.
	(2)	Differential input from channel 2 and 3.
	(3)	Using TIMER0 to trigger EADC to get 6 conversions result.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connects to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect PA2(CH2) to the nodes to be measured.
	(3)	Compiled to execute.
	(4)	Program test procedure -
		1.	Input ��1�� to start single end demo.	Input others key will end the demo.
		2.	COM port will Show conversion result like this�� Conversion result of channel �K(result)��.
		3.	The program will return to procedure '1.'. User can input any others key to end demo.

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
