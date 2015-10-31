# SGX-hardware list
This is a list of hardware which supports Intel SGX - Software Guard Extensions. 

## Desktop

The CPU and the motherboard BIOS must support SGX. SGX is turned off by default and must enabled via MSR.IA32\_Feature\_Control.SGX\_Enable. Only the BIOS can make changes to the IA32\_Feature\_Control.

### Desktop CPUs

According to the [Product Change Notification](http://qdms.intel.com/dm/i.aspx/5A160770-FC47-47A0-BF8A-062540456F0A/PCN114074-00.pdf) from October 1, 2015. Only the CPUs with the following S-Spec Numbers are able to execute SGX instructions:

| Marketing Name | Processor# | Post-Conversion S-Spec |
|----------------|------------|------------------------|
| Intel® Core    | i7-6700K   | SR2L0     |
| Intel® Core    | i5-6400T   | SR2L1     |
| Intel® Core    | i7-6700    | SR2L2     |
| Intel® Core    | i7-6700T   | SR2L3     |
| Intel® Core    | i5-6600K   | SR2L4     |
| Intel® Core    | i5-6600    | SR2L5     |
| Intel® Core    | i5-6500    | SR2L6     |
| Intel® Core    | i5-6400    | SR2L7     |
| Intel® Core    | i5-6500T   | SR2L8     |
| Intel® Core    | i5-6600T   | SR2L9     |
| Intel® Xeon®   | E3-1280 v5    | SR2LC  |
| Intel® Xeon®   | E3-1240 v5    | SR2LD  |
| Intel® Xeon®   | E3-1230 v5    | SR2LE  |
| Intel® Xeon®   | E3-1270 v5    | SR2LF  |
| Intel® Xeon®   | E3-1220 v5    | SR2LG  |
| Intel® Xeon®   | E3-1260L v5   | SR2LH  |
| Intel® Xeon®   | E3-1225 v5    | SR2LJ  |
| Intel® Xeon®   | E3-1275 v5    | SR2LK  |
| Intel® Xeon®   | E3-1245 v5    | SR2LL  |
| Intel® Xeon®   | E3-1235L v5   | SR2LM  |
| Intel® Xeon®   | E3-1240L v5   | SR2LN  |

### Desktop Mainboards

No desktop BIOS with SGX support known.


## Notebooks

| Vendor | Model | Driver Type | Version | Supported OS | Source | Release Date |
|--------------|-------|-------------|---------|--------------|--------|----|
| Dell  |  OptiPlex, Latitude, Tablet, Precision | Dell System Inventory Agent |  1.4.2.0, A60  | VISTA (32/64-bit), Windows 7 (32/64-bit) ,Windows 8 (32/64-bit),Windows 8.1 (32/64-bit), Windows 10(32/64 -bit) | [dell.com](http://www.dell.com/support/home/us/en/19/Drivers/DriversDetails?driverId=433D7) | 27 Jul 2015 |
| Dell  | Inspiron 11 i3153, Inspiron 11 i3158, Inspiron 13 i7353, Inspiron 13 i7359, Inspiron 15 i7568 | Chipset Driver   | 1.0.26920. 1393, A00  | Windows 8.1 64-bit, Windows 10 64-bit | [dell.com](http://www.dell.com/support/home/us/en/19/Drivers/DriversDetails?driverId=F84XC) | 10 Sep 2015 |
| Dell | Inspiron 15 7000 Series 7559 | BIOS | 1.0.3 | Windows/DOS | [dell.com](http://www.dell.com/support/home/us/en/19/Drivers/DriversDetails?driverId=3FWTM) | 09 Oct 2015 |
| HP | HP 240 G4 Notebook PC, HP 246 G4 Notebook PC, HP Notebook 14g, HP Notebook 14q  | Chipset | 1.0.26448. 1348 REV: A PASS: 5 | Microsoft Windows 10 64 | [ftp.hp.com](http://ftp.hp.com/pub/softpaq/sp72001-72500/sp72357.html) | 07 Aug 2015 |
| HP | ENVY 15 Notebook PC | Chipset | 1.0.26805. 1389 REV: A PASS: 4 | Windows 10 64bit, Windows 7 Prof. | [ftp.hp.com](http://ftp.hp.com/pub/softpaq/sp72001-72500/sp72491.html) | 17 Aug 2015 |
| HP | HP Spectre x2 Detachable Ultrabook PC | Chipset | 1.0.26805. 1389 REV: A PASS: 9 | Windows 10 64 | [ftp.hp.com](http://ftp.hp.com/pub/softpaq/sp72501-73000/sp72989.html) | 02 Oct 2015 |

## Test SGX

You can check if SGX is enabled on you system with the test_sgx.c. Just compile and run it.
If the output is something like:
```
Extended feature bits (EAX=07H, ECX=0H)
eax: 0 ebx: 29c6fbf ecx: 0 edx: 0
sgx available: 1

CPUID Leaf 12H, Sub-Leaf 0 of Intel SGX Capabilities (EAX=12H,ECX=0)
eax: 0 ebx: 0 ecx: 0 edx: 0
sgx 1 supported: 0
sgx 2 supported: 0
MaxEnclaveSize_Not64: 0
MaxEnclaveSize_64: 0
```
SGX is available for your CPU but not enabled in BIOS.

## Contribution

Add more hardware to this list via pull requests or simply via issues. 
