synopsis
========

video synopsis


  I want to build a practical surveillance video synopsis engine for personal use. Next time you need to develop a video synopsis prototype engine within short tiem, 
hope this would be helpful.

  I have developed the basic modules including:(with the use of opencv)
    #1  Background Modeling
      This is achieved using running average method for stationary background. The outcome is acceptable. But cannot be made better using algorithm like Gauss Modeling.
    #2  Foreground Detection
      This is achieved using simply the difference between background image and frame image. 
    #3 Target Track
      Using the detection result, we generate a series of rectangle area indicating new target. Then we generate a cascade list as database to store the track information 
      of each moving target. This data structure is easy to understand. However, to make more robust synopsis engine, we need a better and more complicated data structure, 
      maybe a graph structure.
  As you can see, each module can be made better. More attention will be paid to enhance the effectiveness of our engine.
  
  The source code are in the directory of "/source", including two files - "synopsis.h" and "synopsis.cpp". You can use them to build a C++ project. 
I use them to generate dynamic link library(dll) for my own. You can see the hints in "synopsis.h". But anyone can use the interfaces for their own 
convenience. All source code are open here. 

  A simple demo is shown in the "demo.avi". You can see it works for simple situation. But it need improvement to be more robust to various situations.


Author: Sean.W
Date  : 05/21/2014
