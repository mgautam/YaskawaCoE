Tested on Windows 7 64-bit with 64-bit Microsoft Excel
1.Copy all the dll files into into C:\Windows\System32
2.Open Command Prompt with Admin privileges and execute "dumpcap.exe -D" (without quotes)
3.Note down the windows address of lan interface the drives are connected to. (\Device\NPF_{X-X-X-X-X}
4.In Command Prompt (with Admin privileges), run "ycoe_ppm_debug.exe \Device\NPF_{X-X-X-X-X}" (replace X with the values found before.
5.Then, Open ycoe_interface.xlsm file
	a) Click Connect
	b) use the RPM & Revolutions section to change motor parameters
	c) Version contains the version number.
6.Enjoy! :)