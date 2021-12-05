#define S_FUNCTION_NAME  auroraNDIComm
#define S_FUNCTION_LEVEL 2
#include <winsock2.h>
#include "simstruc.h"
#include <math.h>
#include <iostream>
#include "CombinedApi.h"
#include "PortHandleInfo.h"
#include "ToolData.h"


static void mdlInitializeSizes(SimStruct *S)
{
    int numInputs=2;
    int numOutputs=5;

    //Creating a I work vector that will be used to hold the number of sensors attached to the SCU
    ssSetNumIWork(S,1);
    ssSetNumPWork(S,1);
    ssSetNumDWork(S,1);
    ssSetDWorkWidth(S,0,28);
    ssSetDWorkDataType(S,0,SS_DOUBLE);

    //Discrete states are used to implement delays between certain API function calls which require time between them
    ssSetNumDiscStates(S,6);

    ssSetNumInputPorts(S,numInputs);
    ssSetNumOutputPorts(S,numOutputs);

    ssSetOptions(S,SS_OPTION_ALLOW_PORT_SAMPLE_TIME_IN_TRIGSS);

    for(int i=0;i<numInputs;i++)
    {
        ssSetInputPortSampleTime(S,i,-1);
        ssSetInputPortOffsetTime(S,i,0);
        ssSetInputPortWidth(S,i,1);
        ssSetInputPortDataType(S,i,SS_DOUBLE);
        ssSetInputPortDirectFeedThrough(S,i,1);
        ssSetInputPortComplexSignal(S,i,COMPLEX_NO);
    }

    for(int i=0;i<numOutputs;i++)
    {
        ssSetOutputPortSampleTime(S,i,-1);
        ssSetOutputPortOffsetTime(S,i,0);
        ssSetOutputPortWidth(S,i,i==4?1:7);
        ssSetOutputPortDataType(S,i,SS_DOUBLE);
        ssSetOutputPortComplexSignal(S,i,COMPLEX_NO);
    }
}//End of mdlInitializeSizes function

static void mdlInitializeSampleTimes(SimStruct *S)
{
}

static void mdlSetInputPortSampleTime(SimStruct *S,int_T portIdx,real_T sampleTime,real_T offsetTime)
{
    int numInputs=2;
    int numOutputs=4;

    for(int i=0;i<numInputs;i++)
    {
        ssSetInputPortSampleTime(S, i, sampleTime);
        ssSetInputPortOffsetTime(S, i, offsetTime);
    }
    //If the function is within a triggered subsystem then the following is called
    if (ssSampleAndOffsetAreTriggered(sampleTime,offsetTime))
    {
        for(int i=0;i<numOutputs;i++)
        {
            ssSetOutputPortSampleTime(S, i, INHERITED_SAMPLE_TIME);
            ssSetOutputPortOffsetTime(S, i, INHERITED_SAMPLE_TIME);
        }
    }
}


static void mdlSetOutputPortSampleTime(SimStruct *S,int_T portIdx,real_T sampleTime,real_T offsetTime)
{
    int numInputs=2;
    int numOutputs=5;

    for(int i=0;i<numOutputs;i++)
    {
        ssSetOutputPortSampleTime(S, i, sampleTime);
        ssSetOutputPortOffsetTime(S, i, offsetTime);
    }
    //If the function is within a triggered subsystem then the following is called
    if (ssSampleAndOffsetAreTriggered(sampleTime,offsetTime))
    {
        for(int i=0;i<numInputs;i++)
        {
            ssSetInputPortSampleTime(S, i, INHERITED_SAMPLE_TIME);
            ssSetInputPortOffsetTime(S, i, INHERITED_SAMPLE_TIME);
        }
    }
}

#define MDL_START
static void mdlStart(SimStruct *S)
{
    //Going to intialize the value of the IWork vector holding the number of connected sensors
    ssSetIWorkValue(S,0,0);

    //Initilize the DWorkVector holding the previous measurements to values of zero
    /*DWork[0:6]    ->  [W,Qx,Qy,Qz,X,Y,Z] Sensor One
     *DWork[7:13]   ->  [W,Qx,Qy,Qz,X,Y,Z] Sensor Two
     *DWork[14:13]  ->  [W,Qx,Qy,Qz,X,Y,Z] Sensor Three
     *DWork[21:28]  ->  [W,Qx,Qy,Qz,X,Y,Z] Sensor Four
     */
    real_T *dWorkValues=(real_T*)ssGetDWork(S,0);
    for(int i=0;i<7;i++)
    {
        dWorkValues[i]=0;
    }

    //Setting All States to Zero upon program entry
    /*x[0]: Connected
     *x[1]: Initialized
     *x[2]: Searched & Sensor Enabled
     *x[3]: Tracking Enabled
     *x[4]: Time
     *x[5]: Begin Measuring
     */
    real_T *x0 = ssGetRealDiscStates(S);
    for (int i=0;i<6;i++)
    {
        *x0++=0.0;
    }

}

