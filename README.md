# BMS

LTC6813 + MKE18F512 forever

### File Structure

- "path_to_Github"/

  - Github/

    - MKELibrary/
  
    - CanMessageStructs/
  
    - BMS_Code (Any repo being worked on)


## hello

the structure is as follows:

```
source\
|-- common.h:
|       includes and defines useful across all files. especially timing definitions
|-- FreeRTOSConfig.h:
|       configuration for this project's RTOS
|-- bmsCommand.*: 
|       command definitions, and functions for interacting with bmscommand_t
|-- main.*:
|       program flow
|-- transaction.*:
|       task which interacts directly with the SPI bus, and functions for pushing to it
|-- test\
    |-- testroutines.h:
    |       declarations of test routines, especially tasks
    |-- commandSend.cpp:
    |       basic LTC6811 interaction routines. see testroutines.h for details.
```
