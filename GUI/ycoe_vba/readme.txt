Tested on Windows 7 64-bit with Microsoft Excel. Windows SDK Version 10.0.15063.0.
1.Copy all the dll files into C:\Windows\System32 for 64-bit dlls (and copy 32-bit dlls into C:\Windows\SysWOW64)
2.Then, Open ycoe_interface.xlsm file
	a) Click Connect
	b) Change motion parameters
	c) Click SetDrv# to set corresponding drive
	d) Verify each drive by clicking Get Drv# buttons
	e) Click Send PosArray button to send and start motion	
	f) Status: 1=Success -1=Failed
3.Enjoy! :)
