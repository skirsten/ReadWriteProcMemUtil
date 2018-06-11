## ReadWriteProcMemUtil: `ReadProcMem` and `WriteProcMem`

Dual executable to manipulate other processes memory from the commandline or (python) scripts. [Download](https://github.com/skirsten/ReadWriteProcMemUtil/releases)

```
[Read|Write]ProcMem.exe [-v|--verbose|-h|--help] exe|PID|Window "BaseModule: baseoffset offset1 offset2 offsetn" data commands...

  -v | --verbose         Helpful for debugging and to comprehend what is happening
  -h | --help            Print this help
  exe | PID | Window     e.g. FarCry5.exe or 15573 or "Minesweeper"

  address definition: Must be a string in the format specified above
    The first baseoffset directly offsets the BaseModule and all following offsets apply to the address pointed by the previous address.

"CT_FC5.dll: 0x139830 0x8 0x10"
    Here can be observed that first 0x139830 gets added to the module address following a dereferencing.
    The resulting address gets offset by 0x8 and dereferenced again and offset by 0x10 one last time (no dereference here because that only applies to the previous address):
    With the -v flag the following address computation can be observed.
        Note: because we did not supply any data no writes are happening and we can freely experiment.

Found module "CT_FC5.dll" at 0x7ffde4710000
addr = "CT_FC5.dll" + 0x139830 = 0x7ffde4710000 + 0x139830
addr = ["CT_FC5.dll" + 0x139830] + 0x8 = 0x14a8a4823c0 + 0x8
addr = [["CT_FC5.dll" + 0x139830] + 0x8] + 0x10 = 0x14a89903ff0 + 0x10
Final address: 0x14a89904000


Write data commands:
  -f | --float   writes single precision float e.g. -f 22.5 or -f Nan or -f inf or -f 0X1.BC70A3D70A3D7P+6
  -d | --double  writes double precision float e.g. same as float
  -i[8|16|32|64] | --integer[8|16|32|64]   writes 8|16|32|64 bit integer respectively e.g. -i32 -123 or -i16 0xcaffe or -i64 -0xe
  -u[8|16|32|64] | --unsigned[8|16|32|64]  writes 8|16|32|64 bit unsigned integer respectively e.g. same as integer
  -s n  skips n bytes e.g. -f 1 -s 4 -f 0 skips 1 float in the middle
  
Read data commands:
  -f | --float   reads single precision float
  -d | --double  reads double precision float
  -i[8|16|32|64] | --integer[8|16|32|64]   reads 8|16|32|64 bit integer respectively
  -u[8|16|32|64] | --unsigned[8|16|32|64]  reads 8|16|32|64 bit unsigned integer respectively
  -s n  skips n bytes e.g. -f -s 4 -f skips 1 float in the middle
```
