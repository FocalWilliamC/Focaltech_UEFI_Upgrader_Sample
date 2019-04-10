/** @file
  This sample application bases on HelloWorld PCD setting 
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <UsbUpgradeApp.h>

//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//


/**
GetCheckSum
**/
UINT8
GetCheckSum(
IN UINT8  *pData,
IN UINT8  DataNum
)
{
	UINT8	 i=0;
	UINT8 	Checksum=0;
	for(i=0; i<DataNum; i++)
	{
		Checksum ^= pData[i];
	}
	Checksum++;
	return Checksum;
}

/*
EFI_STATUS 
OpenShellProtocol( 
EFI_SHELL_PROTOCOL            **gEfiShellProtocol 
)
{    
	EFI_STATUS                      Status;    
	Status = gBS->OpenProtocol(	gImageHandle,            
							&gEfiShellProtocolGuid,            
							(VOID **)gEfiShellProtocol,            
							gImageHandle,            
							NULL,            
							EFI_OPEN_PROTOCOL_GET_PROTOCOL            
							);    
	if (EFI_ERROR(Status)) {    
		//    
		// Search for the shell protocol    
		//        
		Status = gBS->LocateProtocol(&gEfiShellProtocolGuid,           
								NULL,                
								(VOID **)gEfiShellProtocol                
								);        
		if (EFI_ERROR(Status)) {
			gEfiShellProtocol = NULL;        
			Print(L"ERROR : LocateProtocol fail.\n");
		}  
	}  
	
	return Status;
}
*/

EFI_STATUS 
GetFileIo(
IN OUT EFI_FILE_PROTOCOL** Root
)
{
	EFI_STATUS  Status = 0;
    	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
    	Status = gBS->LocateProtocol(
           	 &gEfiSimpleFileSystemProtocolGuid,
           	 NULL,
           	 (VOID**)&SimpleFileSystem
    	);
    	if (EFI_ERROR(Status)) {
    	 //Î´ÕÒµ½EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
    	    return Status;
    	}
    	Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, Root);
    	return Status;
}


EFI_STATUS
OpenBinFile(
IN OUT USB_MOUSE_DEV 	*usbMouseDev
)
{
	EFI_STATUS  Status = 0;    
	EFI_FILE_PROTOCOL *SystemFile = 0;    
	CHAR16 Buf[17];    UINTN BufSize = 10;    
//	UINT8  Buf[65];    UINTN BufSize = 64;    
	Status = GetFileIo(&(usbMouseDev->FileRoot));    
    	if (EFI_ERROR(Status)) {
		 	
    	 	Print(L"GetFileIo error (%r)", Status);
    	   	return Status;
    	}
		
	Status = usbMouseDev->FileRoot->Open(usbMouseDev->FileRoot,                 
								& SystemFile,            
								(CHAR16*)L"abc.txt",             
								EFI_FILE_MODE_READ ,            
								0
								);    
	if (EFI_ERROR(Status)) {
		 	
    	 	Print(L"Open error (%r)", Status);
    	   	return Status;
    	}
	
			
	Status = SystemFile -> Read ( SystemFile,            
							&BufSize,            
							Buf            
							);    
	if(!EFI_ERROR(Status)){        
		Buf[BufSize] = 0;        
		Print(L"Buf [%s], BufSize(%d)\n", Buf, BufSize );    
	} 
	else
	{
    	 	Print(L"Read error (%r)", Status);
    	   	return Status;
    	}
	
	Status = SystemFile->Close(SystemFile);

	
	return  Status;
}

