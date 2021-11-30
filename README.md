# Aurora NDI EM Tracker Simulink Wrapper

This repo contains a level 2 C-MEX S-Function used to communicate with the Aurora EM tracking unit. The S-Function was created and tested using MATLAB/Simulink 2021a. It works with Aurora firmware revision 11 and compined API revision D.002.007.

To use the S-Function, place the S-Function block into a Simulink model. The blocks has one input and 5 outputs. The input must be a time signal from a clock or an integrator (continous & discrete both work). The clock input is needed to implement delays between certain parts of the initialization code. The first four outputs of the block are for sensor the four sensor ports on the SCU. The output is a 7x1 signal. The first four elements represent the orientation using Quaternians and the remaining three elements contain the position (x,y,z) in mm. This S-Function has been implemented using *single* 5 DOF sensors. It has not been tested with 6 DOF and/or dual sensors. With an understanding of the CompinedAPI one could change the source code to handle such sensors. The fifth output is a signal which indicates the device has been initialized and is now in tracking mode.

This block can be used in 3 different simulation modes

1. Normal Mode
2. Accelerator Mode (Most likely Rapid-Accelerator as well but this has not been tested)
3. Code Generation

The details for each mode is as follows

### Normal & Accelerator Mode

Once in tracking mode, the block can run at up to ~20 Hz before the Simulink engine is unable to keep pace with the real-time demand. The included S-Function block has been made so that it can be placed within a triggered or enabled subsystem. This feature allows for reduced computational cost as the block can be placed in a triggered subsystem with a signal generator (square wave 20Hz) connected to the trigger. This subsystem can be further placed into an enable subsystem so that it is only running when needed.

### Real-Time Code Generation

This block supports real-time code generation, specifically using QuaRC by Quanser. The block likely will work with the MATLAB Desktop Real-Time Kernel however (RTK) this has not been tested. The RTK certian #includes (windows.h for example) so if a build error occours using RTK it is likely that a dependant #include for the S-Function is trying to include an incompatible header file. When generating real-time code one must remember to add the librarys & headers to the custom code tab, underneth code generation in the model preferences. 
