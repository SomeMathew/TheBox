# To compile:

## Local Setup 
On the makefile 
	Set TOOLROOT to point to the local directory for the AVR tool.
	Set ISPPORT to the COM or dev/ttyXXX for which the AVR is connected.

## Compilation
Compile:
	`make`

Upload:
	`make upload ISPPORT=dev/ttyXXX`  OR for windows `ISPPORT=COMxxx`

Clean
	`make clean`
	

Any new .c and .h file should be added to the Makefile on the USRSRCS line.


Please only commit changes that are required for everyone, you should not
commit a change to the TOOLROOT or ISPPORT.
	
	