/**
UsbFirstSiuCmd
**/
EFI_STATUS
EFIAPI
UsbFirstSiuCmd(
IN OUT USB_MOUSE_DEV 	**usbMouseDev
)
{
	UINT32  		Status;
	UINT32     	rtnDataLen,  i;
	EFI_STATUS  	Result = EFI_SUCCESS;
			
	UINT8  		Packet[MAX_PACKET_SIZE];
	UINTN		PacketLen = MAX_PACKET_SIZE;
	UINT8  		rtnPacket[MAX_PACKET_SIZE];
	
	gBS->SetMem(
		Packet, 
		MAX_PACKET_SIZE, 
		0x0);		
	
	Packet[0] = G_REPORT_ID_OUT;
	Packet[1] = Packet[2] = 0xFF;
	Packet[3] = 0;
	Packet[4] = CMD_SET_IIC_CLK;

	Print(L"====== First Packet ========================\n");
	for (i=0;i<5;i++) {
		Print(L"%2X  ",Packet[i]);
	}		
	Print(L"00 00 00 .......\n");
				
	Result = 	(*usbMouseDev)->UsbIo->UsbSyncInterruptTransfer(
	              	(*usbMouseDev)->UsbIo,
	              	(*usbMouseDev)->EndpointOUT,  
	              	Packet, 
	              	&PacketLen,
	              	ONE_SEC,
	              	&Status
		 	);
	if (EFI_ERROR (Result)) 
	{
		Print(L"UsbFirstSiuCmd: UsbSyncInterruptTransfer OUT Error=[%r]\n", Result);
		return Result;
	}  
	
	PacketLen = MAX_PACKET_SIZE;
	Result = 	(*usbMouseDev)->UsbIo->UsbSyncInterruptTransfer(
                  	(*usbMouseDev)->UsbIo,
                  	(*usbMouseDev)->EndpointIN,  
                  	&rtnPacket, 
                  	&PacketLen,
                  	ONE_SEC,
                  	&Status
			);
	if (EFI_ERROR (Result)) 
	{
		Print(L"UsbFirstSiuCmd: UsbSyncInterruptTransfer IN Error=[%r]\n", Result);
		return Result;
	}  

	rtnDataLen = rtnPacket[3]+1;
	for (i=0;i<MAX_PACKET_SIZE;i++) {
			Print(L"%2X  ",rtnPacket[i]);
		}	
	Print(L"\n");	
				
	return Result;
}

/**
UsbSiuCmd
**/
EFI_STATUS
EFIAPI
UsbSiuCmd(
IN OUT USB_MOUSE_DEV 	**usbMouseDev,
IN UINT8				iCmdType,
IN UINT8  				*pCmd,
IN UINT8  				CmdLen,
OUT UINT8  			*pRtnBuf
)
{
	UINT32  		Status = 0, Status2 = 0, i;
	EFI_STATUS  	Result = EFI_SUCCESS;
	UINT8  		Packet[MAX_PACKET_SIZE];
	UINTN		PacketLen = MAX_PACKET_SIZE;
	
	gBS->SetMem(
		Packet, 
		MAX_PACKET_SIZE, 
		0xff);		
	
	Packet[0] = G_REPORT_ID_OUT;
	Packet[1] = Packet[2] = 0xFF;
	Packet[3] = CmdLen + 5;
	Packet[4] = iCmdType;
	CopyMem(Packet + 5, pCmd, CmdLen);
	Packet[5 + CmdLen] = GetCheckSum(Packet+1, Packet[3]-1);		
		
	for (i=0; i< ((UINT32)CmdLen+6); i++) {
		Print(L"%2X  ",Packet[i]);
	}		
	Print(L"\n");
				
	Result = 	(*usbMouseDev)->UsbIo->UsbSyncInterruptTransfer(
	              	(*usbMouseDev)->UsbIo,
	              	(*usbMouseDev)->EndpointOUT,  
	              	Packet, 
	              	&PacketLen,
	              	ONE_SEC,
	              	&Status
		 	);
	if (EFI_ERROR (Result)) 
	{
		Print(L"UsbSiuCmd: UsbSyncInterruptTransfer CmdType(%2X) OUT Error=[%r]\n", iCmdType, Result);
		return Result;
	}  	

	PacketLen = MAX_PACKET_SIZE;
	Result = 	(*usbMouseDev)->UsbIo->UsbSyncInterruptTransfer(
                  	(*usbMouseDev)->UsbIo,
                  	(*usbMouseDev)->EndpointIN,  
                  	pRtnBuf, 
                  	&PacketLen,
                  	ONE_SEC,
                  	&Status2
			);
	if (EFI_ERROR (Result)) 
	{
		Print(L"UsbSiuCmd: UsbSyncInterruptTransfer CmdType(%2X) IN Error=[%r]\n", iCmdType, Result);	
		return Result;
	}  
				
	return Result;
}


