Tested on Windows 7 64-bit with Microsoft Excel. Windows SDK Version 10.0.15063.0.
1.Copy all the dll files into C:\Windows\System32 for 64-bit dlls (and copy 32-bit dlls into C:\Windows\SysWOW64)
2.Open Command Prompt with Admin privileges and execute "dumpcap.exe -D" (without quotes)
3.Note down the windows address of lan interface the drives are connected to. (\Device\NPF_{X-X-X-X-X}
4.In Command Prompt (with Admin privileges), run "ycoe_ppm_debug.exe \Device\NPF_{X-X-X-X-X}" (replace X with the values found before.
5.Then, Open ycoe_interface.xlsm file
	a) Click Connect
	b) use the RPM & Revolutions section to change motor parameters
	c) Version contains the version number.
6.Enjoy! :)