//#define MDL_INITIALIZE_CONDITIONS
static void mdlInitializeConditions(SimStruct *S)
{

}//End of mdlInitlialzeConditions


static void mdlOutputs(SimStruct *S, int_T tid)
{
    //Get pointer to DWork Vector. This is used to update/Acess previous sensor measurements (Post Formatting)
    real_T *dWorkValues=(real_T*)ssGetDWork(S,0);

    static CombinedApi capi = CombinedApi();//Create the capi class object
    static CombinedApi *capiPtr;//Create capi pointer
    capiPtr=&capi;
    ssSetPWorkValue(S,0,capiPtr);
    real_T *x=ssGetRealDiscStates(S);//Get pointer to state vector
    InputRealPtrsType u0Ptrs=ssGetInputPortRealSignalPtrs(S,0);//Pointer to input
    InputRealPtrsType u1Ptrs=ssGetInputPortRealSignalPtrs(S,1);//Pointer to input
    double *y0 = ssGetOutputPortRealSignal(S,0);//Pointer to output
    double *y1 = ssGetOutputPortRealSignal(S,1);//Pointer to output
    double *y2 = ssGetOutputPortRealSignal(S,2);//Pointer to output
    double *y3 = ssGetOutputPortRealSignal(S,3);//Pointer to output
    double *auroraInitialized = ssGetOutputPortRealSignal(S,4);//Pointer to output

    double time=double(*u0Ptrs[0]);

    //Connects to the SCU
    if(x[0]==0&&time>3)
    {
        std::string comPort;
        switch(static_cast<int>(*u1Ptrs[0]))
        {
            case(0):
                comPort="COM0";
                break;
            case(1):
                comPort="COM1";
                break;
            case(2):
                comPort="COM2";
                break;
            case(3):
                comPort="COM3";
                break;
            case(4):
                comPort="COM4";
                break;
            case(5):
                comPort="COM5";
                break;
            case(6):
                comPort="COM6";
                break;
            case(7):
                comPort="COM7";
                break;
            case(8):
                comPort="COM8";
                break;
            case(9):
                comPort="COM9";
                break;
            case(10):
                comPort="COM10";
                break;
            case(11):
                comPort="COM11";
                break;
            case(12):
                comPort="COM12";
                break;
            case(13):
                comPort="COM13";
                break;
            case(14):
                comPort="COM14";
                break;
            case(15):
                comPort="COM15";
                break;
            default:
                comPort="COM0";
        }

        std::string hostname = std::string(comPort);
        std::string scu_hostname = "";

        // Attempt to connect to the device
        if (capi.connect(hostname) != 0)
        {
		// Print the error and exit if we can't connect to a device
		std::cout << "[AURORA EM TRACKER]: Connection Failed!" << std::endl;
		std::cout << "Press Enter to continue...";
		std::cin.ignore();
        }
        std::cout << "[AURORA EM TRACKER]: Connected!" << std::endl;
        x[0]=1;//Update the connected state to 1
        x[4]=*u0Ptrs[0];//Update the holding time state
    }//End of connecting to SCU
    else if(x[0]==0&&time<3)//This else-if is added to make sure a value is assigned to x[4] before other statements check it in the condition
    {
        x[4]=*u0Ptrs[0];
    }

    //Initialize the SCU
    else if(((time-x[4])>.5)&&x[1]==0&&x[0]==1)
    {
        capi.initialize();//Initialize the device. This step here 'allocates' the ports
        std::cout<<"[AURORA EM TRACKER]: Allocated Sensor Ports on SCU"<<std::endl;
        x[1]=1;
        x[4]=*u0Ptrs[0];
    }//End of Initialize SCU



    //Search for connected sensors and Intialize all connected ones found
    else if(((time-x[4])>.5)&&x[2]==0&&x[1]==1)
    {
        std::vector<PortHandleInfo> handleInfo=capi.portHandleSearchRequest((PortHandleSearchRequestOption::value)00);//This is used to inform us which port has been assigned to the device
        int numPorts = handleInfo.size();
        ssSetIWorkValue(S,0,numPorts);
        //Initialize the ports for all connected sensors
        for (int i = 0; i < numPorts; i++)
        {
            capi.portHandleInitialize(handleInfo[i].getPortHandle());
        }
        std::cout<<"[AURORA EM TRACKER]: Successfully Initilaized the Sensors"<<std::endl;
        x[2]=1;
        x[4]=*u0Ptrs[0];
    }//End of Search for and Initialize Sensors



    //Enable the ports for all connected & Initialized Sensors, Also activate the feild generator
    else if(((time-x[4])>.5)&&x[3]==0&&x[2]==1)
    {
        std::vector<PortHandleInfo> handleInfo=capi.portHandleSearchRequest((PortHandleSearchRequestOption::value)00);//This is used to inform us which port has been assigned to the device
        int numPorts=ssGetIWorkValue(S,0);
        for (int i = 0; i < numPorts; i++)
        {
            capi.portHandleEnable(handleInfo[i].getPortHandle(), ToolTrackingPriority::Dynamic);//Enable the sensor and define the tool to be one that is mobile
        }
        std::cout<<"[AURORA EM TRACKER]: Enabled the sensors"<<std::endl;
        capi.startTracking();//Begin tracking mode
        std::cout<<"[AURORA EM TRACKER]: Begining Tracking Mode"<<std::endl;
        x[3]=1;
        auroraInitialized[0]=x[3];
        x[4]=*u0Ptrs[0];
    }//End of enabling ports and activating field generator


    //Take Measurements from device
    else if((((time-x[4])>.5)&&x[3]==1)||(x[5]==1))//Test to see if 2 seconds has passed since last change of state OR if the device is already in measuring mode
    {
        std::string handleNames[4]={"0A","0B","0C","0D"};
        int numPorts=ssGetIWorkValue(S,0);
        double sensorReading[7][4] = { 0 };
        double previousPositions[7][4]={0};
        std::string currentData = capi.getTrackingDataTX();
        //Need to add some checks in case on of the sensors goes out of bounds and resultingly the 'currentData' string is not the expected length
        int sensorReadingBeginingLocation[4];
        bool sensorInBounds[4];
        std::string stringsToFormat[4];

        //Begin reading previous measurements
        for(int j=0;j<4;j++)
        {
            for(int i=0;i<7;i++)
            {
             previousPositions[i][j]=double(dWorkValues[i+7*j]);
            }
        }//End of reading previous measurements

        //Begin determining which sensors have valid measurements within the measurement volume
        for (int i = 0; i < numPorts; i++)
        {
            sensorReadingBeginingLocation[i]=currentData.find(handleNames[i]);
            if (currentData.substr(sensorReadingBeginingLocation[i] + 2, 7) == "MISSING")
            {
                sensorInBounds[i] = FALSE;
            }
            else if((currentData.substr(sensorReadingBeginingLocation[i] + 2, 1)=="+")||(currentData.substr(sensorReadingBeginingLocation[i] + 2, 1)=="-"))
            {
                sensorInBounds[i] = TRUE;
            }
            else
            {
                sensorInBounds[i]=FALSE;
            }

            if (sensorInBounds[i] == TRUE)
            {
                stringsToFormat[i] = currentData.substr(sensorReadingBeginingLocation[i] + 2, 45);
            }
            else
            {
                stringsToFormat[i] = "SENSOROUTOFBOUNDS";
            }
        }//Finish determining if sensors are within measurements volume

        //Begin formatting measurement data
        for(int i=0;i<numPorts;i++)
        {
            capi.returnTrackingDataFormated(stringsToFormat[i], sensorReading, 7,i,previousPositions);
        }//Finished formatting measurement data

        //Update all S-Function Outputs
        for(int i=0;i<7;i++)
        {
           y0[i]=sensorReading[i][0];
           y1[i]=sensorReading[i][1];
           y2[i]=sensorReading[i][2];
           y3[i]=sensorReading[i][3];
        }
        dWorkValues[4]=sensorReading[4][0];
        dWorkValues[5]=sensorReading[5][0];
        dWorkValues[6]=sensorReading[6][0];

        //Place current,formatted, measurements in the DWork vector
        for(int j=0;j<4;j++)
        {
            for(int i=0;i<7;i++)
            {
//                 std::cout<<i<<" "<<j<<" "<<i+7*j<<sensorReading[i][j]<<std::endl;
             dWorkValues[i+7*j]=sensorReading[i][j];
            }
        }

        x[5]=1;//Measurement state is either changed to one or remains one
    }//End of aquiring measurements from device
}//End of mdlOutputs


static void mdlTerminate(SimStruct *S)
{
//     static CombinedApi capi = CombinedApi();
//     capi.stopTracking();
//     static CombinedApi *capiPtr;//Create capi pointer
    void *capiPtr=ssGetPWorkValue(S,0);
//     capiPtr->stopTracking();
    std::cout<<"[AURORA EM TRACKER]: Stopping Tracking"<<std::endl;
}


/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