/**
UsbIoCmd
**/
EFI_STATUS
EFIAPI
UsbIoCmd(
IN OUT USB_MOUSE_DEV 	**usbMouseDev,
IN UINT8  				*pCmd,
IN UINT8  				CmdLen,
OUT UINT8  			*pRtnBuf,
IN UINT8  				ReadLen
)
{
	UINT32  		Status, i;
	EFI_STATUS  	Result = EFI_SUCCESS;
			
	UINT8  		Packet[MAX_PACKET_SIZE];
	UINTN		PacketLen = MAX_PACKET_SIZE;
	UINTN  		RtnBufLen= MAX_PACKET_SIZE;
	
	gBS->SetMem(
		Packet, 
		MAX_PACKET_SIZE, 
		0xff);		

	Packet[0] = G_REPORT_ID_OUT;
	Packet[1] = Packet[2] = 0xFF;
	Packet[3] = CmdLen + 10;
	Packet[4] = CMD_IIC_IO;
	Packet[5] = 0x0;
	Packet[6] = 0x0;
	Packet[7] = CmdLen;	
	Packet[8] = 0x0;
	Packet[9] = ReadLen;
	CopyMem(Packet + 10, pCmd, CmdLen);
	Packet[10 + CmdLen] = GetCheckSum(Packet+1, Packet[3]-1);		

	for (i=0;i<((UINT32)CmdLen + 11);i++) {
		Print(L"%2X  ",Packet[i]);
	}		
	Print(L"\n");

	PacketLen = MAX_PACKET_SIZE;				
	Result = 	(*usbMouseDev)->UsbIo->UsbSyncInterruptTransfer(
	              	(*usbMouseDev)->UsbIo,
	              	(*usbMouseDev)->EndpointOUT,  
	              	Packet, 
	              	&PacketLen,
	              	ONE_SEC,
	              	&Status
		 	);
	if (EFI_ERROR (Result)) 
	{
		Print(L"UsbSendCmd: UsbSyncInterruptTransfer OUT Error=[%r]\n",Result);
		return Result;
	}  
	
	gBS->SetMem(
	Packet, 
	MAX_PACKET_SIZE, 
	0xff);	

	Result = 	(*usbMouseDev)->UsbIo->UsbSyncInterruptTransfer(
                  	(*usbMouseDev)->UsbIo,
                  	(*usbMouseDev)->EndpointIN,  
                  	pRtnBuf, 
                  	&RtnBufLen,
                  	ONE_SEC,
                  	&Status
			);
	if (EFI_ERROR (Result)) 
	{
		Print(L"UsbSendCmd: UsbSyncInterruptTransfer IN Error=[%r]\n",Result);
		return Result;
	}  
				
	return Result;
}


/**
UsbIoCmd
**/
EFI_STATUS
EFIAPI
UsbWriteFlash(
IN OUT USB_MOUSE_DEV 	**usbMouseDev,
IN UINT8  				*pCmd,
IN UINT8  				CmdLen,
IN UINT8				FirstPacket,
IN UINTN				nowAddress,
IN UINTN				flashBlockSize
)
{
	UINT32  		Status;
	EFI_STATUS  	Result = EFI_SUCCESS;		
	UINT8  		Packet[MAX_PACKET_SIZE];
	UINTN		PacketLen = MAX_PACKET_SIZE;
	
	gBS->SetMem(
		Packet, 
		MAX_PACKET_SIZE, 
		0xff);		

	if (FirstPacket == FIRST_PACKET)
	{
		Packet[0] = G_REPORT_ID_OUT;
		Packet[1] = Packet[2] = 0xFF;
		Packet[3] = CmdLen + 16;
		Packet[4] = CMD_IIC_IO;
		Packet[5] = FirstPacket;
		Packet[6] = (flashBlockSize >> 8) & 0xFF;
		Packet[7] = (flashBlockSize & 0xFF)+ 6;	
		Packet[8] = Packet[9] = 0x0;
		Packet[10] = 0xBF;
		Packet[11] = (nowAddress >> 16) & 0xFF;
		Packet[12] = (nowAddress >> 8 ) & 0xFF;
		Packet[13] = nowAddress  & 0xFF;
		Packet[14] = (flashBlockSize >> 8) & 0xFF;
		Packet[15] = flashBlockSize & 0xFF;
		
		CopyMem(Packet + 16, pCmd, CmdLen);
		Packet[16 + CmdLen] = GetCheckSum(Packet+1, Packet[3]-1);		
	}
	else
	{
		Packet[0] = G_REPORT_ID_OUT;
		Packet[1] = Packet[2] = 0xFF;
		Packet[3] = CmdLen + 6;
		Packet[4] = CMD_IIC_IO;
		Packet[5] = FirstPacket;
		CopyMem(Packet + 6, pCmd, CmdLen);
		Packet[6 + CmdLen] = GetCheckSum(Packet+1, Packet[3]-1);	
	}
/*
	UINTN i;
	for (i=0;i<((UINT32)MAX_PACKET_SIZE );i++) {
		Print(L"%2X  ",Packet[i]);
	}		
	Print(L"\n");
*/

	PacketLen = MAX_PACKET_SIZE;	
	Result = 	(*usbMouseDev)->UsbIo->UsbSyncInterruptTransfer(
	              	(*usbMouseDev)->UsbIo,
	              	(*usbMouseDev)->EndpointOUT,  
	              	Packet, 
	              	&PacketLen,
	              	ONE_SEC,
	              	&Status
		 	);
	if (EFI_ERROR (Result)) 
	{
		Print(L"UsbWriteFlash: UsbSyncInterruptTransfer OUT Error=[%r]\n",Result);
		return Result;
	}  
			
	return Result;
}

