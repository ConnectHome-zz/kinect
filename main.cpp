#include <iostream>
#include <map>
#include <string>
#include <utility>
//#include <conio.h>
#include <stdio.h>
#include <time.h>

#include <XnCppWrapper.h>
#include <XnVSessionManager.h>
#include <XnVWaveDetector.h>
#include <XnVSwipeDetector.h>
#include <XnVPushDetector.h>
#include <XnVSteadyDetector.h>
#include <XnUSB.h>
#include <XnTypes.h>

//network
#include "network_module.h"
#include "rapidxml-1.13/createXml.h"
#include <time.h>

using namespace std;
using namespace xn;
using namespace rapidxml;


#define PUSH_DETECTOR 1
#define STEADY_DETECTOR 2
#define SWIPE_LEFT 3
#define SWIPE_RIGHT 4
#define LED_OFF 0
#define LED_GREEN 1
#define LED_RED 2
#define LED_ORANGE 3
#define LED_BLINK_ORANGE 4
#define LED_BLINK_GREEN 5
#define LED_BLINK_RED_ORANGE 6

#define WAITTIME 3

SOCKET sock;
GestureGenerator g_GestureGenerator;
HandsGenerator g_HandsGenerator;
time_t lasttime;


void XN_CALLBACK_TYPE SessionProgress(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* UserCxt)
{
	cout << "Session progress." << endl;
}
void XN_CALLBACK_TYPE SessionStart(const XnPoint3D& ptFocusPoint, void* UserCxt)
{
	cout << "Session started." << endl;
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnd(void* UserCxt)
{
	cout << "Session ended. Please perform focus gesture to start session(wave or raisehand)" << endl;
}

//#
void XN_CALLBACK_TYPE swipeLeft(XnFloat fVelocity, XnFloat fAngle, void *pUserCxt)
{
	if(difftime(time(NULL),lasttime)>WAITTIME)
	{
		char buffer[500];
		std::string xml = "" ;
		std::string s = "1";
		xml = acreateGestureXml(s, s, "2", s, "1");
		strcpy(buffer, xml.c_str());
		write_server(sock,buffer);
		lasttime = time(NULL);
	}
    
}
//#
void XN_CALLBACK_TYPE swipeRight(XnFloat fVelocity, XnFloat fAngle, void *pUserCxt)
{
	if(difftime(time(NULL),lasttime)>WAITTIME)
	{
		char buffer[500];
		std::string xml = "" ;
		std::string s = "1";
		xml = acreateGestureXml(s, s, "2", s, "2");
		strcpy(buffer, xml.c_str());
		write_server(sock,buffer);
		lasttime = time(NULL);
	}
}
//#
void XN_CALLBACK_TYPE pointUpdate(const XnVHandPointContext* pContext, void* cxt)
{
	printf("%d: (%f,%f,%f) [%f]\n", pContext->nID, pContext->ptPosition.X, pContext->ptPosition.Y, pContext->ptPosition.Z, pContext->fTime);
}

void XN_CALLBACK_TYPE push(XnFloat fVelocity, XnFloat fAngle, void *pUserCxt)
{
	if(difftime(time(NULL),lasttime)>WAITTIME)
	{
		char buffer[500];
		std::string xml = "" ;
		std::string s = "1";
		xml = acreateGestureXml(s, s, "2", s, "3");
		strcpy(buffer, xml.c_str());
		write_server(sock,buffer);
		lasttime = time(NULL);
	}
}

void XN_CALLBACK_TYPE steady(XnUInt32 nId, XnFloat fStdDev, void *pUserCxt)
{
}

int main(int argc, char **argv)
{
	lasttime = time(NULL);
	if(argc < 3)
   {
      printf("Usage : %s [address] [name] [port]\n", argv[0]);
      return EXIT_FAILURE;
   }
	//first send the application name
	sock = init_connection_module(argv[1],atoi(argv[3]));
	char buffer[BUF_SIZE];
	//use of a descriptor in order to use non-blocking sockets
	fd_set rdfs;
	if(fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
		printf("Error setting socket in non blocking mode\n");
	else
		printf("Socket is in non blocking mode\n");

	// send the Applcation's name
	write_server(sock, argv[2]);
	write_server(sock, createInitGestureXml().c_str());
	XnStatus retVal = XN_STATUS_OK;

	//context creation
	Context context;

	//depth generator
	DepthGenerator depth;

	//for the led
	XN_USB_DEV_HANDLE usbHandle;
	bool foundUsb = false;
	const XnUSBConnectionString *paths;
    XnUInt32 count;
	//for tracking the user
	XnSkeletonJointPosition pos1, pos2;

	retVal = xnUSBInit();
	//retVal = 0;
	if (retVal != XN_STATUS_OK)
    {
        xnPrintError(retVal, "xnUSBInit failed");        
    }
	else
		retVal = xnUSBEnumerateDevices(0x045E /* VendorID */, 0x02B0 /*ProductID*/, &paths, &count);
    if (retVal != XN_STATUS_OK)
    {
        xnPrintError(retVal, "xnUSBEnumerateDevices failed");        
    }else
	{
		retVal = xnUSBOpenDeviceByPath(paths[0], &usbHandle);
		foundUsb = true;
	}


	//sessiong manager - NITE
	XnVSessionManager* pSessionGenerator;

	//context init
	retVal = context.Init();

	retVal = g_GestureGenerator.Create(context); 
	retVal = g_HandsGenerator.Create(context);
	retVal = depth.Create(context);
	
	pSessionGenerator = new XnVSessionManager();
	pSessionGenerator->Initialize(&context, "Wave", "RaiseHand");

	//start generating data
	retVal = context.StartGeneratingAll();

	/* session callbacks - START = when we detect focus or short focus gesture;
						   STOP = when we loose track;
						   PROGRESS = when we're interacting;
	*/
	pSessionGenerator->RegisterSession(NULL, &SessionStart, &SessionEnd, &SessionProgress);

	//swipe control
	XnVSwipeDetector sw;
	sw.RegisterSwipeLeft(NULL, swipeLeft);
	sw.RegisterSwipeRight(NULL, swipeRight);
	pSessionGenerator->AddListener(&sw);
	XnVPushDetector pw;
	pw.RegisterPush(NULL, push);
	pSessionGenerator->AddListener(&pw);
	XnVSteadyDetector st;
	st.RegisterSteady(NULL, steady);
	//pSessionGenerator->AddListener(&st);

	while(true)
	{

		//wait for data to be ready on depth node and update all nodes;		
		retVal = context.WaitAnyUpdateAll();		
		if(retVal != XN_STATUS_OK)
		{			
			cout << "failed updating data; reason: " << xnGetStatusString(retVal) << endl;
			continue;
		}
		pSessionGenerator->Update(&context);

		//network
		resetDescriptor(&sock, &rdfs);
		int n = read_server(sock, buffer);
		if(n > 0)
		  printf("################## Received from server: %s ##############\n", buffer);
	}
		
	

end_connection_module(sock);
context.Release();
delete pSessionGenerator;
	system("PAUSE");

	return EXIT_SUCCESS;
}

