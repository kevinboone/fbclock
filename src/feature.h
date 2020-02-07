/*==========================================================================
  
  boilerplate

  features.h
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

  Define which features are included

==========================================================================*/

#pragma once

// Enable ANSI terminal features like bold, underline. If not defined the
//   relevant functions still exist, but have no effect
//#define FEATURE_ANSI_TERMINAL 1

// If defined, program_context_read_rc_files reads a system file
//   from /etc
//#define FEATURE_SYSTEM_RC 1

// If defined, program_context_read_rc_files reads a user file
//   from $HOME
//#define FEATURE_USER_RC 1

// If defined, includes in the build support for zipfile processing
//#define FEATURE_ZIPFILE 1

// Include number conversion functions. If defined, the application
//  will have to be linked with the standard math library
#define FEATURE_NUMCONVERSION 1