/**
UsbWriteFlashRtn
**/
 EFI_STATUS
EFIAPI
UsbWriteFlashRtn(
IN OUT USB_MOUSE_DEV 	**usbMouseDev,
OUT UINT8  			*pRtnBuf,
IN UINT8  				ReadLen
)
{
	UINT32  		Status;
	EFI_STATUS  	Result = EFI_SUCCESS;
			
	UINT8  		Packet[MAX_PACKET_SIZE];
	UINTN  		RtnBufLen= MAX_PACKET_SIZE;
	

	gBS->SetMem(
	Packet, 
	MAX_PACKET_SIZE, 
	0xff);	

	Result = 	(*usbMouseDev)->UsbIo->UsbSyncInterruptTransfer(
                  	(*usbMouseDev)->UsbIo,
                  	(*usbMouseDev)->EndpointIN,  
                  	pRtnBuf, 
                  	&RtnBufLen,
                  	ONE_SEC,
                  	&Status
			);
	if (EFI_ERROR (Result)) 
	{
		Print(L"UsbWriteFlashRtn: UsbSyncInterruptTransfer IN Error=[%r]\n",Result);
		return Result;
	}  
				
	return Result;
}



EFI_STATUS
EFIAPI
GetUsbDevice(
IN     EFI_HANDLE           		ImageHandle,
IN OUT USB_MOUSE_DEV 		**usbMouseDev,
IN     UINTN				vID,
IN     UINTN				pID
)
{
	EFI_STATUS  					Status = EFI_SUCCESS;
	EFI_STATUS  					Result = EFI_SUCCESS;
	OUT EFI_USB_IO_PROTOCOL  		*UsbIo;
	EFI_HANDLE 					*ControllerHandle = NULL;
	UINTN	    					HandleIndex , HandleCount;
	EFI_USB_DEVICE_DESCRIPTOR     	DevDesc;
	EFI_USB_INTERFACE_DESCRIPTOR  	IfDesc;
	EFI_USB_ENDPOINT_DESCRIPTOR   	EndPDesc;
	UINT8						EndpointNumber;
	UINT8						EndPointIndex; 
	BOOLEAN                       			Found;
	
	 EFI_USB_DATA_DIRECTION  		Direction;
	 UINT8 						EndpointIN = 0, EndpointOUT = 0;
		
	Status = gBS->LocateHandleBuffer(
				ByProtocol,
				&gEfiUsbIoProtocolGuid,
				NULL,
				&HandleCount,
				&ControllerHandle);  
	if (EFI_ERROR(Status)) {
			Print(L"ERROR : Get USBIO count fail. LocateHandleBuffer(%r)\n", Status);
			return EFI_UNSUPPORTED;
	}
	
	//	Print(L"USBIO count: %d\n", HandleCount);
	 
	for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) 
	{
		Status = gBS->OpenProtocol (
				ControllerHandle[HandleIndex],
				&gEfiUsbIoProtocolGuid,
				&UsbIo,
				ImageHandle,
				NULL,
				EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
		if (EFI_ERROR(Status)) {
				Print(L"ERROR : Open USBIO fail.\n");
				return  EFI_UNSUPPORTED;
		}
     	    	
		Status = UsbIo->UsbGetDeviceDescriptor(UsbIo, &DevDesc);    
		if (EFI_ERROR(Status))
		{
			Print(L"ERROR : Usb Get Device Descriptor fail.\n");
			gBS->FreePool(ControllerHandle); 
			return EFI_UNSUPPORTED;
		}		
		
		//VID=0x10c4 & PID=0x81B9
		if (DevDesc.IdVendor == vID && DevDesc.IdProduct == pID)
		{
//			Print(L"VendorID = %04X, ProductID = %04X\n",
//					DevDesc.IdVendor,
//					DevDesc.IdProduct);  
						
			Status = UsbIo->UsbGetInterfaceDescriptor (UsbIo, &IfDesc);
			if (EFI_ERROR (Status)) 
			{
				Print(L"ERROR : Usb Get Interface Descriptor fail.\n");
				return EFI_SUCCESS;
			}		
					
			//Check the Interface Class for HID
			if (0x03 == IfDesc.InterfaceClass)
			{	
				  
				EndpointNumber = IfDesc.NumEndpoints;
		//		Print(L"IfDesc->EndpointNumber...(%d)\n",IfDesc.NumEndpoints);
							
				  //
				  // Traverse endpoints to find interrupt endpoint
				  //
				Found = FALSE;
				for (EndPointIndex = 0; EndPointIndex < EndpointNumber; EndPointIndex++) {
					    UsbIo->UsbGetEndpointDescriptor (
						             UsbIo,
						             EndPointIndex,
						             &EndPDesc
						             );

					Direction = ((EndPDesc.EndpointAddress & 0x80) != 0) ? EfiUsbDataIn : EfiUsbDataOut;                        
					if(EndPDesc.Attributes == 0x03) //0x00:UsbControlTransfer, 0x02:UsbBulkTransfer, 0x03:UsbSyncInterruptTransfer
					{
					      	switch(Direction)
					      	{
							case EfiUsbDataIn:          
								EndpointIN = EndPDesc.EndpointAddress;
				//			        Print(L"Endpoint : IN (%02X)\n", EndpointIN);
						        break;
						        
						        case EfiUsbDataOut:          
							        EndpointOUT = EndPDesc.EndpointAddress;
				//			        Print(L"Endpoint : OUT (%02X)\n", EndpointOUT); 
						        break;
					      }					                                                                 
					}							    
				}			
				
				(*usbMouseDev)->UsbIo 			= UsbIo;
				(*usbMouseDev)->ControllerHandle 	= ControllerHandle;
				(*usbMouseDev)->IfDesc			= IfDesc;
				(*usbMouseDev)->EndPDesc 			= EndPDesc;
				(*usbMouseDev)->EndpointIN 		= EndpointIN;
				(*usbMouseDev)->EndpointOUT 		= EndpointOUT;
				(*usbMouseDev)->testNum			= 1;
				break;
			}											  
	 	}			 
	}
	if (HandleIndex == HandleCount)
	{
		Print(L"Can't find Device : VID(%02X), PID(%02X)\n", vID, pID); 
		gBS->FreePool(ControllerHandle); 
		Result = EFI_UNSUPPORTED;
	}
	
	return Result;
}


