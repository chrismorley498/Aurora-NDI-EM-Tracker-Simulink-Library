# Aurora NDI EM Tracker Simulink Library

The Simulink library found in this repo (auroraNDILibrary.slx) contains a block which can communicate with the Aurora NDI EM Tracker. Under the hood, the block contains a level 2 C-MEX S-Function used to communicate with the Aurora EM tracking unit. The S-Function has been tested using MATLAB/Simulink 2021a. It works with Aurora firmware revision 11 and combined API revision D.002.007.

### Setting up your block

For those wishing to begin using the block right away all that is required is that you add the block to a Simulink model and ensure the current MATLAB path is set to a directory containing the pre-compiled S-Function (auroraNDIComm.mexw64). Currently this block only supports a connection to the SCU via USB. There are two parameters for the block. The first parameter is the desired sample frequency. See the suggested sample times below under the different simulation modes. The second parameter is the Port Number. If the aurora SCU appears under device manger as "COM6" then you would enter "6" in the Port Number field.

### Inputs & Outputs

The block has 1 input and 5 outputs. The input must be a time signal from a clock or an integrator (continuous & discrete both work). The clock input is needed to implement delays between certain parts of the initialization code. The first 4 outputs of the block are output data from sensors connected to ports 1-4 on the SCU. The output is a 7x1 signal. The first four elements of each output signal represent the orientation using quaternions (W,Qx,Qy,Qz) and the last three elements contain the position (x,y,z) in mm. This S-Function has been implemented using *single* 5 DOF sensors attached at each port. It has not been tested with 6 DOF and/or dual sensors (Dual meaning two sensors connected to a single port). With an understanding of the CompinedAPI one could change the source code to handle such sensors. The block supports measurements for up to four sensors. The fifth output is a signal which indicates the device has been initialized and is now in tracking mode (0=not initialized, 1=initialized & tracking).Note that the enable output on the right is not representative of the enable on the top. The enable port on the top of the block is there to allow users to reduce computational load when the sensor measurements are not needed. If the block enable input port is switched to low when the model is running, the outputs will hold their value until the block is enabled again. 

This block can be used in 3 different simulation modes

1. Normal Mode
2. Accelerator Mode (Most likely Rapid-Accelerator as well but this has not been tested)
3. Code Generation

The details for each mode is as follows

### Normal & Accelerator Mode

Once in tracking mode, the block can run at up to ~20 Hz before the Simulink engine is unable to keep pace with the real-time demand.

### Real-Time Code Generation

This block supports real-time code generation, specifically using QuaRC by Quanser. The block will likely work with the MATLAB Desktop Real-Time Kernel (RTK) however this has not been tested. The RTK is incompatible with certain #includes (windows.h for example) so if a build error occurs using RTK it is likely that a dependent #include for the S-Function is trying to include an incompatible header file. When generating real-time code one must remember to add the libraries & headers to the custom code tab, underneath code generation in the model preferences. While running the block with generated code, the block can run up to 40Hz which is the hardware limitation for sampling frequency.

### Runtime Behaviour

The SCU will beep multiple times upon startup. The LEDs for each port, with a connected sensor, will turn green once the SCU has allocated and initialized the sensor connected to that port. The output of the block for a particular sensor will be zero until it has entered the measurement volume and the SCU has been initialized. If a sensor goes out of bounds while the model is running the output of the block will hold it's output as the last known orientation & position data until the sensor re-enters the measurement volume.