EFI_STATUS
EFIAPI
UpgradeTpFw(
IN OUT USB_MOUSE_DEV 		**usbMouseDev
)
{
	EFI_STATUS  		Result = EFI_SUCCESS;
	
	UINT8         		Data[64];	
	UINT8			DataLen, ReadLen;
	UINT8			rtnData[64];	
	UINTN 			rtnDataLen, i;


Print(L"testNum..(%d)\n", (*gUsbMouseDev).testNum);

//Read tp fw ver.
	Print(L"====== Read tp fw ver. ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0xa6;
	DataLen = 1;
	ReadLen = 1;
	Result = UsbIoCmd(usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<MAX_PACKET_SIZE;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

		(*usbMouseDev)->testNum = 3;
	
	return Result;	
}


EFI_STATUS
EFIAPI
InitSiuToI2cMode(
IN OUT USB_MOUSE_DEV 		**usbMouseDev
)
{
	EFI_STATUS  		Result = EFI_SUCCESS;


	UINT8         		Data[MAX_FLASH_BLOCK_SIZE];
//	UINT8 			Packet[MAX_PACKET_SIZE];
	
	UINT8			DataLen, ReadLen;
	UINT8			rtnData[64];	
	UINTN 			rtnDataLen, i;

/////////////////////////
	UsbFirstSiuCmd(usbMouseDev);

//Get SIU FW ver.	
	Print(L"====== Get SIU FW ver. ========================\n");	
	DataLen = 0;
	Result = UsbSiuCmd(usbMouseDev, CMD_GET_FW_VERSION, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Set Interface
	Print(L"====== Set Interface ========================\n");
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x00;	
	DataLen = 1;
	Result = UsbSiuCmd(usbMouseDev, CMD_SET_INTERFACE, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Get Int Mode
	Print(L"====== Get Int Mode ========================\n");	
	DataLen = 0;
	Result = UsbSiuCmd(usbMouseDev, CMD_GET_TP_INT_MODE, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Set Int Mode
	Print(L"====== Set Int Mode ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x00;	
	DataLen = 1;
	Result = UsbSiuCmd(usbMouseDev, CMD_SET_TP_INT_MODE, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Set IIC CLK
	Print(L"====== Set IIC Mode ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x02;	
	DataLen = 1;
	Result = UsbSiuCmd(usbMouseDev, CMD_SET_IIC_CLK, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");		

//Set IIC Address
	Print(L"====== Set IIC Address ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x2a;	
	DataLen = 1;
	Result = UsbSiuCmd(usbMouseDev, CMD_SET_IIC_SLV_ADDR, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Get SIU FW ver.	
	Print(L"====== Get SIU FW ver. ========================\n");	
	DataLen = 0;
	Result = UsbSiuCmd(usbMouseDev, CMD_GET_FW_VERSION, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Set Interface
	Print(L"====== Set Interface ========================\n");
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x00;	
	DataLen = 1;
	Result = UsbSiuCmd(usbMouseDev, CMD_SET_INTERFACE, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	
	
//Get Int Mode
	Print(L"====== Get Int Mode ========================\n");	
	DataLen = 0;
	Result = UsbSiuCmd(usbMouseDev, CMD_GET_TP_INT_MODE, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Get IIC CLK
	Print(L"====== Get IIC CLK ========================\n");	
	DataLen = 0;
	Result = UsbSiuCmd(usbMouseDev, CMD_GET_IIC_CLK, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Write 0x55
	Print(L"====== Write 0x55 ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x55;	
	DataLen = 1;
	ReadLen = 0;
	Result = UsbIoCmd(usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Set IIC Byte Delay
	Print(L"====== Set IIC Byte Delay ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x02;	
	DataLen = 1;
	Result = UsbSiuCmd(usbMouseDev, CMD_SET_IIC_BYTE_DELAY, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	
	
//Write 00, Read 00
	Print(L"====== Write 00, Read 00 ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x00;	
	DataLen = 1;
	ReadLen = 1;
	Result = UsbIoCmd(usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Write eb aa 09
	Print(L"====== Write eb aa 09 ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0xeb;	
	Data[1] = 0xaa;
	Data[2] = 0x09;
	DataLen = 3;
	ReadLen = 0;
	Result = UsbIoCmd(usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Read eb aa 08
	Print(L"====== Read eb aa 08 ========================\n");	
	DataLen = 0;
	ReadLen = 3;
	Result = UsbIoCmd(usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Read tp fw ver.
	Print(L"====== Read tp fw ver. ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0xa6;
	DataLen = 1;
	ReadLen = 1;
	Result = UsbIoCmd(usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

	(*usbMouseDev)->testNum = 2;
/*
/////////////////////// Enter Upgrade Mode ////////////////////////

//Get  IIC SLV Addr.	
	Print(L"====== Get IIC SLV Addr. ========================\n");	
	DataLen = 0;
	Result = UsbSiuCmd(*usbMouseDev, CMD_GET_IIC_SLV_ADDR, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Send 0xFF
	Print(L"====== Send 0xFF ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x55;
	DataLen = 1;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Set  IIC Bye Delay	
	Print(L"====== Set IIC Byte Delay ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x02;	
	DataLen = 1;
	Result = UsbSiuCmd(*usbMouseDev, CMD_SET_IIC_BYTE_DELAY, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Send 0xFC 0xAA
	Print(L"====== Send 0xFc 0xaa ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0xfc;
	Data[1] = 0xaa;
	DataLen = 2;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Set  IIC Bye Delay	
	Print(L"====== Set IIC Byte Delay ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x02;	
	DataLen = 1;
	Result = UsbSiuCmd(*usbMouseDev, CMD_SET_IIC_BYTE_DELAY, Data, DataLen, rtnData);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");	

//Send 0xFC 0x55
	Print(L"====== Send 0xFC 0x55 ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0xfc;
	Data[1] = 0x55;
	DataLen = 2;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Write eb aa 09
	Print(L"====== Write eb aa 09 ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0xeb;	
	Data[1] = 0xaa;
	Data[2] = 0x09;
	DataLen = 3;
	ReadLen = 3;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//EnterUpgradeMode
	Print(L"====== EnterUpgradeMode ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x55;	
	Data[1] = 0xaa;
	DataLen = 2;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");
	
//Write 0x90 0x00 0x00 0x00 (READ CHIP ID)
	Print(L"====== Write 0x90 0x00 0x00 0x00 ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x90;	
	Data[1] = Data[2] = Data[3] = 0x00;	
	DataLen = 4;
	ReadLen = 2;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Write 0x10 0x0c 
	Print(L"====== Write 0x10 0x0c  ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x10;	
	Data[1] = 0x0c;	
	DataLen = 2;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Write Fw Length
	Print(L"====== Write Fw Length  ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0xb0;	
	Data[1] = 0x00;	
	Data[2] = 0xd8;	
	Data[3] = 0x00;	
	DataLen = 4;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Enable Flash Writable
	Print(L"====== Enable Flash Writable  ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x06;	
	DataLen = 1;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Earse Flash
	Print(L"====== Earse Flash  ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x61;	
	DataLen = 1;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");


//Disable Flash Writable
	Print(L"====== Disable Flash Writable  ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x04;	
	DataLen = 1;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");

//Wait 3 Seconds
	gBS->Stall(3 * ONE_SEC);


	UINTN nowAddress = 0;
	UINT8 nowPacket = 0;
	UINTN BinDataLen  =0;
	while(nowAddress < MAX_BIN_FILE_SIZE){
		gBS->SetMem(Data, MAX_FLASH_BLOCK_SIZE + 1 , 0x0); 
		DataLen  =0;
		if(nowAddress+ MAX_FLASH_BLOCK_SIZE >  MAX_BIN_FILE_SIZE)
		{
			gBS->CopyMem(Data, m_FwBinData+nowAddress,  MAX_BIN_FILE_SIZE - nowAddress);
			BinDataLen =  MAX_BIN_FILE_SIZE - nowAddress;		
		}
		else
		{
			gBS->CopyMem(Data, m_FwBinData+nowAddress,  MAX_FLASH_BLOCK_SIZE);
			BinDataLen =  MAX_FLASH_BLOCK_SIZE;
		}

		nowPacket = 0;
		UINT8 PacketLen  =0;
		while (nowPacket < MAX_FLASH_BLOCK_SIZE){
			gBS->SetMem(Packet, MAX_PACKET_SIZE , 0x0);
			PacketLen  =0;		
			if (nowPacket == 0)
			{ //First section
				gBS->CopyMem(Packet, Data+nowPacket,  MAX_DATA_IN_1ST_PACKET);	
				PacketLen = MAX_DATA_IN_1ST_PACKET;
				UsbWriteFlash(*usbMouseDev, Packet, PacketLen,FIRST_PACKET, nowAddress, MAX_FLASH_BLOCK_SIZE);				
			}
			else if (nowPacket +  MAX_DATA_IN_2ND_PACKET>  MAX_FLASH_BLOCK_SIZE)
			{//Thrid Section
				gBS->CopyMem(Packet, Data+nowPacket,  MAX_DATA_IN_3RD_PACKET);			
				PacketLen =  MAX_DATA_IN_3RD_PACKET;
				UsbWriteFlash(*usbMouseDev, Packet, PacketLen,NOT_FIRST_PACKET, nowAddress, MAX_FLASH_BLOCK_SIZE);					
			}
			else
			{//2//Second Section
				gBS->CopyMem(Packet, Data+nowPacket,  MAX_DATA_IN_2ND_PACKET);				
				PacketLen =  MAX_DATA_IN_2ND_PACKET;
				UsbWriteFlash(*usbMouseDev, Packet, PacketLen,NOT_FIRST_PACKET, nowAddress, MAX_FLASH_BLOCK_SIZE);	
			}		

			nowPacket += PacketLen;
		}
		nowAddress += BinDataLen;
		Print(L"Packet No =(%d)\n", nowAddress/MAX_FLASH_BLOCK_SIZE);
		
		UsbWriteFlashRtn(*usbMouseDev,  rtnData, ReadLen);
		Print(L"UsbWriteFlashRtn 1  ReadLen=(%d) ",ReadLen);
		UsbWriteFlashRtn(*usbMouseDev,  rtnData, ReadLen);
		Print(L"UsbWriteFlashRtn 2  ReadLen=(%d) ",ReadLen);
		
	}


//Wait 1 Seconds
	gBS->Stall(ONE_SEC);

//TP Reset
	Print(L"====== TP Reset  ========================\n");	
	gBS->SetMem(Data, 64, 0xff); 
	Data[0] = 0x07;	
	DataLen = 1;
	ReadLen = 0;
	Result = UsbIoCmd(*usbMouseDev, Data, DataLen, rtnData, ReadLen);
	rtnDataLen = rtnData[3]+1;
	for (i=0;i<rtnDataLen;i++) {
			Print(L"%2X  ",rtnData[i]);
		}	
	Print(L"\n");


*/
	
	return Result;
}


/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
 EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
	EFI_STATUS  			Result = EFI_SUCCESS;
	EFI_FILE_PROTOCOL         FileRoot;
//	EFI_SHELL_PROTOCOL      *gEfiShellProtocol;
	
	gUsbMouseDev 	= AllocateZeroPool(sizeof(USB_MOUSE_DEV));
	gUsbMouseDev->FileRoot = &FileRoot;
	
	//VID=0x10c4 & PID=0x81B9
	Result = GetUsbDevice(
				ImageHandle,
				&gUsbMouseDev,
				0x10c4,							
				0x81B9
				);
	if (EFI_ERROR (Result)) 
	{
		Print(L"ERROR : GetUsbDevice fail.\n");
		goto EXIT;
	}
	gUsbMouseDev->testNum = 0;

	//InitSiuToI2cMode(&gUsbMouseDev);

Print(L"testNum..(%d)\n", (*gUsbMouseDev).testNum);
	
	UpgradeTpFw(&gUsbMouseDev);
        
Print(L"testNum..(%d)\n", (*gUsbMouseDev).testNum);


/*
SHELL_FILE_HANDLE         FileHandle;    
UINTN                           	WbufSize, RbufSize = 16;    
CHAR16                          	*Wbuf= (CHAR16*)L"This is test file\n";    
CHAR16                          	Rbuf[256] ;     
//EFI_SHELL_PROTOCOL      *gEfiShellProtocol;    



Result 	= OpenShellProtocol(&gEfiShellProtocol);    
	if (EFI_ERROR (Result)) 
	{
		Print(L"ERROR : OpenShellProtocol fail.\n");
		goto EXIT;
	}
WbufSize 	= StrLen(Wbuf) * 2;        
 
Result = gEfiShellProtocol->OpenFileByName((CONST CHAR16*)L"abc.txt", &FileHandle, EFI_FILE_MODE_READ);    
	if (EFI_ERROR (Result)) 
	{
		Print(L"ERROR : OpenFileByName fail.\n");
		goto EXIT;
	}
Result = gEfiShellProtocol->ReadFile(FileHandle, &RbufSize ,Rbuf  );   
	if (EFI_ERROR (Result)) 
	{
		Print(L"ERROR : ReadFile fail.\n");
		goto EXIT;
	}
Result = gEfiShellProtocol->CloseFile(FileHandle);
	if (EFI_ERROR (Result)) 
	{
		Print(L"ERROR : CloseFile fail.\n");
		goto EXIT;
	}
	
	for (i=0;i<RbufSize;i++) {
			Print(L"%2X  ",Rbuf[i]);
		}

*/
EXIT:	
	 gBS->FreePool(gUsbMouseDev->ControllerHandle); 	
	 return Result;
}

